/*
 * Copyright 2010-2022. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 *  Chris Roberts
 *
 * A graphical launch panel finding an app via a query.
 */

#include "DeskbarReplicant.h"
#include "QLFilter.h"
#include "QuickLaunch.h"

#include <AboutWindow.h>
#include <Deskbar.h>
#include <Catalog.h>
#include <storage/NodeMonitor.h>

const char* kApplicationSignature = "application/x-vnd.humdinger-quicklaunch";
const char* kApplicationName = "QuickLaunch";


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


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
	const char* authors[] = {
		"Humdinger",
		"Chris Roberts",
		"David Murphy",
		"Kevin Adams",
		NULL
	};
	BAboutWindow* aboutW = new BAboutWindow(kApplicationName, kApplicationSignature);
	aboutW->AddDescription(B_TRANSLATE(
		"QuickLaunch quickly starts any installed application. "
		"Just enter the first few letters of its name and choose "
		"from a list of all found programs."));
	aboutW->AddCopyright(2022, "Humdinger");
	aboutW->AddAuthors(authors);
	aboutW->Show();
}


void
QLApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case SETUP_BUTTON:
		{
			if (fSetupWindow->IsHidden()) {
				SetWindowsFeel(B_NORMAL_WINDOW_FEEL);
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

			if (!fMainWindow->fListView->IsEmpty())
				_RestorePositionAndSelection();

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
		case SORTFAVS_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);

			if (fSettings.Lock()) {
				fSettings.SetSortFavorites(value);
				fSettings.Unlock();
			}

			if (!fMainWindow->fListView->IsEmpty()) {
				fMainWindow->LockLooper();
				fMainWindow->BuildList();
				fMainWindow->UnlockLooper();
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
				_RestorePositionAndSelection();
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
			_RestorePositionAndSelection();
			break;
		}
		case B_NODE_MONITOR:
		{
			int32 opcode = message->GetInt32("opcode", -1);

			if ((opcode == B_DEVICE_MOUNTED) || (opcode == B_DEVICE_UNMOUNTED))
				_RestorePositionAndSelection();
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


void
QLApp::SetWindowsFeel(window_feel feel)
{
	fMainWindow->LockLooper();
	fMainWindow->SetFeel(feel);
	fMainWindow->UnlockLooper();
}


#pragma mark -- Private Methods --


void
QLApp::_AddToDeskbar()
{
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);

	BDeskbar deskbar;

	if (!deskbar.IsRunning())
		return;

	if (deskbar.HasItem(kApplicationName))
		_RemoveFromDeskbar();

	status_t res = deskbar.AddItem(&appInfo.ref);
	if (res != B_OK)
		printf("Failed adding deskbar icon: %" B_PRId32 "\n", res);
}


void
QLApp::_RemoveFromDeskbar()
{
	BDeskbar deskbar;
	int32 found_id;

	if (deskbar.GetItemInfo(kApplicationName, &found_id) == B_OK) {
		status_t err = deskbar.RemoveItem(found_id);
		if (err != B_OK) {
			printf("QuickLaunch: Error removing replicant id "
				"%" B_PRId32 ": %s\n", found_id, strerror(err));
		}
	}
}


void
QLApp::_RestorePositionAndSelection()
{
	fMainWindow->fListView->LockLooper();
	int32 selection = fMainWindow->fListView->CurrentSelection();
	float position = fMainWindow->GetScrollPosition();
	fMainWindow->BuildList();
	if (selection >= 0) {
		fMainWindow->fListView->Select((selection
			< fMainWindow->fListView->CountItems())
			? selection : fMainWindow->fListView->CountItems() - 1);
	} else if (!fMainWindow->fListView->IsEmpty())
		fMainWindow->fListView->Select(0);

	fMainWindow->SetScrollPosition(position);
	fMainWindow->fListView->UnlockLooper();
}


#pragma mark -- main --


int
main()
{
	QLApp app;
	app.Run();
	return 0;
}
