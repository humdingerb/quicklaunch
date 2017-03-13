/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 * A graphical launch panel finding an app via a query.
 */

#include "QLFilter.h"
#include "QuickLaunch.h"

#include <Catalog.h>

const char *kApplicationSignature = "application/x-vnd.humdinger-quicklaunch";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"

QLApp::QLApp()
	:
	BApplication(kApplicationSignature)
{
	fSettings = new QLSettings();
	fSetupWindow = new SetupWindow();
	fMainWindow = new MainWindow();
}


void
QLApp::ReadyToRun()
{
	fMainWindow->SetSizeLimits(190.0, 800.0, 90.0, 1000.0);
	BRect frame = fSettings->GetMainWindowFrame();
	fMainWindow->MoveTo(frame.LeftTop());
	fMainWindow->ResizeTo(frame.right - frame.left, 100.0);
	fMainWindow->Show();

	frame.OffsetBy(70.0, 120.0);
	fSetupWindow->MoveTo(frame.LeftTop());
	BRect bounds = fSettings->GetSetupWindowBounds();
	fSetupWindow->ResizeTo(bounds.Width(), bounds.Height());
	fSetupWindow->SetSizeLimits(340.0, 800.0, 160.0, 1000.0);
	fSettings->InitIgnoreList();
	fSetupWindow->Hide();
	fSetupWindow->Show();

	if (fSettings->GetSaveSearch()) {
		BMessenger messenger(fMainWindow);
		BMessage message(NEW_FILTER);
		messenger.SendMessage(&message);
	}
}


QLApp::~QLApp()
{
	BMessenger messengerMain(fMainWindow);
	if (messengerMain.IsValid() && messengerMain.LockTarget()) {
		fSettings->SetSearchTerm(fMainWindow->GetSearchString());
		fMainWindow->Quit();
	}
	delete fSettings;

	BMessenger messengerSetup(fSetupWindow);
	if (messengerSetup.IsValid() && messengerSetup.LockTarget())
		fSetupWindow->Quit();
}


void
QLApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case SETUP_BUTTON:
		{
			if (fSetupWindow->IsHidden()) {
				SetWindowsFeel(0);
				BRect frame = fMainWindow->Frame();
				frame.OffsetBy(70.0, 120.0);
				fSetupWindow->MoveTo(frame.LeftTop());
				fSetupWindow->Show();
			}
			else {
				fSetupWindow->Hide();
			}
			break;
		}
		case VERSION_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetShowVersion(value);
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
			fSettings->SetShowPath(value);
			if (!fMainWindow->fListView->IsEmpty()) {
				fMainWindow->LockLooper();
				fMainWindow->fListView->Invalidate();
				fMainWindow->UnlockLooper();
			}
			break;
		}
		case DELAY_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetDelay(value);
			fMainWindow->LockLooper();
			fMainWindow->PostMessage('fltr');
			fMainWindow->UnlockLooper();
			break;
		}
		case SAVESEARCH_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetSaveSearch(value);
			break;
		}
		case SINGLECLICK_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetSingleClick(value);
			break;
		}
		case ONTOP_CHK:
		{
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetOnTop(value);
			break;
		}
		case IGNORE_CHK:
		{
			if (fSetupWindow->fIgnoreList->IsEmpty()) {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(false);
				fSettings->SetShowIgnore(false);
				fSetupWindow->UnlockLooper();
				break;
			}
			int32 value;
			message->FindInt32("be:value", &value);
			fSettings->SetShowIgnore(value);
			if (!fSetupWindow->fIgnoreList->IsEmpty()) {
 				fSetupWindow->fChkIgnore->SetValue(value);
				if (!fMainWindow->fListView->IsEmpty()) {
	 				fMainWindow->fListView->LockLooper();
	 				const char *searchString = fMainWindow->GetSearchString();
					fMainWindow->BuildList(searchString);
					fMainWindow->fListView->UnlockLooper();
				}
			}
			break;
		}
		case FILEPANEL:
		{
			if (!fSetupWindow->fIgnoreList->IsEmpty()) {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(true);
				fSettings->SetShowIgnore(true);
				fSetupWindow->UnlockLooper();
			} else {
				fSetupWindow->LockLooper();
				fSetupWindow->fChkIgnore->SetValue(false);
				fSettings->SetShowIgnore(false);
				fSetupWindow->UnlockLooper();
			}
			if (fMainWindow->GetStringLength() > 0) {

				fMainWindow->fListView->LockLooper();
				float position = fMainWindow->GetScrollPosition();
				const char *searchString = fMainWindow->GetSearchString();
				fMainWindow->BuildList(searchString);
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


void
QLApp::SetWindowsFeel(int32 value)
{
	fMainWindow->LockLooper();
	fMainWindow->SetFeel(value ?
		B_MODAL_ALL_WINDOW_FEEL : B_NORMAL_WINDOW_FEEL);
	fMainWindow->UnlockLooper();
}


bool
QLApp::QuitRequested()
{
	return true;
}


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
	text.ReplaceAll("%version%", "v0.9.13");
	text.ReplaceAll("%years%", "2010-2017");

	BAlert *alert = new BAlert("about", text.String(),
		B_TRANSLATE("Thank you"));

	BTextView *view = alert->TextView();
	BFont font;

	view->SetStylable(true);
	view->GetFont(&font);
	font.SetSize(font.Size()+4);
	font.SetFace(B_BOLD_FACE);
	view->SetFontAndColor(0, 11, &font);
	alert->Go();
}


int
main()
{
	QLApp app;
	app.Run();
	return 0;
}
