/*
 * Copyright 2010-2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "MainListView.h"
#include "MainListItem.h"
#include "MainWindow.h"
#include "QLFilter.h"
#include "QuickLaunch.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ListView"

class PopUpMenu : public BPopUpMenu {
public:
				PopUpMenu(const char* name, BMessenger target);
	virtual 	~PopUpMenu();

private:
	BMessenger 	fTarget;
};


PopUpMenu::PopUpMenu(const char* name, BMessenger target)
	:
	BPopUpMenu(name, false, false),
	fTarget(target)
{
	SetAsyncAutoDestruct(true);
}


PopUpMenu::~PopUpMenu()
{
	fTarget.SendMessage(POPCLOSED);
}


MainListView::MainListView()
	:
	BListView("ResultList"),
	fShowingPopUpMenu(false)
{
}


MainListView::~MainListView()
{
}


void
MainListView::Draw(BRect rect)
{
	MainWindow *window = dynamic_cast<MainWindow *> (Window());
	int letters = window->GetStringLength();
	float width, height;
	BFont font;
	QLApp *app = dynamic_cast<QLApp *> (be_app);

	if (IsEmpty()) {
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		FillRect(rect);

		BString string;
		if (letters <= app->fSettings->GetDelay())
			string = B_TRANSLATE("Use '*' as wildcards.");
		else
			string = B_TRANSLATE("Found no matches.");

		float strwidth = font.StringWidth(string);
   		GetPreferredSize(&width, &height);
		GetFont(&font);
		MovePenTo(width / 2 - strwidth / 2, height / 2 + font.Size() / 2);
		SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
        DrawString(string.String());
	}
	else {
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		BRect bounds(Bounds());
		BRect itemFrame = ItemFrame(CountItems() - 1);
		bounds.top = itemFrame.bottom;
		FillRect(bounds);
	}
	BListView::Draw(rect);
}


void
MainListView::FrameResized(float w, float h)
{
	BListView::FrameResized(w, h);
	
	for (int32 i = 0; i < CountItems(); i++) {
		BListItem *item = ItemAt(i);
		item->Update(this, be_plain_font);
	}
	Invalidate();
}


void
MainListView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case POPCLOSED:
		{
			fShowingPopUpMenu = false;
			break;
		}
		case ADDIGNORE:
		{
			fShowingPopUpMenu = false;

			QLApp *app = dynamic_cast<QLApp *> (be_app);
			char		string[512];
			entry_ref	*ref = NULL;
			MainListItem	*item = NULL;

			int selection = app->fMainWindow->fListView->CurrentSelection();
			item = dynamic_cast<MainListItem *>
				(app->fMainWindow->fListView->ItemAt(selection));
			if (item)
				ref = item->Ref();

			if (ref) {
				BMessenger msgr(app->fSetupWindow);
				BMessage refMsg(B_REFS_RECEIVED);
				refMsg.AddRef("refs",ref);
				msgr.SendMessage(&refMsg);
			}
			break;
		}
		case OPENLOCATION:
		{
			fShowingPopUpMenu = false;

			QLApp *app = dynamic_cast<QLApp *> (be_app);
			BMessenger msgr(app->fMainWindow);
			BMessage refMsg(RETURN_CTRL_KEY);
			msgr.SendMessage(&refMsg);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
MainListView::MouseDown(BPoint position)
{
	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if (buttons == B_SECONDARY_MOUSE_BUTTON)
		ShowPopUpMenu(ConvertToScreen(position));

	BListView::MouseDown(position);
}


void
MainListView::ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu)
		return;

	PopUpMenu* menu = new PopUpMenu("PopUpMenu", this);

	BMenuItem* item = new BMenuItem(B_TRANSLATE("Add to ignore list"),
		new BMessage(ADDIGNORE));
	menu->AddItem(item);

	item = new BMenuItem(B_TRANSLATE("Open app's location"),
		new BMessage(OPENLOCATION));
	menu->AddItem(item);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
