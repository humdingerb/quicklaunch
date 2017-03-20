/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "QLFilter.h"
#include "QuickLaunch.h"
#include "MainWindow.h"
#include "SetupListItem.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <Font.h>
#include <LayoutBuilder.h>

#include <algorithm>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"

static int
compare_items(const void* a, const void* b)
{
	MainListItem* stringA = *(MainListItem**)a;
	MainListItem* stringB = *(MainListItem**)b;

	return strcasecmp(stringA->GetName(), stringB->GetName());
}


MainWindow::MainWindow()
	:
	BWindow(BRect(), B_TRANSLATE_SYSTEM_NAME("QuickLaunch"),
		B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE
		| B_FRAME_EVENTS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	QLApp *app = dynamic_cast<QLApp *> (be_app);

	fSearchBox = new BTextControl("SearchBox", NULL, NULL,
		new BMessage(SEARCH_BOX));

	fSetupButton = new BButton ("Setup", B_TRANSLATE("Setup"),
		new BMessage(SETUP_BUTTON));
	fSetupButton->SetTarget(be_app);
	fHelpButton = new BButton ("Help", B_TRANSLATE("Help"),
		new BMessage(HELP_BUTTON));
	fHelpButton->SetTarget(be_app);

	fListView = new MainListView();

	font_height finfo;
	be_plain_font->GetHeight(&finfo);
	float fontHeight = finfo.ascent + finfo.descent + finfo.leading;
	if (fontHeight < 16.0)
		fontHeight = 16.0;
	fListView->SetExplicitMinSize(BSize(B_SIZE_UNSET, fontHeight * 2.5));
	fListView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fScrollView = new BScrollView("ScrollList", fListView, B_WILL_DRAW,
		false, true);

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

	if (app->fSettings->GetSaveSearch())
		fSearchBox->SetText(app->fSettings->GetSearchTerm());

	int32 value = app->fSettings->GetOnTop();
	SetFeel(value ?	B_MODAL_ALL_WINDOW_FEEL : B_NORMAL_WINDOW_FEEL);

	_GetIconHeight();
}




MainWindow::~MainWindow()
{
}


#pragma mark -- BWindow Overrides --


