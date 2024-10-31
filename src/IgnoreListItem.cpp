/*
 * Copyright 2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <ControlLook.h>
#include <sys/stat.h>

#include "IgnoreListItem.h"


IgnoreListItem::IgnoreListItem(BString item)
	:
	BListItem(),
	fItemString(item)
{
	fPath = new BPath(item.String());

	struct stat s;
	stat(fPath->Path(), &s);
	fIsDirectory = S_ISDIR(s.st_mode);
}


IgnoreListItem::~IgnoreListItem()
{
	delete fPath;
}


#pragma mark-- BListItem Overrides --


void
IgnoreListItem::DrawItem(BView* view, BRect rect, bool complete)
{
	// set background color

	rgb_color bgColor;

	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);

	view->SetHighColor(bgColor);
	view->SetLowColor(bgColor);
	view->FillRect(rect);

	// truncate and draw string

	if (IsSelected())
		view->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		view->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));

	float spacing = be_control_look->DefaultLabelSpacing();
	BFont font;
	font_height finfo;
	font.GetHeight(&finfo);

	if (fIsDirectory)
		font.SetFace(B_ITALIC_FACE);
	else
		font.SetFace(B_REGULAR_FACE);

	view->SetFont(&font);

	view->MovePenTo(spacing,
		rect.top - 2 + ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading)) / 2)
			+ (finfo.ascent + finfo.descent));

	float width, height;
	view->GetPreferredSize(&width, &height);
	BString string(fItemString);

	if (fIsDirectory)
		string << " *";

	view->TruncateString(&string, B_TRUNCATE_MIDDLE, width - spacing);
	view->DrawString(string.String());
}


#pragma mark-- Public Methods --


bool
IgnoreListItem::Ignores(const BString& path) const
{
	BString sPath = path;

	if (fIsDirectory) {
		BPath container = sPath.String();
		while (sPath.Length() > GetItem().Length()) {
			if (container.GetParent(&container) != B_OK)
				break;
			sPath = container.Path();
		}
	}

	return sPath.Compare(GetItem()) == 0;
}
