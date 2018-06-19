/*
 * Copyright 2010-2018. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 *
 * A graphical launch panel finding an app via a query.
 */

#include "DeskButton.h"
#include "QLFilter.h"
#include "QuickLaunch.h"

#include <Deskbar.h>
#include <Catalog.h>
#include <storage/NodeMonitor.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"

const char* kApplicationSignature = "application/x-vnd.humdinger-quicklaunch";


extern "C" _EXPORT BView* instantiate_deskbar_item();

extern "C" _EXPORT BView*
instantiate_deskbar_item()
{
	return new DeskButton();
}


QLApp::QLApp()
	:
	BApplication(kApplicationSignature)
{
	if (fSettings.GetDeskbar())	// make sure the replicant is shown
		_AddToDeskbar();

	fSetupWindow = new SetupWindow(fSettings.GetSetupWindowFrame());
	fMainWindow = new MainWindow();
}


QLApp::~QLApp()
{
	stop_watching(this);

	if (fSettings.Lock()) {
		fSettings.SetSearchTerm(fMainWindow->GetSearchString());
		fSettings.SaveSettings();
		fSettings.Unlock();
	}

	BMessenger messengerMain(fMainWindow);
	if (messengerMain.IsValid() && messengerMain.LockTarget())
		fMainWindow->Quit();

	BMessenger messengerSetup(fSetupWindow);
	if (messengerSetup.IsValid() && messengerSetup.LockTarget())
		fSetupWindow->Quit();
}


#pragma mark -- BApplication Overrides --


void
QLApp::AboutRequested()
{
	BString text = B_TRANSLATE_COMMENT(
		"QuickLaunch %version%\n"
		"\twritten by Humdinger\n"
		"\tCopyright %years%\n\n"
		"QuickLaunch quickly starts any installed application. "
		"Just enter the first few letters of its name and choose "
		"from a list of all found programs.\n",
		"Don't change the variables %years% and %version%.");
	text.ReplaceAll("%version%", kVersion);
	text.ReplaceAll("%years%", kCopyright);

	BAlert* alert = new BAlert("about", text.String(),
		B_TRANSLATE("Thank you"));

	BTextView* view = alert->TextView();
	BFont font;

	view->SetStylable(true);
	view->GetFont(&font);
	font.SetSize(font.Size()+4);
	font.SetFace(B_BOLD_FACE);
	view->SetFontAndColor(0, 11, &font);
	alert->Go();
}


void
QLApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case SETUP_BUTTON:
		{
			if (fSetupWindow->IsHidden()) {
				SetWindowsFeel(0);
				fSetupWindow->Show();
			} else
				fSetupWindow->Hide();
			break;
		}
		case HELP_BUTTON:
		{
			app_info info;
			BPath path;
			be_roster->GetActiveAppInfo(&info);
			BEntry entry(&info.ref);

			entry.GetPath(&path);
			path.GetParent(&path);
			path.Append("ReadMe.html");

			entry = path.Path();
			entry_ref ref;
			entry.GetRef(&ref);
			be_roster->Launch(&ref);
			break;
		}
		case DESKBAR_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetDeskbar(value);
				fSettings.Unlock();
			}

			if (value)
				_AddToDeskbar();
			else
				_RemoveFromDeskbar();
			break;
		}
		case VERSION_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetShowVersion(value);
				fSettings.Unlock();
			}

			if (!fMainWindow->fListView->IsEmpty()) {
				fMainWindow->LockLooper();
				fMainWindow->fListView->Invalidate();
				fMainWindow->UnlockLooper();
			}
			break;
		}
		case PATH_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetShowPath(value);
				fSettings.Unlock();
			}

			if (!fMainWindow->fListView->IsEmpty()) {
				fMainWindow->LockLooper();
				fMainWindow->fListView->Invalidate();
				fMainWindow->UnlockLooper();
			}
			break;
		}
		case SEARCHSTART_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetSearchStart(value);
				fSettings.Unlock();
			}

			if (!fMainWindow->fListView->IsEmpty()) {
				fMainWindow->LockLooper();
				fMainWindow->BuildList();
				fMainWindow->UnlockLooper();
			}
			break;
		}
		case DELAY_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetDelay(value);
				fSettings.Unlock();
			}

			fMainWindow->LockLooper();
			fMainWindow->PostMessage('fltr');
			fMainWindow->UnlockLooper();
			break;
		}
		case SAVESEARCH_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetSaveSearch(value);
				fSettings.Unlock();
			}
			break;
		}
		case SINGLECLICK_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			if (fSettings.Lock()) {
				fSettings.SetSingleClick(value);
				fSettings.Unlock();
			}
			break;
		}
		case ONTOP_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetOnTop(value);
				fSettings.Unlock();
			}
			break;
		}
		case IGNORE_CHK:
		{
			if (fSettings.fIgnoreList->IsEmpty()) {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(false);

				if (fSettings.Lock()) {
					fSettings.SetShowIgnore(false);
					fSettings.Unlock();
				}

				fSetupWindow->UnlockLooper();
				break;
			}
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetShowIgnore(value);
				fSettings.Unlock();
			}

			if (!fSettings.fIgnoreList->IsEmpty()) {
 				fSetupWindow->fChkIgnore->SetValue(value);
					fMainWindow->fListView->LockLooper();
					fMainWindow->BuildList();
					fMainWindow->fListView->UnlockLooper();
			}
			break;
		}
		case FILEPANEL:
		{
			if (!fSettings.fIgnoreList->IsEmpty()) {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(true);
				if (fSettings.Lock()) {
					fSettings.SetShowIgnore(true);
					fSettings.Unlock();
				}
				fSetupWindow->UnlockLooper();
			} else {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(false);
				if (fSettings.Lock()) {
					fSettings.SetShowIgnore(false);
					fSettings.Unlock();
				}
				fSetupWindow->UnlockLooper();
			}
			fMainWindow->fListView->LockLooper();
			int32 selection = fMainWindow->fListView->CurrentSelection();
			float position = fMainWindow->GetScrollPosition();
			fMainWindow->BuildList();
			fMainWindow->fListView->Select((selection
				< fMainWindow->fListView->CountItems())
				? selection : fMainWindow->fListView->CountItems() - 1);
			fMainWindow->SetScrollPosition(position);
			fMainWindow->fListView->UnlockLooper();
			break;
		}
		case B_NODE_MONITOR:
		{
			int32 opcode = message->GetInt32("opcode", -1);

			if ((opcode == B_DEVICE_MOUNTED)
				|| (opcode == B_DEVICE_UNMOUNTED)) {
				fMainWindow->fListView->LockLooper();
				float position = fMainWindow->GetScrollPosition();
				fMainWindow->BuildList();
				fMainWindow->SetScrollPosition(position);
				fMainWindow->fListView->UnlockLooper();
			}
			break;
		}
		default:
		{
			BApplication::MessageReceived(message);
			break;
		}
	}
}


