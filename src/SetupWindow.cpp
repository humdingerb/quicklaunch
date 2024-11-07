/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdinger@mailbox.org
 *  Kevin Adams
 *  Chris Roberts
 */

#include "SetupWindow.h"
#include "IgnoreListItem.h"
#include "QLSettings.h"
#include "QuickLaunch.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SetupWindow"


static int
compare_items(const void* a, const void* b)
{
	IgnoreListItem* stringA = *(IgnoreListItem**)a;
	IgnoreListItem* stringB = *(IgnoreListItem**)b;

	return strcmp(stringA->GetItem(), stringB->GetItem());
}


SetupWindow::SetupWindow(BRect frame, BMessenger main_msgr)
	:
	BWindow(frame, B_TRANSLATE("Settings"), B_TITLED_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_FRAME_EVENTS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fMainMessenger(main_msgr)
{
	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		fIgnoreList = settings.IgnoreList();
		settings.Unlock();
	}
	fChkDeskbar = new BCheckBox("DeskbarChk", B_TRANSLATE("Show Deskbar replicant"),
		new BMessage(DESKBAR_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkDeskbar->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkVersion = new BCheckBox("VersionChk", B_TRANSLATE("Show application version"),
		new BMessage(VERSION_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkVersion->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkPath = new BCheckBox("PathChk", B_TRANSLATE("Show application path"),
		new BMessage(PATH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkPath->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSearchStart
		= new BCheckBox("SearchStartChk", B_TRANSLATE("Search from start of application name"),
			new BMessage(SEARCHSTART_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSearchStart->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSaveSearch = new BCheckBox("SaveSearchChk", B_TRANSLATE("Remember last search term"),
		new BMessage(SAVESEARCH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSaveSearch->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSortFavorites
		= new BCheckBox("SortFavoritesChk", B_TRANSLATE("Sort favorite items to the top"),
			new BMessage(SORTFAVS_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSortFavorites->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* openShortcuts = new BButton("OpenShortcuts", B_TRANSLATE("Open Shortcuts preferences"),
		new BMessage(OPEN_SHORTCUTS));
	openShortcuts->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkIgnore = new BCheckBox("IgnoreChk",
		B_TRANSLATE("Ignore these files & folders (and their subfolders):"),
		new BMessage(IGNORE_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkIgnore->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fIgnoreScroll = new BScrollView(
		"IgnoreList", fIgnoreList, B_WILL_DRAW | B_NAVIGABLE, false, true, B_FANCY_BORDER);
	fIgnoreScroll->SetExplicitMinSize(BSize(B_SIZE_UNSET, 48));
	fIgnoreScroll->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* butDefaults = new BButton("DefaultsButton", B_TRANSLATE("Defaults"),
		new BMessage(DEFAULTS_BUT));

	fButAdd = new BButton("AddButton", B_TRANSLATE("Add" B_UTF8_ELLIPSIS), new BMessage(ADD_BUT));
	fButAdd->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fButRem = new BButton("RemButton", B_TRANSLATE("Remove"), new BMessage(REM_BUT));
	fButRem->SetEnabled(false);
	fButRem->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkDeskbar->SetTarget(be_app);
	fChkVersion->SetTarget(fMainMessenger);
	fChkPath->SetTarget(fMainMessenger);
	fChkSearchStart->SetTarget(fMainMessenger);
	fChkSortFavorites->SetTarget(fMainMessenger);

	// Build the layout

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkDeskbar)
			.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkVersion)
			.Add(fChkPath)
			.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkSearchStart)
			.Add(fChkSaveSearch)
			.Add(fChkSortFavorites)
			.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(openShortcuts)
			.AddGlue()
			.End()
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkIgnore)
			.Add(fIgnoreScroll)
			.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(butDefaults)
			.AddGlue()
			.Add(fButAdd)
			.Add(fButRem)
			.End()
		.End();

	if (settings.Lock()) {
		fChkDeskbar->SetValue(settings.GetDeskbar());
		fChkVersion->SetValue(settings.GetShowVersion());
		fChkPath->SetValue(settings.GetShowPath());
		fChkSearchStart->SetValue(settings.GetSearchStart());
		fChkSaveSearch->SetValue(settings.GetSaveSearch());
		fChkSortFavorites->SetValue(settings.GetSortFavorites());
		fChkIgnore->SetValue(settings.GetApplyIgnore());

		settings.Unlock();
	}
	fIgnoreList->SetViewColor(B_TRANSPARENT_COLOR);

	fOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE | B_DIRECTORY_NODE);
	fOpenPanel->SetTarget(this);

	AddShortcut('S', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));
}


SetupWindow::~SetupWindow()
{
	delete fOpenPanel;
}


#pragma mark-- BWindow Overrides --


bool
SetupWindow::QuitRequested()
{
	QLSettings& settings = my_app->Settings();
	settings.SetSetupWindowFrame(Frame());

	if (fOpenPanel->Window()->IsHidden() == false)
		fOpenPanel->Hide();

	fMainMessenger.SendMessage(SETUP_MENU);

	return false;
}


void
SetupWindow::MessageReceived(BMessage* message)
{
	QLSettings& settings = my_app->Settings();

	switch (message->what) {
		case IGNORE_CHK:
		{
			if (settings.fIgnoreList->IsEmpty()) {
				fChkIgnore->SetValue(false);
				settings.SetApplyIgnore(false);

				break;
			}
			int32 value;
			message->FindInt32("be:value", &value);

			settings.SetApplyIgnore(value);
			fChkIgnore->SetValue(value);

			fMainMessenger.SendMessage(message);
			break;
		}
		case SAVESEARCH_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			settings.SetSaveSearch(value);
			break;
		}
		case FILEPANEL:
		{
			if (!settings.fIgnoreList->IsEmpty()) {
				fChkIgnore->SetValue(true);
				settings.SetApplyIgnore(true);
			} else {
				fChkIgnore->SetValue(false);
				settings.SetApplyIgnore(false);
			}
			fMainMessenger.SendMessage(new BMessage(BUILDAPPLIST));
			break;
		}
		case OPEN_SHORTCUTS:
		{
			be_roster->Launch("application/x-vnd.Haiku-Shortcuts");
			break;
		}
		case DEFAULTS_BUT:
		{
			QLSettings& settings = my_app->Settings();
			settings.AddDefaultIgnore();
			fIgnoreList->SortItems(&compare_items);
			break;
		}
		case ADD_BUT:
		{
			fOpenPanel->Show();
			break;
		}
		case REM_BUT:
		{
			_RemoveSelected();
			fMainMessenger.SendMessage(new BMessage(BUILDAPPLIST));
			break;
		}
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			int32 ref_num;
			entry_ref ref;
			ref_num = 0;

			QLSettings& settings = my_app->Settings();
			if (settings.Lock()) {
				while ((message->FindRef("refs", ref_num, &ref)) == B_OK) {
					BPath path;
					BEntry* entry = new BEntry(&ref);
					entry->GetPath(&path);
					IgnoreListItem* newitem = new IgnoreListItem(path.Path());
					bool duplicate = false;

					for (int i = 0; i < fIgnoreList->CountItems(); i++) {
						IgnoreListItem* sItem
							= dynamic_cast<IgnoreListItem*>(fIgnoreList->ItemAt(i));
						if (strcmp(sItem->GetItem(), newitem->GetItem()) == 0) {
							duplicate = true;
							break;
						}
					}
					if (!duplicate) {
						fIgnoreList->LockLooper();
						fIgnoreList->AddItem(newitem);
						fIgnoreList->SortItems(&compare_items);
						fIgnoreList->UnlockLooper();
					} else
						delete newitem;
					ref_num++;
				}
				settings.Unlock();
			}
			fMainMessenger.SendMessage(new BMessage(BUILDAPPLIST));
		}
	}
}


#pragma mark-- Private Methods --


void
SetupWindow::_GetSelectedItems(BList& indices)
{
	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		for (int32 i = 0; true; i++) {
			int32 index = fIgnoreList->CurrentSelection(i);
			if (index < 0)
				break;
			if (!indices.AddItem((void*)(addr_t)index))
				break;
		}
		settings.Unlock();
	}
}


void
SetupWindow::_RemoveSelected()
{
	BList indices;
	int32 index = 0;
	_GetSelectedItems(indices);

	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		index = fIgnoreList->CurrentSelection() - 1;
		fIgnoreList->DeselectAll();
		settings.Unlock();
	}

	if (indices.CountItems() > 0)
		_RemoveItemList(indices);

	if (settings.Lock()) {
		if (fIgnoreList->CountItems() > 0) {
			if (index < 0)
				index = 0;

			fIgnoreList->Select(index);
		}
		settings.Unlock();
	}
}


void
SetupWindow::_RemoveItemList(const BList& indices)
{
	int32 count = indices.CountItems();

	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		for (int32 i = 0; i < count; i++) {
			int32 index = (int32)(addr_t) indices.ItemAtFast(i) - i;
			delete fIgnoreList->RemoveItem(index);
		}
		settings.Unlock();
	}
}
