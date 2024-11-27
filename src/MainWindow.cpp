/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdinger@mailbox.org
 *  Kevin Adams
 *  Máximo Castañeda
 *  Chris Roberts
 */

#include "MainWindow.h"
#include "MenuBar.h"
#include "IconMenuItem.h"
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
extern char* kApplicationSignature;
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
			| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fBusy(false),
	fAppList(20, true)
{
	QLSettings& settings = my_app->Settings();
	fIconHeight = (int32(be_control_look->ComposeIconSize(B_LARGE_ICON).Height()) + 2);

	fSetupWindow = new SetupWindow(settings.GetSetupWindowFrame(), this);
	fSetupWindow->Hide();
	fSetupWindow->Show();

	BMenuBar* menubar = new BMenuBar("mainmenu");
	BMenuItem* item;

	BMenu* menu = new BMenu("");
	menu->AddItem(new BMenuItem(
		B_TRANSLATE("Settings" B_UTF8_ELLIPSIS), new BMessage(SETUP_MENU), ','));
	menu->AddSeparatorItem();
	item = new BMenuItem(B_TRANSLATE("Help"), new BMessage(HELP_MENU), 'H');
	item->SetTarget(be_app);
	menu->AddItem(item);
	item = new BMenuItem(B_TRANSLATE("About QuickLaunch"), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));

	menubar->AddItem(new IconMenuItem(menu, NULL, kApplicationSignature, B_MINI_ICON));

	fSelectionMenu = new BMenu(B_TRANSLATE("Selection"));
	fAddRemoveFav = new BMenuItem("AddRemoveFavorite", new BMessage(ADD_REMOVE_FAVORITE));
	fSelectionMenu->AddItem(fAddRemoveFav);
	fAddToIgnore = new BMenuItem(B_TRANSLATE_CONTEXT("Add to ignore list", "ListView"), new BMessage(ADDIGNORE) , 'I');
	fSelectionMenu->AddItem(fAddToIgnore);
	item = new BMenuItem(B_TRANSLATE_CONTEXT("Open containing folder", "ListView"), new BMessage(OPENLOCATION), 'O');
	fSelectionMenu->AddItem(item);

	menubar->AddItem(fSelectionMenu);

	menu = new BMenu(B_TRANSLATE("Temporary options"));
	fTempShowPath = new BMenuItem(
		B_TRANSLATE_CONTEXT("Show application path", "SetupWindow"), new BMessage(PATH_CHK), 'P');
	fTempShowPath->SetMarked(settings.GetTempShowPath() == true);
	menu->AddItem(fTempShowPath);
	fTempShowVersion = new BMenuItem(
		B_TRANSLATE_CONTEXT("Show application version", "SetupWindow"), new BMessage(VERSION_CHK), 'V');
	fTempShowVersion->SetMarked(settings.GetTempShowVersion() == true);
	menu->AddItem(fTempShowVersion);
	menu->AddSeparatorItem();
	fTempSearchStart = new BMenuItem(
		B_TRANSLATE_CONTEXT("Search from start of application name", "SetupWindow"), new BMessage(SEARCHSTART_CHK), 'S');
	fTempSearchStart->SetMarked(settings.GetTempSearchStart() == true);
	menu->AddItem(fTempSearchStart);
	fTempApplyIgnore = new BMenuItem(
		B_TRANSLATE("Apply ignore list"), new BMessage(IGNORE_CHK), 'A');
	fTempApplyIgnore->SetMarked(settings.GetTempApplyIgnore() == true);
	menu->AddItem(fTempApplyIgnore);

	menubar->AddItem(menu);

	BString searchstring;
	if (settings.Lock()) {
		if (settings.GetSaveSearch())
			searchstring = settings.GetSearchTerm();
		settings.Unlock();
	}
	fSearchBox = new BTextControl("SearchBox", NULL, searchstring, NULL);
	fSearchBox->SetModificationMessage(new BMessage(NEW_FILTER));

	fListView = new MainListView();
	fListView->SetExplicitMinSize(BSize(B_SIZE_UNSET, fIconHeight + 8));

	fScrollView = new BScrollView("ScrollList", fListView, B_WILL_DRAW, false, true);

	// Build the layout

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menubar)
		.AddGroup(B_VERTICAL, 0)
			.Add(fSearchBox)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(fScrollView)
			.SetInsets(B_USE_HALF_ITEM_SPACING)
			.End();

	fSearchBox->MakeFocus(true);

	AddCommonFilter(new QLFilter);
	fListView->SetInvocationMessage(new BMessage(RETURN_KEY));
	fListView->SetViewColor(B_TRANSPARENT_COLOR);

	if (searchstring == "")
		_ShowFavorites();
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
MainWindow::MenusBeginning()
{
	MainListItem* sItem
		= dynamic_cast<MainListItem*>(fListView->ItemAt(fListView->CurrentSelection()));

	if (sItem == NULL)
		return;

	if (sItem->IsFavorite()) {
		fAddRemoveFav->SetLabel(B_TRANSLATE_CONTEXT("Remove favorite", "ListView"));
		fAddRemoveFav->SetShortcut('R', B_COMMAND_KEY);
		fAddToIgnore->SetEnabled(false);
	} else {
		fAddRemoveFav->SetLabel(B_TRANSLATE_CONTEXT("Add to favorites", "ListView"));
		fAddRemoveFav->SetShortcut('F', B_COMMAND_KEY);
		fAddToIgnore->SetEnabled(true);
	}
}


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
		case ADD_REMOVE_FAVORITE:
		case ADDIGNORE:
		{
			BMessenger messenger(fListView);
			messenger.SendMessage(message);
			break;
		}
		case B_REFS_RECEIVED:
		{
			fSetupWindow->MessageReceived(message);
			break;
		}
		case NEW_FILTER:
		{
			_FilterAppList();

			if (fListView->IsEmpty()) {
				fSelectionMenu->SetEnabled(false);
				break;
			} else
				fSelectionMenu->SetEnabled(true);

			fListView->Select(0);
			break;
		}
		case B_SIMPLE_DATA:
		{
			if (fSearchBox->TextLength() == 0)
				_AddDroppedAsFav(message);
			break;
		}
		case SETUP_MENU:
		{
			if (fSetupWindow->IsHidden()) {
				SetFeel(B_NORMAL_WINDOW_FEEL);
				fSetupWindow->Show();
			} else {
				SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
				fSetupWindow->Hide();
			}
			break;
		}
		case VERSION_CHK:
		{
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				settings.SetShowVersion(value);
			else
				value = fTempShowVersion->IsMarked() == true ? 0 : 1;

			settings.SetTempShowVersion(value);
			fTempShowVersion->SetMarked(value);
			fListView->Invalidate();
			break;
		}
		case PATH_CHK:
		{
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				settings.SetShowPath(value);
			else
				value = fTempShowPath->IsMarked() == true ? 0 : 1;

			settings.SetTempShowPath(value);
			fTempShowPath->SetMarked(value);
			fListView->Invalidate();
			break;
		}
		case SEARCHSTART_CHK:
		{
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				settings.SetSearchStart(value);
			else
				value = fTempSearchStart->IsMarked() == true ? 0 : 1;

			settings.SetTempSearchStart(value);
			fTempSearchStart->SetMarked(value);

			if (!fListView->IsEmpty())
				_FilterKeepPositionSelection();

			break;
		}
		case SORTFAVS_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			settings.SetSortFavorites(value);

			if (!fListView->IsEmpty())
				_FilterKeepPositionSelection();
			break;
		}
		case IGNORE_CHK:
		{
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				settings.SetApplyIgnore(value);
			else
				value = fTempApplyIgnore->IsMarked() == true ? 0 : 1;

			settings.SetTempShowIgnore(value);
			fTempApplyIgnore->SetMarked(value);
			// intentional fall-thru

		}
		case BUILDAPPLIST:
		{
			BuildAppList();
			break;
		}
		case OPENLOCATION:
		{
			fListView->MessageReceived(message);
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
	fThreadId = spawn_thread(_AppListThread, "build app list", B_NORMAL_PRIORITY, this);
	if (fThreadId < 0)
		return;
	resume_thread(fThreadId);
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


/*static*/ status_t
MainWindow::_AppListThread(void* _self)
{
	MainWindow* self = (MainWindow*)_self;
	self->_BuildAppList();
	return B_OK;
}


void
MainWindow::_BuildAppList()
{
	fBusy = true;

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

	fBusy = false;
	PostMessage(NEW_FILTER);
}


void
MainWindow::_FilterAppList()
{
	if (fBusy)
		return;

	if (fAppList.IsEmpty())
		BuildAppList();

	QLSettings& settings = my_app->Settings();
	BString searchtext = GetSearchString();

	fListView->MakeEmpty();

	if (settings.Lock()) {
		if (searchtext == "")
			_ShowFavorites();
		else {
			int32 searchFromStart = settings.GetTempSearchStart();
			bool showAll = (searchtext == "*");
			bool startJocker = searchtext.StartsWith("*");
			if (startJocker)
				searchtext.RemoveFirst("*");
			for (int32 i = 0; i < fAppList.CountItems(); i++) {
				BString name = fAppList.ItemAt(i)->GetName();
				bool found = true;
				if (!showAll) {
					if (searchFromStart == 1 && !startJocker)
						found = name.IStartsWith(searchtext);
					else
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
MainWindow::_GetScrollPosition()
{
	float position;
	BScrollBar* scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	position = scrollBar->Value();
	return (position);
}


void
MainWindow::_SetScrollPosition(float position)
{
	BScrollBar* scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	scrollBar->SetValue(position);
	return;
}


void
MainWindow::_FilterKeepPositionSelection()
{
	int32 selection = fListView->CurrentSelection();
	float position = _GetScrollPosition();
	_FilterAppList();
	if (selection >= 0) {
		fListView->Select((selection < fListView->CountItems())
				? selection : fListView->CountItems() - 1);
	} else if (!fListView->IsEmpty())
		fListView->Select(0);

	_SetScrollPosition(position);
}


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


void
MainWindow::_AddDroppedAsFav(BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("refs", &ref) != B_OK)
		return;

	BPoint dropPoint = message->DropPoint();
	int32 dropIndex = fListView->IndexOf(fListView->ConvertFromScreen(dropPoint));
	if (dropIndex < 0)
		dropIndex = fListView->CountItems(); // move to bottom

	QLSettings& settings = my_app->Settings();

	bool duplicate = false;

	if (settings.Lock()) {
		for (int i = 0; i < settings.fFavoriteList->CountItems(); i++) {
			entry_ref* favorite = settings.fFavoriteList->ItemAt(i);
			if (ref == *favorite) {
				duplicate = true;
				break;
			}
		}
		if (!duplicate) {
			settings.fFavoriteList->AddItem(new entry_ref(ref), dropIndex);
			fListView->MakeEmpty();
			_ShowFavorites();
		}
		settings.Unlock();
	}
	ResizeWindow();
}


void
MainWindow::_ShowFavorites()
{
	QLSettings& settings = my_app->Settings();
	bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();

	for (int32 i = 0; i < settings.fFavoriteList->CountItems(); i++) {
		entry_ref* favorite = settings.fFavoriteList->ItemAt(i);
		if (favorite == NULL)
			continue;
		BEntry entry(favorite);
		if (entry.InitCheck() == B_OK) {
			BString appName;
			if (!localized
				|| BLocaleRoster::Default()->GetLocalizedFileName(appName, *favorite) != B_OK)
				appName = favorite->name;
			fListView->AddItem(new MainListItem(&entry, appName, fIconHeight, true));
		}
	}
}
