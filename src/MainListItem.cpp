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


MainListItem::MainListItem(BEntry* entry, BString name, int iconSize, bool isFav)
	:
	BListItem(),
	fIsNoApp(false),
	fFavoriteIcon(NULL)
{
	fIconSize = iconSize;
	fIsFavorite = isFav;

	BNode node;
	BNodeInfo node_info;

	// try to get node info for this entry
	if ((node.SetTo(entry) == B_NO_ERROR) && (node_info.SetTo(&node) == B_NO_ERROR)) {

		// cache name and path
		snprintf(fName, sizeof(fName), "%s", name.String());
		entry->GetPath(&fPath);

		// check if the favorite is no application
		if (fIsFavorite) {
			char mimeString[B_MIME_TYPE_LENGTH];
			BMimeType nodeType;

			node_info.GetType(mimeString);
			if (strcasecmp(mimeString, "application/x-vnd.Be-elfexecutable") != 0) {
				fIsNoApp = true;
				// In case the non-App favorite is a link,
				// traverse to the source to get the right icon
				entry_ref followRef;
				entry->GetRef(&followRef);
				BEntry followLink(&followRef, true); // traverse link
				node.SetTo(&followLink);
				node_info.SetTo(&node);
			}
		}

		// create bitmap large enough for icon
		fIcon = new BBitmap(BRect(0, 0, fIconSize, fIconSize), 0, B_RGBA32);

		// cache the icon
		status_t result = node_info.GetIcon(fIcon, icon_size(fIconSize));
		if (result != B_OK) {
			char mimeString[B_MIME_TYPE_LENGTH];
			BMimeType nodeType;

			if (node_info.GetType(mimeString) != B_OK) {
				entry_ref ref;
				entry->GetRef(&ref);
				if (BMimeType::GuessMimeType(&ref, &nodeType) == B_OK) {
					strlcpy(mimeString, nodeType.Type(), B_MIME_TYPE_LENGTH);
					node_info.SetType(nodeType.Type());
				} else
					nodeType.SetTo("application/x-vnd.Be-elfexecutable");
			} else
				nodeType.SetTo(mimeString);

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
	delete fFavoriteIcon;
}


#pragma mark-- BListItem Overrides --


void
MainListItem::DrawItem(BView* view, BRect rect, bool complete)
{
	QLSettings& settings = my_app->Settings();
	bool showVersion = settings.GetShowVersion();
	bool showPath = settings.GetShowPath();

	float spacing = be_control_look->DefaultLabelSpacing();
	float offset = spacing;

	BFont appfont;
	font_height appFI;
	appfont.GetHeight(&appFI);
	appfont.SetFace(B_BOLD_FACE);

	BFont pathfont;
	font_height pathFI;
	pathfont.SetSize(appfont.Size() * 0.9);
	pathfont.GetHeight(&pathFI);

	// set background color

	rgb_color bgColor;
	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else {
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);
		if (IsFavorite()) {
			rgb_color favColor = (rgb_color){255, 255, 0, 32};
			rgb_color noAppColor = (rgb_color){0, 255, 0, 64};
			bgColor = blend_color(fIsNoApp == true ? noAppColor : favColor, bgColor, 80);
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

	view->SetFont(&appfont);

	if (!showPath && fIsNoApp) {
		view->MovePenTo(offset, floor(rect.top + appFI.ascent
		+ (rect.Height() + 1 - (appFI.ascent + appFI.descent))
		/ 2));
	} else if (showVersion || showPath) {
		view->MovePenTo(offset, floor(rect.top + appFI.ascent + 1
		+ (rect.Height() + 1 - (appFI.ascent + appFI.descent + pathFI.ascent + pathFI.descent))
		/ 2));
	} else {
		view->MovePenTo(offset, floor(rect.top + appFI.ascent
		+ (rect.Height() + 1 - (appFI.ascent + appFI.descent))
		/ 2));
	}

	float width, height;
	view->GetPreferredSize(&width, &height);
	BString string(fName);
	view->TruncateString(&string, B_TRUNCATE_MIDDLE, width - fIconSize - offset / 2);
	view->DrawString(string.String());

	// application path and version
	if (showVersion || showPath) {
		if (IsSelected()) {
			view->SetHighColor(tint_color(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR),
				ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR).IsDark() ?
				B_LIGHTEN_1_TINT : B_DARKEN_1_TINT));
		} else {
			view->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR),
				ui_color(B_LIST_ITEM_TEXT_COLOR).IsDark() ?
				B_LIGHTEN_1_TINT : B_DARKEN_1_TINT));
		}

		view->SetFont(&pathfont);
		view->MovePenTo(offset, floor(rect.top + appFI.ascent + 2 + pathFI.ascent
			+ (rect.Height() + 1
				- (appFI.ascent + appFI.descent + 1 + pathFI.ascent + pathFI.descent))
			/ 2));

		BPath parent;
		fPath.GetParent(&parent);
		string = "";

		char text[256];

		if (showVersion && !fIsNoApp) {
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.major);
			string << "v" << text << ".";
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.middle);
			string << text << ".";
			snprintf(text, sizeof(text), "%" B_PRId32, fVersionInfo.minor);
			string << text;
		}
		if (showVersion && showPath && !fIsNoApp)
			string << " - ";

		if (showPath) {
			string << parent.Path();
			string << "/";
		}

		view->TruncateString(&string, B_TRUNCATE_MIDDLE, width - fIconSize - offset / 2);
			view->DrawString(string.String());
	}
	// draw lines

	view->SetHighColor(tint_color(ui_color(B_LIST_BACKGROUND_COLOR),
			ui_color(B_LIST_BACKGROUND_COLOR).IsDark() ? B_LIGHTEN_1_TINT : B_DARKEN_1_TINT));
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
	if (state && fFavoriteIcon == NULL) {
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
