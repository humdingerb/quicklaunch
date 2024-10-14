/*
 * Copyright 2010-2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 *  Chris Roberts
 */

#include "MainWindow.h"
#include "IgnoreListItem.h"
#include "QLFilter.h"
#include "QuickLaunch.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MessageRunner.h>

#include <algorithm>

// from QuickLaunch.cpp
extern const char* kApplicationName;

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


static int
compare_items(const void* a, const void* b)
{
	MainListItem* stringA = *(MainListItem**)a;
	MainListItem* stringB = *(MainListItem**)b;

	return strcasecmp(stringA->GetName(), stringB->GetName());
}


static int
compare_favorite_items(const void* a, const void* b)
{
	MainListItem* stringA = *(MainListItem**)a;
	MainListItem* stringB = *(MainListItem**)b;

	if (stringA->IsFavorite() && !stringB->IsFavorite())
		return -1;
	else if (stringB->IsFavorite() && !stringA->IsFavorite())
		return 1;

	return compare_items(a, b);
}


MainWindow::MainWindow()
	:
	BWindow(BRect(), B_TRANSLATE_SYSTEM_NAME(kApplicationName), B_TITLED_WINDOW_LOOK,
		B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE | B_FRAME_EVENTS
			| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	BuildAppList();

	QLSettings& settings = my_app->Settings();
	fIconHeight = (int32(be_control_look->ComposeIconSize(B_LARGE_ICON).Height()) + 2);

	fSearchBox = new BTextControl("SearchBox", NULL, NULL, NULL);
	fSearchBox->SetModificationMessage(new BMessage (NEW_FILTER));

	fSetupButton = new BButton("Setup", B_TRANSLATE("Setup"), new BMessage(SETUP_BUTTON));
	fSetupButton->SetTarget(be_app);
	fHelpButton = new BButton("Help", B_TRANSLATE("Help"), new BMessage(HELP_BUTTON));
	fHelpButton->SetTarget(be_app);

	fListView = new MainListView();
	fListView->SetExplicitMinSize(BSize(B_SIZE_UNSET, fIconHeight + 8));

	fScrollView = new BScrollView("ScrollList", fListView, B_WILL_DRAW, false, true);

	// Build the layout
	float spacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fSearchBox)
			.AddStrut(spacing)
			.Add(fSetupButton)
			.AddStrut(spacing / 2)
			.Add(fHelpButton)
			.SetInsets(spacing / 2)
			.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fScrollView)
			.SetInsets(spacing / 2, 0, spacing / 2, spacing / 2)
			.End();

	fSearchBox->MakeFocus(true);

	AddCommonFilter(new QLFilter);
	fListView->SetInvocationMessage(new BMessage(RETURN_KEY));
	fListView->SetViewColor(B_TRANSPARENT_COLOR);

	if (settings.Lock()) {
		if (settings.GetSaveSearch())
			fSearchBox->SetText(settings.GetSearchTerm());
		settings.Unlock();
	}
}


MainWindow::~MainWindow()
{
	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		settings.SetSearchTerm(GetSearchString());
		settings.SaveSettings();
		settings.Unlock();
	}
}


#pragma mark-- BWindow Overrides --


