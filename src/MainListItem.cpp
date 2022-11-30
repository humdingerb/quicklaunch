/*
 * Copyright 2010-2022. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *	Chris Roberts
 */

#include <Application.h>
#include <ControlLook.h>
#include <IconUtils.h>
#include <Resources.h>

#include "MainListItem.h"
#include "QuickLaunch.h"


MainListItem::MainListItem(BEntry* entry, int iconSize, bool isFav)
	:
	BListItem()
{
	fIconSize = iconSize;
	fIsFavorite = isFav;

	BNode node;
	BNodeInfo node_info;

	// try to get node info for this entry
	if ((node.SetTo(entry) == B_NO_ERROR) && (node_info.SetTo(&node) == B_NO_ERROR)) {

		// cache name and path
		entry->GetName(fName);
		entry->GetPath(&fPath);

		// create bitmap large enough for icon
		fIcon = new BBitmap(BRect(0, 0, fIconSize, fIconSize), 0, B_RGBA32);

		// cache the icon
		status_t result = node_info.GetIcon(fIcon, icon_size(fIconSize));
		if (result != B_OK) {
			BMimeType nodeType;
			nodeType.SetTo("application/x-vnd.Be-elfexecutable");
			result = nodeType.GetIcon(fIcon, icon_size(fIconSize));
			if (result != B_OK)
				fIcon = NULL;
		}

		// if it's a favorite, cache the star icon
		if (fIsFavorite)
			SetFavorite(true);

		// cache ref
		entry->GetRef(&fRef);

		// cache version info
		BFile file(entry, B_READ_ONLY);
		if (file.InitCheck() != B_OK)
			return;
		BAppFileInfo info(&file);
		if (info.InitCheck() != B_OK)
			return;
		if (info.GetVersionInfo(&fVersionInfo, B_APP_VERSION_KIND) != B_OK) {
			fVersionInfo.major = 0;
			fVersionInfo.middle = 0;
			fVersionInfo.minor = 0;
		}

	} else {
		fIcon = NULL;
		strcpy(fName, "<Lost File>");
	}
}


MainListItem::~MainListItem()
{
	delete fIcon;
	if (fIsFavorite)
		delete fFavoriteIcon;
}


#pragma mark-- BListItem Overrides --


void
MainListItem::DrawItem(BView* view, BRect rect, bool complete)
{
	float spacing = be_control_look->DefaultLabelSpacing();
	float offset = spacing;
	BFont appfont;
	BFont pathfont;
	font_height finfo;

	// set background color

	rgb_color bgColor;
	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else {
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);
		if (IsFavorite()) {
			rgb_color favColor = (rgb_color){255, 255, 0, 255};
			bgColor = mix_color(bgColor, favColor, 16);
		}
	}

	view->SetHighColor(bgColor);
	view->SetLowColor(bgColor);
	view->FillRect(rect);

	// if we have an icon, draw it

	if (fIcon) {
		view->PushState();
		view->SetDrawingMode(B_OP_OVER);
		view->DrawBitmap(
			fIcon, BPoint(rect.left + spacing / 2, rect.top + (rect.Height() - fIconSize) / 2));

		if (fIsFavorite) {
			view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
			view->DrawBitmap(fFavoriteIcon,
				BPoint(
					rect.left + spacing / 2 - 3, rect.top + (rect.Height() - fIconSize) / 2 + 4));
		}
		view->PopState();
		offset = fIcon->Bounds().Width() + offset + spacing;
	}

	// application name

	if (IsSelected())
		view->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		view->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));

	appfont.GetHeight(&finfo);
	appfont.SetFace(B_BOLD_FACE);
	view->SetFont(&appfont);

	QLSettings& settings = my_app->Settings();
	if (settings.Lock()) {
		if (settings.GetShowVersion() || settings.GetShowPath()) {
			view->MovePenTo(offset,
				rect.top + ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading)) / 2)
					+ (finfo.ascent + finfo.descent) - appfont.Size() + 2 + 3);
		} else {
			view->MovePenTo(offset,
				rect.top - 2
					+ ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading)) / 2)
					+ (finfo.ascent + finfo.descent));
		}

		float width, height;
		view->GetPreferredSize(&width, &height);
		BString string(fName);
		view->TruncateString(&string, B_TRUNCATE_MIDDLE, width - fIconSize - offset / 2);
		view->DrawString(string.String());

		// application path and version

		if (IsSelected())
			view->SetHighColor(tint_color(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR), 0.7));
		else
			view->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.6));

		pathfont.SetSize(appfont.Size() * 0.9);
		pathfont.GetHeight(&finfo);
		view->SetFont(&pathfont);

		view->MovePenTo(offset,
			rect.top + appfont.Size() - pathfont.Size() + 3
				+ ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading)) / 2)
				+ (finfo.ascent + finfo.descent));

		BPath parent;
		fPath.GetParent(&parent);
		string = "";

		char text[256];
		if (settings.GetShowVersion()) {
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.major);
			string << "v" << text << ".";
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.middle);
			string << text << ".";
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.minor);
			string << text;
		}
		if (settings.GetShowVersion() && settings.GetShowPath())
			string << " - ";

		if (settings.GetShowPath()) {
			string << parent.Path();
			string << "/";
		}

		view->TruncateString(&string, B_TRUNCATE_MIDDLE, width - fIconSize - offset / 2);
		view->DrawString(string.String());
		settings.Unlock();
	}
	// draw lines

	view->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
	view->StrokeLine(rect.LeftBottom(), rect.RightBottom());
}


void
MainListItem::Update(BView* owner, const BFont* finfo)
{
	// we need to override the update method so we can make sure the
	// list item size doesn't change
	BListItem::Update(owner, finfo);

	float spacing = be_control_look->DefaultLabelSpacing();
	SetHeight(fIcon->Bounds().Height() + spacing + 4);
}


void
MainListItem::SetFavorite(bool state)
{
	if (state) {
		size_t size;
		const void* buf
			= be_app->AppResources()->LoadResource(B_VECTOR_ICON_TYPE, "FavoriteStar", &size);

		if (buf != NULL) {
			fFavoriteIcon = new BBitmap(BRect(0, 0, fIconSize, fIconSize), B_RGBA32);
			BIconUtils::GetVectorIcon((const uint8*)buf, size, fFavoriteIcon);
		}
	}
	fIsFavorite = state;
}
