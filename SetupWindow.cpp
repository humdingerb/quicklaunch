/*
 * Copyright 2010-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 */

#include "QuickLaunch.h"
#include "QLSettings.h"
#include "IgnoreListItem.h"
#include "SetupWindow.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SetupWindow"


static int
compare_items(const void* a, const void* b)
{
	IgnoreListItem* stringA = *(IgnoreListItem**)a;
	IgnoreListItem* stringB = *(IgnoreListItem**)b;

	return strcmp(stringA->GetItem(), stringB->GetItem());
}


SetupWindow::SetupWindow(BRect frame)
	:
	BWindow(frame, B_TRANSLATE("Setup"), B_TITLED_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE | B_FRAME_EVENTS
		| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		fIgnoreList = settings.IgnoreList();
		settings.Unlock();
	}
	fChkDeskbar = new BCheckBox("DeskbarChk",
		B_TRANSLATE("Show Deskbar replicant"),
		new BMessage(DESKBAR_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkDeskbar->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkVersion = new BCheckBox("VersionChk",
		B_TRANSLATE("Show application version"),
		new BMessage(VERSION_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkVersion->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkPath = new BCheckBox("PathChk", B_TRANSLATE("Show application path"),
		new BMessage(PATH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkPath->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSearchStart = new BCheckBox("SearchStartChk",
		B_TRANSLATE("Search from start of application name"),
		new BMessage(SEARCHSTART_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSearchStart->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSaveSearch = new BCheckBox("SaveSearchChk",
		B_TRANSLATE("Remember last search term"),
		new BMessage(SAVESEARCH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSaveSearch->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkSortFavorites = new BCheckBox("SortFavoritesChk",
	B_TRANSLATE("Sort favorite items to the top"),
	new BMessage(SORTFAVS_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSortFavorites->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkIgnore = new BCheckBox("IgnoreChk",
		B_TRANSLATE("Ignore these files & folders (and their subfolders):"),
		new BMessage(IGNORE_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkIgnore->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fIgnoreScroll = new BScrollView("IgnoreList", fIgnoreList,
		B_WILL_DRAW | B_NAVIGABLE, false, true, B_FANCY_BORDER);
	fIgnoreScroll->SetExplicitMinSize(BSize(B_SIZE_UNSET, 48));
	fIgnoreScroll->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fButAdd = new BButton("AddButton", B_TRANSLATE("Add" B_UTF8_ELLIPSIS),
		new BMessage(ADD_BUT));
	fButAdd->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fButRem = new BButton("RemButton", B_TRANSLATE("Remove"),
		new BMessage(REM_BUT));
	fButRem->SetEnabled(false);
	fButRem->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fChkDeskbar->SetTarget(be_app);
	fChkVersion->SetTarget(be_app);
	fChkPath->SetTarget(be_app);
	fChkSearchStart->SetTarget(be_app);
	fChkSaveSearch->SetTarget(be_app);
	fChkSortFavorites->SetTarget(be_app);
	fChkIgnore->SetTarget(be_app);

	// Build the layout

	float spacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkDeskbar)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkVersion)
			.Add(fChkPath)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkSearchStart)
			.Add(fChkSaveSearch)
			.Add(fChkSortFavorites)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkIgnore)
			.Add(fIgnoreScroll)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_HORIZONTAL, spacing)
			.AddGlue()
			.Add(fButAdd)
			.Add(fButRem)
			.AddGlue()
			.SetInsets(0, spacing, spacing, spacing)
		.End()
	.End();

	if (settings.Lock()) {
		fChkDeskbar->SetValue(settings.GetDeskbar());
		fChkVersion->SetValue(settings.GetShowVersion());
		fChkPath->SetValue(settings.GetShowPath());
		fChkSearchStart->SetValue(settings.GetSearchStart());
		fChkSaveSearch->SetValue(settings.GetSaveSearch());
		fChkSortFavorites->SetValue(settings.GetSortFavorites());
		fChkIgnore->SetValue(settings.GetShowIgnore());

		settings.Unlock();
	}
	fIgnoreList->SetViewColor(B_TRANSPARENT_COLOR);

	fOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
		B_FILE_NODE | B_DIRECTORY_NODE);
	fOpenPanel->SetTarget(this);

	AddShortcut('S', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));
}


SetupWindow::~SetupWindow()
{
	delete fOpenPanel;
}


#pragma mark -- BWindow Overrides --


bool
SetupWindow::QuitRequested()
{
	this->Hide();

	if (fOpenPanel->Window()->IsHidden() == false)
		fOpenPanel->Hide();

	my_app->SetWindowsFeel(B_MODAL_ALL_WINDOW_FEEL);
	return false;
}


void
SetupWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case ADD_BUT:
		{
			fOpenPanel->Show();
			break;
		}
		case REM_BUT:
		{
			_RemoveSelected();
			be_app->PostMessage(FILEPANEL);
			break;
		}
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			int32 ref_num;
			entry_ref ref;
			status_t err;
			ref_num = 0;

			QLSettings& settings = my_app->Settings();
			if (settings.Lock()) {
				while ((err = message->FindRef("refs", ref_num, &ref)) == B_OK) {
					BPath path;
					BEntry* entry = new BEntry(&ref);
					entry->GetPath(&path);
					IgnoreListItem* newitem = new IgnoreListItem(path.Path());
					bool duplicate = false;

					for (int i = 0; i < fIgnoreList->CountItems(); i++)
					{
						IgnoreListItem* sItem = dynamic_cast<IgnoreListItem *>
							(fIgnoreList->ItemAt(i));
						if (strcmp(sItem->GetItem(), newitem->GetItem()) == 0)
							duplicate = true;
					}
					if (!duplicate)	{
						fIgnoreList->AddItem(newitem);
						fIgnoreList->SortItems(&compare_items);
					}
					ref_num++;
				}
			settings.Unlock();
			}
			be_app->PostMessage(FILEPANEL);
		}
	}
}


#pragma mark -- Private Methods --


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
			int32 index = (int32)(addr_t)indices.ItemAtFast(i) - i;
			delete fIgnoreList->RemoveItem(index);
		}
		settings.Unlock();
	}
}