void
MainWindow::MessageReceived(BMessage* message)
{
	QLSettings& settings = my_app->Settings();

	switch (message->what) {
		case CURSOR_UP:
		{
			int selection = fListView->CurrentSelection();
			int last = fListView->IndexOf(fListView->LastItem());
			if (selection != 0)
				fListView->Select(selection - 1);
			else
				fListView->Select(last);

			fListView->ScrollToSelection();
			break;
		}
		case CURSOR_DOWN:
		{
			int selection = fListView->CurrentSelection();
			int last = fListView->IndexOf(fListView->LastItem());
			int first = fListView->IndexOf(fListView->FirstItem());
			if (selection != last)
				fListView->Select(selection + 1);
			else
				fListView->Select(first);

			fListView->ScrollToSelection();
			break;
		}
		case PAGE_UP:
		{
			int selection = fListView->CurrentSelection();
			int first = fListView->IndexOf(fListView->FirstItem());

			if (selection > kMAX_DISPLAYED_ITEMS) {
				fListView->Select(selection - kMAX_DISPLAYED_ITEMS + 1);
				fListView->ScrollBy(0.0,
					-(kMAX_DISPLAYED_ITEMS - 1) * fListView->ItemFrame(0).Height()
						- kMAX_DISPLAYED_ITEMS + 1);
			} else
				fListView->Select(first);

			fListView->ScrollToSelection();
			break;
		}
		case PAGE_DOWN:
		{
			int selection = fListView->CurrentSelection();
			int last = fListView->IndexOf(fListView->LastItem());

			if (selection < (fListView->CountItems() - kMAX_DISPLAYED_ITEMS)) {
				fListView->Select(selection + kMAX_DISPLAYED_ITEMS - 1);
				fListView->ScrollBy(0.0,
					(kMAX_DISPLAYED_ITEMS - 1) * fListView->ItemFrame(0).Height()
						+ kMAX_DISPLAYED_ITEMS - 1);
			} else
				fListView->Select(last);
			break;
		}
		case HOME:
		{
			if (fListView->IsEmpty() == 0)
				fListView->Select(0);

			fListView->ScrollToSelection();
			break;
		}
		case END:
		{
			if (fListView->IsEmpty() == 0)
				fListView->Select(fListView->IndexOf(fListView->LastItem()));

			fListView->ScrollToSelection();
			break;
		}
		case RETURN_CTRL_KEY:
		{
			Hide();

			// Begin DW code
			entry_ref* ref = NULL;
			MainListItem* item = NULL;

			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem*>(fListView->ItemAt(selection));
			if (item)
				ref = item->Ref();

			if (ref) {
				// if we got a ref, try opening its parent folder by sending a
				// B_REFS_RECEIVED to Tracker
				BEntry entry(ref);
				BEntry parent;
				entry.GetParent(&parent);
				entry_ref folderRef;
				parent.GetRef(&folderRef);
				BMessenger msgr("application/x-vnd.Be-TRAK");
				BMessage refMsg(B_REFS_RECEIVED);
				refMsg.AddRef("refs", &folderRef);
				msgr.SendMessage(&refMsg);
				// End DW code
				// select the app in the opened folder (thanks, opentargetfolder)
				BMessage selectMessage('Tsel');
				entry_ref target;
				if (entry.GetRef(&target) != B_OK) {
					// don't alert, selection is not critical
					break;
				}
				selectMessage.AddRef("refs", &target);
				// wait 0.3 sec to give Tracker time to populate
				BMessageRunner::StartSending(BMessenger("application/x-vnd.Be-TRAK"),
					&selectMessage, 300000, 1);
			}
			if (settings.Lock()) {
				settings.SetSearchTerm(GetSearchString());
				settings.Unlock();
			}

			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case RETURN_SHIFT_KEY:
		{
			MainListItem* item = NULL;
			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem*>(fListView->ItemAt(selection));
			if (item != NULL)
				_LaunchApp(item);
			break;
		}
		case SINGLE_CLICK:
		case RETURN_KEY:
		{
			Hide();

			MainListItem* item = NULL;
			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem*>(fListView->ItemAt(selection));
			if (item != NULL)
				_LaunchApp(item);

			if (settings.Lock()) {
				settings.SetSearchTerm(GetSearchString());
				settings.Unlock();
			}

			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case ADDFAVORITE:
		case REMOVEFAVORITE:
		case ADDIGNORE:
		{
			BMessenger messenger(fListView);
			messenger.SendMessage(message);
			break;
		}
		case NEW_FILTER:
		{
			FilterAppList();
			fListView->Select(0);
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}


bool
MainWindow::QuitRequested()
{
	return true;
}


#pragma mark-- Public Methods --



void
MainWindow::BuildAppList()
{
	fAppList.MakeEmpty();
	QLSettings& settings = my_app->Settings();

	bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();

	BVolumeRoster volumeRoster;
	BVolume volume;
	BQuery query;

	while (volumeRoster.GetNextVolume(&volume) == B_OK) {
		if (volume.KnowsQuery()) {
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

				BPath dir;
				BPath parent;
				entry.GetPath(&path);
				path.GetParent(&parent);

				find_directory(B_SYSTEM_ADDONS_DIRECTORY, &dir);
				if (strstr(parent.Path(), dir.Path()))
					continue;
				// check Trash on all volumes
				find_directory(B_TRASH_DIRECTORY, &dir, false, &volume);
				if (strstr(parent.Path(), dir.Path()))
					continue;

				bool ignore = false;
				if (settings.GetShowIgnore()) {
					int32 ignoreCount = settings.fIgnoreList->CountItems();
					if (ignoreCount != 0) {
						BString* newItem = new BString(path.Path());
						for (int i = 0; i < ignoreCount; i++) {
							IgnoreListItem* sItem = dynamic_cast<IgnoreListItem*>(
								settings.fIgnoreList->ItemAt(i));

							if (newItem->ICompare(sItem->GetItem(),
								std::min(newItem->Length(), sItem->GetItem().Length())) == 0)
								ignore = true;
						}
					}
				}
				if (!ignore && entry.InitCheck() == B_OK)
					fAppList.AddItem(new AppListItem(entry, localized));
			}
			query.Clear();
		}
	}
}


void
MainWindow::FilterAppList()
{
	QLSettings& settings = my_app->Settings();

	fListView->MakeEmpty();
	if (settings.Lock()) {
		if (GetStringLength() == 0) {
			// show favorites
			for (int32 i = 0; i < settings.fFavoriteList->CountItems(); i++) {
				entry_ref* favorite = static_cast<entry_ref*>(settings.fFavoriteList->ItemAt(i));
				if (!favorite)
					continue;
				BEntry entry(favorite);
				if (entry.InitCheck() == B_OK)
					fListView->AddItem(new MainListItem(&entry, favorite->name, fIconHeight, true));
			}
		} else {
			for (int32 i = 0; i < fAppList.CountItems(); i++) {
				BString searchtext = GetSearchString();
				BString name = fAppList.ItemAt(i)->GetName();

				bool found;
				if (settings.GetSearchStart() == 1 && !searchtext.IStartsWith("*"))
					found = name.IStartsWith(searchtext);
				else {
					searchtext.RemoveFirst("*");
					found = name.IFindFirst(searchtext) == B_ERROR ? false : true;
				}

				if (found) {
					bool isFav = false;
					BEntry entry = fAppList.ItemAt(i)->GetRef();
					for (int32 i = 0; i < settings.fFavoriteList->CountItems(); i++) {
						entry_ref* favorite
							= static_cast<entry_ref*>(settings.fFavoriteList->ItemAt(i));

						if (!favorite)
							continue;
						BEntry favEntry(favorite);
						if (favEntry == entry)
							isFav = true;
					}
					if (entry.InitCheck() == B_OK)
						fListView->AddItem(new MainListItem(&entry, name, fIconHeight, isFav));
				}
			}

			if (settings.GetSortFavorites())
				fListView->SortItems(&compare_favorite_items);
			else
				fListView->SortItems(&compare_items);
		}
	}
	settings.Unlock();
	ResizeWindow();
}


float
MainWindow::GetScrollPosition()
{
	float position;
	BScrollBar* scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	position = scrollBar->Value();
	return (position);
}


void
MainWindow::SetScrollPosition(float position)
{
	BScrollBar* scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	scrollBar->SetValue(position);
	return;
}


void
MainWindow::ResizeWindow()
{
	int32 count = fListView->CountItems();
	count = (count < kMAX_DISPLAYED_ITEMS) ? count : kMAX_DISPLAYED_ITEMS;
	BRect itemRect = fListView->ItemFrame(0);
	float itemHeight = itemRect.Height();
	float windowRest = Frame().Height() - fListView->Frame().Height();
	ResizeTo(Bounds().Width(), count * itemHeight + windowRest + count - 2);
}


#pragma mark-- Private Methods --


void
MainWindow::_LaunchApp(MainListItem* item)
{
	entry_ref* ref = NULL;
	ref = item->Ref();

	if (ref != NULL) {
		status_t result = be_roster->Launch(ref);
		BString errorMessage;
		if (result != B_OK && result != B_ALREADY_RUNNING) {
			BString errStr(B_TRANSLATE_COMMENT("Failed to launch %appname%.\n\n"
											   "Error:",
				"Don't translate the variable %appname%."));
			errStr.ReplaceFirst("%appname%", item->GetName());
			errorMessage << errStr.String() << " ";
			errorMessage << strerror(result);
		} else {
			// clear error message on success (might have been
			// filled when trying to launch by ref)
			errorMessage = "";
		}
		if (errorMessage.Length() > 0) {
			BAlert* alert = new BAlert(
				"error", errorMessage.String(), B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_FROM_WIDEST);
			alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
			alert->Go();
		}
	}
}