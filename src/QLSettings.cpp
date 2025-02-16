/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdinger@mailbox.org
 *  Kevin Adams
 *  Chris Roberts
 */

#include "QLSettings.h"
#include "IgnoreListItem.h"
#include "QuickLaunch.h"

#include <Application.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <Screen.h>

#include <stdio.h>


const char*
QLSettings::kDefaultSystemIgnore[] = {
	"add-ons",
	"bin",
	"data",
	"lib",
	"servers",
	"haiku_loader.bios_ia32",
	"kernel_x86_64",
	"runtime_loader",
	NULL
};


QLSettings::QLSettings()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	// Defaults
	BScreen* screen = new BScreen(B_MAIN_SCREEN_ID);
	BRect resolution = screen->Frame();
	fMainWindowFrame = BRect(
		resolution.Width() / 2 - 340.0 / 2, 100,
		resolution.Width() / 2 + 340.0 / 2, 0);
	delete screen;
	fSetupWindowFrame = fMainWindowFrame.OffsetByCopy(70.0, 120.0);
	fSetupWindowFrame.bottom = 770;
	fDeskbar = false;
	fShowVersion = fTempShowVersion = false;
	fShowPath = fTempShowPath = true;
	fSearchStart = fTempSearchStart = true;
	fSaveSearch = false;
	fSortFavorites = false;
	fSearchTerm = "";
	fShowIgnore = fTempApplyIgnore = true;
	fFavoriteList = new BObjectList<entry_ref, true>();
	fIgnoreList = new IgnoreListView();

	path.Append("QuickLaunch_settings");
	BFile file(path.Path(), B_READ_ONLY);

	BMessage settings;

	if (file.InitCheck() == B_OK && settings.Unflatten(&file) == B_OK) {
		BRect frame;
		if (settings.FindRect("main window frame", &frame) == B_OK)
			fMainWindowFrame = frame;

		BRect setupframe;
		if (settings.FindRect("setup window frame", &setupframe) == B_OK)
			fSetupWindowFrame = setupframe;

		int32 deskbar;
		if (settings.FindInt32("deskbar", &deskbar) == B_OK)
			fDeskbar = deskbar;

		int32 version;
		if (settings.FindInt32("show version", &version) == B_OK)
			fShowVersion = fTempShowVersion = version;

		int32 path;
		if (settings.FindInt32("show path", &path) == B_OK)
			fShowPath = fTempShowPath = path;

		int32 searchstart;
		if (settings.FindInt32("searchstart", &searchstart) == B_OK)
			fSearchStart = fTempSearchStart = searchstart;

		int32 savesearch;
		if (settings.FindInt32("savesearch", &savesearch) == B_OK)
			fSaveSearch = savesearch;

		BString searchterm;
		if (settings.FindString("searchterm", &searchterm) == B_OK)
			fSearchTerm = searchterm;

		int32 ignore;
		if (settings.FindInt32("show ignore", &ignore) == B_OK)
			fShowIgnore = fTempApplyIgnore = ignore;

		int32 sortfavs;
		if (settings.FindInt32("sort favorites", &sortfavs) == B_OK)
			fSortFavorites = sortfavs;
	}
}


QLSettings::~QLSettings()
{
	delete fFavoriteList;
}


void
QLSettings::SaveSettings()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;

	BMessage settings;
	settings.AddRect("main window frame", my_app->fMainWindow->Frame());
	settings.AddRect("setup window frame", fSetupWindowFrame);
	settings.AddInt32("deskbar", fDeskbar);
	settings.AddInt32("show version", fShowVersion);
	settings.AddInt32("show path", fShowPath);
	settings.AddInt32("searchstart", fSearchStart);
	settings.AddInt32("savesearch", fSaveSearch);
	settings.AddString("searchterm", fSearchTerm);
	settings.AddInt32("show ignore", fShowIgnore);
	settings.AddInt32("sort favorites", fSortFavorites);

	for (int32 i = 0; i < fIgnoreList->CountItems(); i++) {
		IgnoreListItem* item = dynamic_cast<IgnoreListItem*>(fIgnoreList->ItemAt(i));
		if (!item)
			continue;

		if (item->GetItem())
			settings.AddString("item", item->GetItem());
	}

	for (int32 i = 0; i < fFavoriteList->CountItems(); i++) {
		entry_ref* favorite = fFavoriteList->ItemAt(i);

		if (!favorite)
			continue;

		BEntry entry(favorite);
		if (entry.InitCheck() == B_OK) {
			BPath path;
			entry.GetPath(&path);
			settings.AddString("favorite", path.Path());
		}
	}

	path.Append("QuickLaunch_settings");
	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK)
		settings.Flatten(&file);
}


void
QLSettings::InitLists()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;
	path.Append("QuickLaunch_settings");
	BFile file(path.Path(), B_READ_ONLY);

	BMessage settings;
	if (file.InitCheck() == B_OK && settings.Unflatten(&file) == B_OK) {
		BString itemText;
		int32 i = 0;
		while (settings.FindString("item", i++, &itemText) == B_OK) {
			fIgnoreList->AddItem(new IgnoreListItem(itemText.String()));
		}
		i = 0;
		while (settings.FindString("favorite", i++, &itemText) == B_OK) {
			entry_ref favorite;
			get_ref_for_path(itemText.String(), &favorite);
			BEntry entry(&favorite);
			if (entry.Exists())
				fFavoriteList->AddItem(new entry_ref(favorite));
		}
	} else // First launch? Add default ignore items
		AddDefaultIgnore();
}


void
QLSettings::AddDefaultIgnore()
{
	BPath systemDir;
	if (find_directory(B_SYSTEM_DIRECTORY, &systemDir) != B_OK)
		return;

	int32 count = fIgnoreList->CountItems();

	for (const char** list = kDefaultSystemIgnore; *list != NULL; ++list) {
		bool inList = false;
		BString dir = systemDir.Path();
		dir << "/" << *list;
		for (int i = 0; i < count; i++) {
			IgnoreListItem* item = dynamic_cast<IgnoreListItem*>(fIgnoreList->ItemAt(i));
			if (strcasecmp(dir.String(), item->GetItem()) == 0) {
				inList = true;
				break;
			}
		}
		if (!inList)
			fIgnoreList->AddItem(new IgnoreListItem(dir));
	}
}


bool
QLSettings::Lock()
{
	return fLock.Lock();
}


void
QLSettings::Unlock()
{
	fLock.Unlock();
}