void
MainWindow::MessageReceived(BMessage* message)
{
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
				fListView->ScrollBy(0.0, -1 * (kMAX_DISPLAYED_ITEMS - 1)
					* fListView->ItemFrame(0).Height() - 8);
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
				fListView->ScrollBy(0.0, (kMAX_DISPLAYED_ITEMS - 1)
					* fListView->ItemFrame(0).Height() + 8);
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
			entry_ref *ref = NULL;
			MainListItem *item = NULL;

			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem *>(fListView->ItemAt(selection));
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
			}
			// End DW code
			QLApp *app = dynamic_cast<QLApp *> (be_app);
			app->fSettings->SetSearchTerm(GetSearchString());

			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case RETURN_SHIFT_KEY:
		{
			MainListItem *item = NULL;
			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem *>(fListView->ItemAt(selection));
			if (item != NULL)
				_LaunchApp(item);
			break;
		}
		case SINGLE_CLICK:
		{
			QLApp *app = dynamic_cast<QLApp *> (be_app);
			if (app->fSettings->GetSingleClick() == false)
				break;
		}	// intentional fall-through
		case RETURN_KEY:
		{
			Hide();

			MainListItem *item = NULL;
			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem *>(fListView->ItemAt(selection));
			if (item != NULL)
				_LaunchApp(item);

			QLApp *app = dynamic_cast<QLApp *> (be_app);
			app->fSettings->SetSearchTerm(GetSearchString());

			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case NEW_FILTER:
		{
			QLApp *app = dynamic_cast<QLApp *> (be_app);
			int letters = GetStringLength();
			if (letters > app->fSettings->GetDelay()) {
				const char *searchString = GetSearchString();
				BuildList(searchString);
				fListView->Select(0);
			} else {
				fListView->MakeEmpty();
				ResizeTo(Bounds().Width(), 0);		// original size
				fListView->Invalidate();
			}
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


#pragma mark -- Public Methods --


void
MainWindow::BuildList(const char *predicate)
{
	fListView->MakeEmpty();

	BVolumeRoster volumeRoster;
	BVolume volume;
	BQuery query;

	while (volumeRoster.GetNextVolume(&volume) == B_OK) {
		if (volume.KnowsQuery())
		{
			// Set up the volume and predicate for the query.
			query.SetVolume(&volume);
			query.SetPredicate(predicate);

			status_t status = query.Fetch();
			char buffer[B_FILE_NAME_LENGTH];
			volume.GetName(buffer);
			if (status == B_BAD_VALUE) {
				query.PushAttr("BEOS:TYPE");
				query.PushString("application/x-vnd.be-elfexecutable", true);
				query.PushOp(B_EQ);

				query.PushAttr("BEOS:APP_SIG");
				query.PushString("application/x");
				query.PushOp(B_BEGINS_WITH);
				query.PushOp(B_AND);

				query.PushAttr("name");
				query.PushString(predicate, true);
				query.PushOp(B_BEGINS_WITH);
				query.PushOp(B_AND);

				status = query.Fetch();
			}
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

				QLApp *app = dynamic_cast<QLApp *> (be_app);
				bool ignore = false;
				if (app->fSettings->GetShowIgnore()) {
					BString *newItem = new BString(path.Path());
					for (int i = 0; i < app->fSetupWindow->fIgnoreList->CountItems(); i++)
					{
						SetupListItem *sItem = dynamic_cast<SetupListItem *>
							(app->fSetupWindow->fIgnoreList->ItemAt(i));
						BString *ignoreItem = new BString(sItem->GetItem());

						if (newItem->ICompare(*ignoreItem, std::min(newItem->Length(),
								ignoreItem->Length())) == 0)
							ignore = true;
					}
				}
				if (!ignore)
					fListView->AddItem(new MainListItem(&entry, fIconHeight));
			}
			query.Clear();
		}
	}
	fListView->SortItems(&compare_items);

	BRect windowRest = Frame().Height() - fListView->Frame().Height();
	BRect itemFrame = fListView->ItemFrame(0);
	int32 count = fListView->CountItems();
	if (count < kMAX_DISPLAYED_ITEMS) {
		ResizeTo(Bounds().Width(), count * itemFrame.Height()
			+ windowRest.Height() + count);
	} else {
		ResizeTo(Bounds().Width(), kMAX_DISPLAYED_ITEMS * itemFrame.Height()
			+ windowRest.Height() + kMAX_DISPLAYED_ITEMS);
	}
}


float
MainWindow::GetScrollPosition()
{
	float position;
	BScrollBar *scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	position = scrollBar->Value();
	return (position);
}


void
MainWindow::SetScrollPosition(float position)
{
	BScrollBar *scrollBar = fScrollView->ScrollBar(B_VERTICAL);
	scrollBar->SetValue(position);
	return;
}


#pragma mark -- Private Methods --


void
MainWindow::_GetIconHeight()
{
	font_height	fontHeight;
	be_plain_font->GetHeight(&fontHeight);
	float height = 2 * (fontHeight.ascent + fontHeight.descent
		+ fontHeight.leading);
	fIconHeight = int(height * 0.9);
//	printf("height: %i, fIconHeight: %i\n", (int)height, fIconHeight);

	static int iconSizes[] = { 16, 32, 40, 48, 64, 72, 80, 96, 1000 };

	int count = sizeof(iconSizes)/sizeof(iconSizes[0]);
	for (int i = 0; i < count; i++) {
		if (abs(fIconHeight - iconSizes[i])
				< abs(fIconHeight - iconSizes[i+1])) {
			fIconHeight = iconSizes[i];
			break;
		}
	}
//	printf("After: fIconHeight: %i\n", fIconHeight);
}



void
MainWindow::_LaunchApp(MainListItem *item)
{
	entry_ref *ref = NULL;
	ref = item->Ref();

	if (ref != NULL) {
		status_t result = be_roster->Launch(ref);
		BString errorMessage;
		if (result != B_OK && result != B_ALREADY_RUNNING) {
			BString errStr(B_TRANSLATE_COMMENT("Failed to launch %appname%.\n\n"
			"Error:", "Don't translate the variable %appname%."));
			errStr.ReplaceFirst("%appname%", item->GetName());
			errorMessage << errStr.String() << " ";
			errorMessage << strerror(result);
		} else {
			// clear error message on success (might have been
			// filled when trying to launch by ref)
			errorMessage = "";
		}
		if (errorMessage.Length() > 0) {
			BAlert *alert = new BAlert("error", errorMessage.String(),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_FROM_WIDEST);
			alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
			alert->Go();
		}
	}
}
