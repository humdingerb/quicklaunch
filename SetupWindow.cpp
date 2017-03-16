/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "MainListItem.h"
#include "QuickLaunch.h"
#include "QLSettings.h"
#include "SetupWindow.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SetupWindow"


static int
compare_items(const void* a, const void* b)
{
	BStringItem* stringA = *(BStringItem**)a;
	BStringItem* stringB = *(BStringItem**)b;

	return strcmp(stringA->Text(), stringB->Text());
}


SetupWindow::SetupWindow(BRect frame)
	:
	BWindow(frame, B_TRANSLATE("Setup"), B_TITLED_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
		| B_CLOSE_ON_ESCAPE)
{
	fChkVersion = new BCheckBox("VersionChk",
		B_TRANSLATE("Show application version"),
		new BMessage(VERSION_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkVersion->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkPath = new BCheckBox("PathChk", B_TRANSLATE("Show application path"),
		new BMessage(PATH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkPath->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkDelay = new BCheckBox("DelayChk",
		B_TRANSLATE("Wait for a second letter before searching"),
		new BMessage(DELAY_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkDelay->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkSaveSearch = new BCheckBox("SaveSearchChk",
		B_TRANSLATE("Remember last search term"),
		new BMessage(SAVESEARCH_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSaveSearch->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkSingleClick = new BCheckBox("SingleClickChk",
		B_TRANSLATE("Launch applications with a single click"),
		new BMessage(SINGLECLICK_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkSingleClick->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkOnTop = new BCheckBox("OnTopChk",
		B_TRANSLATE("Window always on top"),
		new BMessage(ONTOP_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkOnTop->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fChkIgnore = new BCheckBox("IgnoreChk",
		B_TRANSLATE("Ignore these files & folders (and their subfolders):"),
		new BMessage(IGNORE_CHK), B_WILL_DRAW | B_NAVIGABLE);
	fChkIgnore->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fIgnoreList = new SetupListView();
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

	fChkVersion->SetTarget(be_app);
	fChkPath->SetTarget(be_app);
	fChkDelay->SetTarget(be_app);
	fChkSaveSearch->SetTarget(be_app);
	fChkOnTop->SetTarget(be_app);
	fChkSingleClick->SetTarget(be_app);
	fChkIgnore->SetTarget(be_app);

	// Build the layout

	float spacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkVersion)
			.Add(fChkPath)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fChkDelay)
			.Add(fChkSaveSearch)
			.Add(fChkSingleClick)
			.Add(fChkOnTop)
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

	QLApp *app = dynamic_cast<QLApp *> (be_app);
	fChkVersion->SetValue(app->fSettings->GetShowVersion());
	fChkPath->SetValue(app->fSettings->GetShowPath());
	fChkDelay->SetValue(app->fSettings->GetDelay());
	fChkSaveSearch->SetValue(app->fSettings->GetSaveSearch());
	fChkSingleClick->SetValue(app->fSettings->GetSingleClick());
	fChkOnTop->SetValue(app->fSettings->GetOnTop());
	fChkIgnore->SetValue(app->fSettings->GetShowIgnore());

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

	QLApp *app = dynamic_cast<QLApp *> (be_app);
	int32 value = app->fSettings->GetOnTop();
	app->SetWindowsFeel(value);

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
			int32		ref_num;
			entry_ref	ref;
			status_t	err;
			ref_num = 0;

	    	while ((err = message->FindRef("refs", ref_num, &ref)) == B_OK) {
				BPath path;
				BEntry *entry = new BEntry(&ref);
				entry->GetPath(&path);
				BStringItem *newitem = new BStringItem(path.Path());
				bool duplicate = false;

				for (int i = 0; i < fIgnoreList->CountItems(); i++)
				{
					BStringItem *sItem = dynamic_cast<BStringItem *>
						(fIgnoreList->ItemAt(i));
					if (strcmp(sItem->Text(), newitem->Text()) == 0)
						duplicate = true;
				}
				if (!duplicate)	{
					fIgnoreList->AddItem(newitem);
					fIgnoreList->SortItems(&compare_items);
				}
				ref_num++;
			}
			be_app->PostMessage(FILEPANEL);
		}
	}
}


#pragma mark -- Private Methods --


void
SetupWindow::_GetSelectedItems(BList& indices)
{
	for (int32 i = 0; true; i++) {
		int32 index = fIgnoreList->CurrentSelection(i);
		if (index < 0)
			break;
		if (!indices.AddItem((void*)(addr_t)index))
			break;
	}
}


void
SetupWindow::_RemoveSelected()
{
	BList indices;
	_GetSelectedItems(indices);
	int32 index = fIgnoreList->CurrentSelection() - 1;

	fIgnoreList->DeselectAll();

	if (indices.CountItems() > 0)
		_RemoveItemList(indices);

	if (fIgnoreList->CountItems() > 0) {
		if (index < 0)
			index = 0;

		fIgnoreList->Select(index);
	}
}


void
SetupWindow::_RemoveItemList(const BList& indices)
{
	int32 count = indices.CountItems();
	for (int32 i = 0; i < count; i++) {
		int32 index = (int32)(addr_t)indices.ItemAtFast(i) - i;
		delete fIgnoreList->RemoveItem(index);
	}
}
