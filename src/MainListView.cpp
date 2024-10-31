/*
 * Copyright 2010-2019. All rights reserved.
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

class PopUpMenu : public BPopUpMenu
{
public:
				PopUpMenu(const char* name, BMessenger target);
	virtual 	~PopUpMenu();

private:
	BMessenger	fTarget;
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
	fShowingPopUpMenu(false),
	fPrimaryButton(false),
	fDropRect()
{
}


MainListView::~MainListView()
{
	MakeEmpty();
}


#pragma mark-- BListView Overrides --


void
MainListView::Draw(BRect rect)
{
	MainWindow* window = dynamic_cast<MainWindow*>(Window());
	int letters = window->GetStringLength();
	float width, height;
	BFont font;

	if (IsEmpty()) {
		SetLowColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		FillRect(rect);

		QLSettings& settings = my_app->Settings();
		BString string;

		if (settings.Lock()) {
			if (letters == 0)
				string = B_TRANSLATE("Use '*' as wildcards.");
			else
				string = B_TRANSLATE("Found no matches.");

			settings.Unlock();
		}
		float strwidth = font.StringWidth(string);
		GetPreferredSize(&width, &height);
		GetFont(&font);
		MovePenTo(width / 2 - strwidth / 2, height / 2 + font.Size() / 2);
		SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
		DrawString(string.String());
	} else {
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		BRect bounds(Bounds());
		BRect itemFrame = ItemFrame(CountItems() - 1);
		bounds.top = itemFrame.bottom;
		FillRect(bounds);
	}
	BListView::Draw(rect);

	// Only for Favorites == empty search field
	if (fDropRect.IsValid() && letters == 0) {
		SetHighColor(255, 0, 0, 255);
		StrokeRect(fDropRect);
	}
}


void
MainListView::FrameResized(float w, float h)
{
	BListView::FrameResized(w, h);

	for (int32 i = 0; i < CountItems(); i++) {
		BListItem* item = ItemAt(i);
		item->Update(this, be_plain_font);
	}
	Invalidate();
}


bool
MainListView::InitiateDrag(BPoint point, int32 dragIndex, bool wasSelected)
{
	BPoint pt;
	uint32 buttons;
	GetMouse(&pt, &buttons);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0)
		return false;

	MainListItem* sItem = dynamic_cast<MainListItem*>(ItemAt(dragIndex));

	entry_ref* ref = NULL;
	ref = sItem->Ref();
	if (ref == NULL)
		return false;

	BMessage message;
	message.what = B_SIMPLE_DATA;
	message.AddRef("refs", ref);
	message.AddInt32("index", dragIndex);
	message.AddBool("isfav", sItem->IsFavorite());

	BRect dragRect(0.0f, 0.0f, Bounds().Width(), sItem->Height());
	BBitmap* dragBitmap = new BBitmap(dragRect, B_RGB32, true);
	if (dragBitmap->IsValid()) {
		BView* view = new BView(dragBitmap->Bounds(), "helper", B_FOLLOW_NONE, B_WILL_DRAW);
		dragBitmap->AddChild(view);
		dragBitmap->Lock();

		sItem->DrawItem(view, dragRect, true);
		view->SetHighColor(0, 0, 0, 255);
		view->StrokeRect(view->Bounds());
		view->Sync();

		dragBitmap->Unlock();
	} else {
		delete dragBitmap;
		dragBitmap = NULL;
	}

	if (dragBitmap != NULL)
		DragMessage(&message, dragBitmap, B_OP_ALPHA, BPoint(0.0, 0.0));
	else
		DragMessage(&message, dragRect.OffsetToCopy(point), this);

	return true;
}


void
MainListView::MessageReceived(BMessage* message)
{
	QLSettings& settings = my_app->Settings();

	switch (message->what) {
		case POPCLOSED:
		{
			fShowingPopUpMenu = false;
			break;
		}
		case ADDFAVORITE:
		{
			fShowingPopUpMenu = false;
			entry_ref* ref = NULL;
			MainListItem* item = NULL;

			int selection = CurrentSelection();
			item = dynamic_cast<MainListItem*>(ItemAt(selection));

			if (item == NULL || item->IsFavorite())
				break;

			item->SetFavorite(true);

			ref = item->Ref();

			if (ref) {
				bool duplicate = false;

				if (settings.Lock()) {
					for (int i = 0; i < settings.fFavoriteList->CountItems(); i++) {
						entry_ref* favorite = settings.fFavoriteList->ItemAt(i);
						if (*ref == *favorite) {
							duplicate = true;
							break;
						}
					}
					if (!duplicate)
						settings.fFavoriteList->AddItem(new entry_ref(*ref));

					settings.Unlock();
				}
			}
			Invalidate();
			break;
		}
		case REMOVEFAVORITE:
		{
			fShowingPopUpMenu = false;
			entry_ref* ref = NULL;
			MainListItem* item = NULL;

			int selection = CurrentSelection();
			item = dynamic_cast<MainListItem*>(ItemAt(selection));

			if (item == NULL || !item->IsFavorite())
				break;

			MainWindow* window = dynamic_cast<MainWindow*>(Window());
			int letters = window->GetStringLength();

			ref = item->Ref();
			if (ref) {
				if (settings.Lock()) {
					for (int i = 0; i < settings.fFavoriteList->CountItems(); i++) {
						entry_ref* favorite = settings.fFavoriteList->ItemAt(i);
						if (*ref == *favorite) {
							delete settings.fFavoriteList->RemoveItemAt(i);
							break;
						}
					}
					settings.Unlock();
				}
			}

			item->SetFavorite(false);
			if (letters == 0) { // remove from result list
				delete RemoveItem(selection);
				Select((selection - 1 < 0) ? 0 : selection - 1);
			}

			Invalidate();
			window->ResizeWindow();
			break;
		}
		case ADDIGNORE:
		{
			fShowingPopUpMenu = false;
			entry_ref* ref = NULL;
			MainListItem* item = NULL;

			int selection = CurrentSelection();
			item = dynamic_cast<MainListItem*>(ItemAt(selection));

			if (item == NULL || item->IsFavorite())
				break;

			ref = item->Ref();

			if (ref) {
				BMessenger msgr(my_app->fSetupWindow);
				BMessage refMsg(B_REFS_RECEIVED);
				refMsg.AddRef("refs", ref);
				msgr.SendMessage(&refMsg);
			}
			break;
		}
		case OPENLOCATION:
		{
			fShowingPopUpMenu = false;

			BMessenger msgr(Window());
			BMessage refMsg(RETURN_CTRL_KEY);
			msgr.SendMessage(&refMsg);
			break;
		}
		case B_SIMPLE_DATA:
		{
			bool fav = false;
			if (message->FindBool("isfav", &fav) != B_OK)
				Window()->MessageReceived(message);

			if (!fav)
				break;

			// see if we're dragging a Favorite in a result list
			if (my_app->fMainWindow->GetStringLength() > 0)
				break;

			int32 origIndex;
			int32 dropIndex;
			BPoint dropPoint;

			if (message->FindInt32("index", &origIndex) != B_OK)
				origIndex = CountItems() - 1; // new Fav added at the bottom
			dropPoint = message->DropPoint();
			dropIndex = IndexOf(ConvertFromScreen(dropPoint));
			if (dropIndex > origIndex)
				dropIndex--;
			if (dropIndex < 0)
				dropIndex = CountItems() - 1; // move to bottom

			MoveItem(origIndex, dropIndex);
			Select(dropIndex);

			if (settings.Lock()) {
				settings.fFavoriteList->MoveItem(origIndex, dropIndex);
				settings.Unlock();
			}
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
	BRect bounds(Bounds());
	BRect itemFrame = ItemFrame(CountItems() - 1);
	bounds.top = itemFrame.bottom;
	if (bounds.Contains(position))
		return;

	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if (buttons == B_SECONDARY_MOUSE_BUTTON) {
		Select(IndexOf(position));
		_ShowPopUpMenu(ConvertToScreen(position));
		fPrimaryButton = false;
		Invalidate();
		return;
	}

	if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		fCurrentItemIndex = IndexOf(position);
		fPrimaryButton = true;
	}

	BListView::MouseDown(position);
}


void
MainListView::MouseUp(BPoint position)
{
	if ((fCurrentItemIndex == IndexOf(position) && fPrimaryButton == true)) {
		BMessenger msgr(Window());
		BMessage refMsg(SINGLE_CLICK);
		msgr.SendMessage(&refMsg);
		fPrimaryButton = false;
		fCurrentItemIndex = -1;
	}

	fDropRect = BRect(-1, -1, -1, -1);
	Invalidate();

	BListView::MouseUp(position);
}


void
MainListView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (dragMessage != NULL) {
		switch (transit) {
			case B_ENTERED_VIEW:
			case B_INSIDE_VIEW:
			{
				int32 index = IndexOf(where);
				if (index < 0)
					index = CountItems();

				fDropRect = ItemFrame(index);
				if (fDropRect.IsValid()) {
					fDropRect.top = fDropRect.top - 1;
					fDropRect.bottom = fDropRect.top + 1;
				} else {
					fDropRect = ItemFrame(index - 1);
					if (fDropRect.IsValid())
						fDropRect.top = fDropRect.bottom - 1;
					else {
						// empty view, show indicator at top
						fDropRect = Bounds();
						fDropRect.bottom = fDropRect.top + 1;
					}
				}
				Invalidate();
				break;
			}
			case B_EXITED_VIEW:
			{
				fDropRect = BRect(-1, -1, -1, -1);
				Invalidate();
				break;
			}
		}
	}
	BListView::MouseMoved(where, transit, dragMessage);
}


void
MainListView::MakeEmpty()
{
	// This breaks the contract of BListView, but we are the only ones
	// keeping track of the items.

	int32 count = CountItems();
	BObjectList<MainListItem> list(count, true);
	for (int32 i = 0; i < count; i++)
		list.AddItem(dynamic_cast<MainListItem*>(ItemAt(i)));

	BListView::MakeEmpty();
}


#pragma mark-- Private Methods --


void
MainListView::_ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu || IsEmpty())
		return;

	MainListItem* sItem = dynamic_cast<MainListItem*>(ItemAt(CurrentSelection()));

	bool isFav = false;
	if (sItem != NULL)
		isFav = sItem->IsFavorite();

	PopUpMenu* menu = new PopUpMenu("PopUpMenu", this);

	BMenuItem* item;

	if (isFav) {
		item = new BMenuItem(B_TRANSLATE("Remove favorite"), new BMessage(REMOVEFAVORITE), 'R');
	} else {
		item = new BMenuItem(B_TRANSLATE("Add to favorites"), new BMessage(ADDFAVORITE), 'F');
	}
	menu->AddItem(item);

	if (!isFav) {
		item = new BMenuItem(B_TRANSLATE("Add to ignore list"), new BMessage(ADDIGNORE), 'I');
		menu->AddItem(item);
	}

	item = new BMenuItem(B_TRANSLATE("Open containing folder"), new BMessage(OPENLOCATION), 'O');
	menu->AddItem(item);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
