/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdinger@mailbox.org
 */

#include "AppList.h"

#include "IgnoreListItem.h"
#include "QLSettings.h"
#include "QuickLaunch.h"

#include <LocaleRoster.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Query.h>
#include <Volume.h>
#include <VolumeRoster.h>


AppList::AppList()
	:
	BLooper("app list builder"),
	fInit(false),
	fAppList(20, true)
{
	Run();
}


AppList::~AppList()
{
	if (fInit)
		stop_watching(this);
}


void
AppList::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case BUILDAPPLIST:
		{
			_BuildAppList();
			break;
		}
		case B_NODE_MONITOR:
		{
			int32 opcode = message->GetInt32("opcode", -1);

			if (opcode == B_DEVICE_MOUNTED) {
				int32 device;
				if (message->FindInt32("new device", 0, &device) != B_OK) {
					_BuildAppList();
					break;
				}

				bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();
				BVolume volume(device);
				if (_AppendVolumeItems(volume, localized) > 0)
					SendNotices(BUILDAPPLIST);

			} else if (opcode == B_DEVICE_UNMOUNTED) {
				_BuildAppList();
			}

			break;
		}
		default:
		{
			BLooper::MessageReceived(message);
			break;
		}
	}
}


const AppListItems*
AppList::Items()
{
	if (IsLocked() && fInit)
		return &fAppList;

	if (!fInit)
		PostMessage(BUILDAPPLIST);

	return NULL;
}


int
AppList::_AppendVolumeItems(const BVolume& volume, bool localized)
{
	if (volume.InitCheck() != B_OK || !volume.KnowsQuery())
		return 0;

	QLSettings& settings = my_app->Settings();
	int32 ignoreCount;
	if (settings.GetTempApplyIgnore())
		ignoreCount = settings.fIgnoreList->CountItems();
	else
		ignoreCount = 0;

	// Check if the whole volume is on ignore list
	if (ignoreCount > 0) {
		BDirectory root;
		volume.GetRootDirectory(&root);
		BPath mountPoint(&root, NULL);

		BString newItem(mountPoint.Path());
		for (int i = 0; i < ignoreCount; i++) {
			IgnoreListItem* sItem = dynamic_cast<IgnoreListItem*>(settings.fIgnoreList->ItemAt(i));
			if (sItem->Ignores(newItem))
				return 0;
		}
	}

	int appended = 0;
	BQuery query;

	char trashPath[B_PATH_NAME_LENGTH];
	size_t trashPathLength;
	if (find_directory(B_TRASH_DIRECTORY, volume.Device(), false, trashPath, sizeof(trashPath)) == B_OK)
		trashPathLength = strlen(trashPath);
	else
		trashPathLength = 0;

	// Set up the volume and predicate for the query.
	query.SetVolume(&volume);
	query.PushAttr("BEOS:TYPE");
	query.PushString("application/x-vnd.be-elfexecutable", true);
	query.PushOp(B_EQ);

	query.PushAttr("BEOS:APP_SIG");
	query.PushString("application/x");
	query.PushOp(B_BEGINS_WITH);
	query.PushOp(B_AND);

	status_t status = query.Fetch();

	if (status != B_OK)
		printf("2. what happened? %s\n", strerror(status));

	BEntry entry;
	BPath path;
	while (query.GetNextEntry(&entry) == B_OK) {
		if (!entry.IsFile())
			continue;

		if (entry.GetPath(&path) < B_OK) {
			fprintf(stderr, "could not get path for entry\n");
			continue;
		}

		BPath parent;
		entry.GetPath(&path);
		path.GetParent(&parent);

		// ignore Trash
		if (trashPathLength > 0 && strncmp(parent.Path(), trashPath, trashPathLength) == 0) {
			char nextChar = parent.Path()[trashPathLength];
			if (nextChar == '\0' || nextChar == '/')
				continue;
		}

		bool ignore = false;
		if (ignoreCount > 0) {
			BString newItem(path.Path());
			for (int i = 0; i < ignoreCount; i++) {
				IgnoreListItem* sItem = dynamic_cast<IgnoreListItem*>(
					settings.fIgnoreList->ItemAt(i));

				if (sItem->Ignores(newItem)) {
					ignore = true;
					break;
				}
			}
		}
		if (!ignore && entry.InitCheck() == B_OK) {
			fAppList.AddItem(new AppListItem(entry, localized));
			appended++;
		}
	}
	query.Clear();
	return appended;
}


void
AppList::_BuildAppList()
{
	if (!fInit) {
		fInit = true;
		watch_node(NULL, B_WATCH_MOUNT, this);
	}

	fAppList.MakeEmpty();

	bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();

	BVolumeRoster volumeRoster;
	BVolume volume;
	while (volumeRoster.GetNextVolume(&volume) == B_OK)
		_AppendVolumeItems(volume, localized);

	SendNotices(BUILDAPPLIST);
}
