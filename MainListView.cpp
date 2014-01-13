/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "MainListView.h"
#include "MainWindow.h"

MainListView::MainListView()
		  : BListView(BRect(), "ResultList", B_SINGLE_SELECTION_LIST, B_WILL_DRAW)
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
	if (IsEmpty() && letters > 0) {
		BString *string = new BString("Found no matches.");
   		float strwidth = font.StringWidth("Found no matches.");
   		GetPreferredSize(&width, &height);
		GetFont(&font);
		MovePenTo(width / 2 - strwidth / 2, height / 2 + font.Size() / 2);
		SetHighColor(ui_color(B_FAILURE_COLOR));
        DrawString(string->String());
		delete string;
	}
	else if (IsEmpty() && letters == 0) {
		BString *string = new BString("Use '*' as wildcards.");
   		float strwidth = font.StringWidth("Use '*' as wildcards.");
   		GetPreferredSize(&width, &height);
		GetFont(&font);
		MovePenTo(width / 2 - strwidth / 2, height / 2 + font.Size() / 2);
		SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
        DrawString(string->String());
		delete string;
	}
	else {
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		FillRect(rect);
		BListView::Draw(rect);
	}
}


void
MainListView::FrameResized(float w, float h)
{
	BListView::FrameResized(w, h);
	
	for (int32 i = 0; i < CountItems(); i++) {
		BListItem *item = ItemAt(i);
		item->Update(this,be_plain_font);
	}
	Invalidate();
}