bool
QLApp::QuitRequested()
{
	return true;
}


void
QLApp::ReadyToRun()
{
	BRect frame = fSettings.GetMainWindowFrame();
	fMainWindow->MoveTo(frame.LeftTop());
	fMainWindow->ResizeTo(frame.right - frame.left, 0);
	fMainWindow->Show();

	fSettings.InitLists();

	fMainWindow->fListView->LockLooper();
	fMainWindow->BuildList();
	fMainWindow->fListView->Select(0);
	fMainWindow->fListView->UnlockLooper();

	fSetupWindow->Hide();
	fSetupWindow->Show();

	if (fSettings.GetSaveSearch()) {
		BMessenger messenger(fMainWindow);
		BMessage message(NEW_FILTER);
		messenger.SendMessage(&message);
	}

	watch_node(NULL, B_WATCH_MOUNT, this);
}


#pragma mark -- Public Methods --


void
QLApp::SetWindowsFeel(int32 value)
{
	fMainWindow->LockLooper();
	fMainWindow->SetFeel(value
		? B_MODAL_ALL_WINDOW_FEEL : B_NORMAL_WINDOW_FEEL);
	fMainWindow->UnlockLooper();
}


#pragma mark -- Private Methods --


status_t
our_image(image_info& image)
{
	int32 cookie = 0;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &image) == B_OK) {
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}


void
QLApp::_AddToDeskbar()
{
	BDeskbar deskbar;

	if (deskbar.IsRunning() && !deskbar.HasItem("QuickLaunch")) {
		image_info info;
		entry_ref ref;

		if (our_image(info) == B_OK
			&& get_ref_for_path(info.name, &ref) == B_OK) {
			int32 id;
			status_t err = deskbar.AddItem(&ref, &id);

			if (err != B_OK) {
			printf("info_name: %s, ref_name: %s, id: %" B_PRId32
				"\nerr: %" B_PRId32 "\n",
				info.name, ref.name, id, err);
			}
		}
	}
}


void
QLApp::_RemoveFromDeskbar()
{
	BDeskbar deskbar;
	int32 found_id;

	if (deskbar.GetItemInfo("QuickLaunch", &found_id) == B_OK) {
		status_t err = deskbar.RemoveItem(found_id);
		if (err != B_OK) {
			printf("QuickLaunch: Error removing replicant id "
				"%" B_PRId32 ": %s\n", found_id, strerror(err));
		}
	}
}


#pragma mark -- main --


int
main()
{
	QLApp app;
	app.Run();
	return 0;
}
