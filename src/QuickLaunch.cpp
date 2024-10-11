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

#include "QuickLaunch.h"
#include "QLFilter.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <PathFinder.h>
#include <storage/NodeMonitor.h>

const char* kApplicationSignature = "application/x-vnd.humdinger-quicklaunch";
const char* kApplicationName = "QuickLaunch";


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


QLApp::QLApp()
	:
	BApplication(kApplicationSignature),
	fSetupWindow(NULL),
	fMainWindow(NULL)
{
	// Check if user's Shortcuts have the old QL location
	// ToDo: Remove some time after R1beta5
	if (_OpenShortcutPrefs())
		return;

	if (fSettings.GetDeskbar()) // make sure the replicant is shown
		_AddToDeskbar();

	fSettings.InitLists();

	fSetupWindow = new SetupWindow(fSettings.GetSetupWindowFrame());
	fMainWindow = new MainWindow();
}


QLApp::~QLApp()
{
	stop_watching(this);

	if (fMainWindow != NULL) {
		BMessenger messengerMain(fMainWindow);
		if (messengerMain.IsValid() && messengerMain.LockTarget())
			fMainWindow->Quit();
	}
	if (fSetupWindow != NULL) {
		BMessenger messengerSetup(fSetupWindow);
		if (messengerSetup.IsValid() && messengerSetup.LockTarget())
			fSetupWindow->Quit();
	}
}


#pragma mark-- BApplication Overrides --


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
		"from a list of all found applications."));
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
			_OpenHelp();
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
				fMainWindow->FilterAppList();
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
				fMainWindow->BuildAppList();
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
			fMainWindow->BuildAppList();
			_RestorePositionAndSelection();
			break;
		}
		case B_NODE_MONITOR:
		{
			int32 opcode = message->GetInt32("opcode", -1);

			if ((opcode == B_DEVICE_MOUNTED) || (opcode == B_DEVICE_UNMOUNTED)) {
				fMainWindow->BuildAppList();
				_RestorePositionAndSelection();
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

	fMainWindow->fListView->LockLooper();
	fMainWindow->FilterAppList();
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


#pragma mark-- Private Methods --


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
				   "%" B_PRId32 ": %s\n",
				found_id, strerror(err));
		}
	}
}


void
QLApp::_RestorePositionAndSelection()
{
	fMainWindow->fListView->LockLooper();
	int32 selection = fMainWindow->fListView->CurrentSelection();
	float position = fMainWindow->GetScrollPosition();
	fMainWindow->FilterAppList();
	if (selection >= 0) {
		fMainWindow->fListView->Select((selection < fMainWindow->fListView->CountItems())
				? selection
				: fMainWindow->fListView->CountItems() - 1);
	} else if (!fMainWindow->fListView->IsEmpty())
		fMainWindow->fListView->Select(0);

	fMainWindow->SetScrollPosition(position);
	fMainWindow->fListView->UnlockLooper();
}


void
QLApp::_OpenHelp()
{
	BPathFinder pathFinder;
	BStringList paths;
	BPath path;
	BEntry entry;

	status_t error = pathFinder.FindPaths(B_FIND_PATH_DOCUMENTATION_DIRECTORY,
		"packages/quicklaunch", paths);

	for (int i = 0; i < paths.CountStrings(); ++i) {
		if (error == B_OK && path.SetTo(paths.StringAt(i)) == B_OK
			&& path.Append("ReadMe.html") == B_OK) {
			entry = path.Path();
			if (!entry.Exists())
				continue;
			entry_ref ref;
			entry.GetRef(&ref);
			be_roster->Launch(&ref);
		}
	}
}


bool
QLApp::_OpenShortcutPrefs()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return false;

	path.Append("shortcuts_settings");
	BFile file(path.Path(), B_READ_ONLY);

	BMessage settings;
	if (file.InitCheck() != B_OK || settings.Unflatten(&file) != B_OK)
		return false;

	int32 i = 0;
	BMessage specMsg;
	BString command;
	bool found = false;
	while (settings.FindMessage("spec", i, &specMsg) == B_OK) {
		if (specMsg.FindString("command", &command) == B_OK) {
			if (command.FindFirst("/apps/QuickLaunch/QuickLaunch") != B_ERROR) {
				found = true;
				break;
			}
		i++;
		}
	}

	if (!found)
		return false;

	BAlert* alert = new BAlert("Found old Shortcuts preference entry", B_TRANSLATE(
		"The Shortcuts preferences appear to have a shortcut that points to the old "
		"location of QuickLaunch.\n\n"
		"Do you want to open the Shortcuts preferences so you can set the correct location: "
		"'/boot/system/apps/QuickLaunch' ?"),
		B_TRANSLATE("Cancel"), B_TRANSLATE("Open Shortcuts"));
	alert->SetShortcut(1, B_ESCAPE);
	int32 button = alert->Go();

	if (button == 0)
		return false;
	else {
		be_roster->Launch("application/x-vnd.Haiku-Shortcuts");
		PostMessage(B_QUIT_REQUESTED);
		return true;
	}
}


#pragma mark-- main --


int
main()
{
	QLApp app;
	app.Run();
	return 0;
}
