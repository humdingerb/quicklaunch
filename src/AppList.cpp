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


void
AppList::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case BUILDAPPLIST:
		{
			_BuildAppList();
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


void
AppList::_BuildAppList()
{
	fAppList.MakeEmpty();
	QLSettings& settings = my_app->Settings();

	bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();
	bool activeIgnore = settings.GetTempApplyIgnore();
	int32 ignoreCount = settings.fIgnoreList->CountItems();

	BVolumeRoster volumeRoster;
	BVolume volume;
	BQuery query;

	while (volumeRoster.GetNextVolume(&volume) == B_OK) {
		if (volume.KnowsQuery()) {
			// Check if the whole volume is on ignore list
			if (activeIgnore && ignoreCount != 0) {
				bool ignore = false;
				BDirectory root;
				volume.GetRootDirectory(&root);
				BPath mountPoint(&root, NULL);

				BString newItem(mountPoint.Path());
				for (int i = 0; i < ignoreCount; i++) {
					IgnoreListItem* sItem = dynamic_cast<IgnoreListItem*>(
						settings.fIgnoreList->ItemAt(i));

					if (newItem.Compare(sItem->GetItem()) == 0) {
						ignore = true;
						break;
					}
				}
				if (ignore)
					continue;
			}

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

				// ignore Trash on all volumes
				BPath trashDir;
				if (find_directory(B_TRASH_DIRECTORY, &trashDir, false, &volume) == B_OK) {
					if (strstr(parent.Path(), trashDir.Path()))
						continue;
				}

				bool ignore = false;
				if (activeIgnore && ignoreCount != 0) {
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
				if (!ignore && entry.InitCheck() == B_OK)
					fAppList.AddItem(new AppListItem(entry, localized));
			}
			query.Clear();
		}
	}

	fInit = true;
	SendNotices(BUILDAPPLIST);
}
