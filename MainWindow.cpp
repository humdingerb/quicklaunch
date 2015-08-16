/*
 * Copyright 2010-2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "QLFilter.h"
#include "QuickLaunch.h"
#include "MainListItem.h"
#include "MainWindow.h"

#include <Catalog.h>
#include <ControlLook.h>
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


MainWindow::MainWindow(BRect frame)
	:
	BWindow(frame, B_TRANSLATE_SYSTEM_NAME("QuickLaunch"),
		B_TITLED_WINDOW_LOOK, B_MODAL_ALL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE
		| B_FRAME_EVENTS | B_CLOSE_ON_ESCAPE)
{
	QLApp *app = dynamic_cast<QLApp *> (be_app);
	BRect winframe = app->fSettings->GetMainWindowFrame();
	MoveTo(winframe.LeftTop());
	
	fSearchBox = new BTextControl("SearchBox", NULL,
							NULL, new BMessage(SEARCH_BOX));
							
	fSetupButton = new BButton ("Setup", B_TRANSLATE("Setup"),
		new BMessage(SETUP_BUTTON));
	fSetupButton->SetTarget(be_app);

	fListView = new MainListView();
	fScrollView = new BScrollView("ScrollList", fListView, B_FOLLOW_ALL_SIDES,
							false, true);
	
	// Build the layout
	float spacing = be_control_look->DefaultItemSpacing();
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fSearchBox)
			.AddStrut(spacing)
			.Add(fSetupButton)
			.SetInsets(spacing/2)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fScrollView)
			.SetInsets(spacing/2, 0, spacing/2, spacing/2)
		.End();

	fSearchBox->MakeFocus(true);

	AddCommonFilter(new QLFilter);
	fListView->SetInvocationMessage(new BMessage(RETURN_KEY));
	fListView->SetViewColor(B_TRANSPARENT_COLOR);

	if (app->fSettings->GetSaveSearch())
		fSearchBox->SetText(app->fSettings->GetSearchTerm());
}


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
						BStringItem *sItem = dynamic_cast<BStringItem *>
							(app->fSetupWindow->fIgnoreList->ItemAt(i));
						BString *ignoreItem = new BString(sItem->Text());

						if (newItem->ICompare(*ignoreItem, std::min(newItem->Length(),
								ignoreItem->Length())) == 0)
							ignore = true;
					}
				}
				if (!ignore)
					fListView->AddItem(new MainListItem(&entry));
			}
			query.Clear();
		}
	}
	fListView->SortItems(&compare_items);

	if (fListView->CountItems() < kMAX_DISPLAYED_ITEMS)
		ResizeTo(Bounds().Width(), (fListView->CountItems()) * (kBitmapSize + 4) + 85);
	else
		ResizeTo(Bounds().Width(), kMAX_DISPLAYED_ITEMS * (kBitmapSize + 4) + 90);
}


MainWindow::~MainWindow()
{
	QLApp *app = dynamic_cast<QLApp *> (be_app);
	app->fSettings->SetSearchTerm(GetSearchString());
}


bool
MainWindow::QuitRequested()
{
	return true;
}


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
			
			if (selection > kMAX_DISPLAYED_ITEMS)
				fListView->Select(selection - kMAX_DISPLAYED_ITEMS);
			else
				fListView->Select(first);

			fListView->ScrollToSelection();
			break;
		}
		case PAGE_DOWN:
		{
			int selection = fListView->CurrentSelection();
			int last = fListView->IndexOf(fListView->LastItem());
			
			if (selection < (fListView->CountItems() - kMAX_DISPLAYED_ITEMS)) {
				fListView->Select(selection + kMAX_DISPLAYED_ITEMS);
				fListView->ScrollBy(0.0, (kMAX_DISPLAYED_ITEMS)
					* (kBitmapSize + 9.0));
			}
			else
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
			entry_ref	*ref = NULL;
			MainListItem	*item = NULL;
			
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
				refMsg.AddRef("refs",&folderRef);
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
			char		string[512];
			entry_ref	*ref = NULL;
			status_t	result;
			MainListItem	*item = NULL;
			
			int selection = fListView->CurrentSelection();
			item = dynamic_cast<MainListItem *>(fListView->ItemAt(selection));
			if (item)
				ref = item->Ref();
				
			if (ref) {
			// if we got a ref, try launching it
				result = be_roster->Launch(ref);
				if (result != B_NO_ERROR) {
					sprintf(string, "Error launching: %s", strerror(result));
				}
			}
			break;
		}
		case RETURN_KEY:
		{
			Hide();

			char		string[512];
			entry_ref	*ref = NULL;
			status_t	result;
			MainListItem	*item = NULL;
			
			int selection = fListView->CurrentSelection();
			
			item = dynamic_cast<MainListItem *>(fListView->ItemAt(selection));
			if (item)
				ref = item->Ref();
				
			if (ref) {
			// if we got a ref, try launching it
			result = be_roster->Launch(ref);
				if (result != B_NO_ERROR) {
					sprintf(string, "Error launching: %s", strerror(result));
				}
			}
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
			}
			else {
				fListView->MakeEmpty();
				ResizeTo(Bounds().Width(), 90);		// original size
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
