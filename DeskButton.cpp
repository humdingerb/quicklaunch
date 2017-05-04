/*
 * Copyright 2003-2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Jérôme Duval
 *		Jonas Sundström
 *
 * Simplified by Humdinger, 2017
 */


#include "DeskButton.h"
#include "QuickLaunch.h"

#include <Bitmap.h>
#include <Catalog.h>
#include <Message.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Roster.h>

#include <image.h>

#define OPEN_REF	'opre'

// from QuickLaunch.cpp
extern const char* kApplicationSignature;
extern status_t our_image(image_info& image);

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"

DeskButton::DeskButton(BRect frame, entry_ref* ref, const char* name,
		uint32 resizeMask, uint32 flags)
	:
	BView(frame, name, resizeMask, flags),
	fRef(*ref)
{
	fIcon = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);
	BNodeInfo::GetTrackerIcon(&fRef, fIcon, B_MINI_ICON);
}


DeskButton::DeskButton(BMessage* message)
	:
	BView(message)
{
	message->FindRef("ref", &fRef);
	
	fIcon = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);
	BNodeInfo::GetTrackerIcon(&fRef, fIcon, B_MINI_ICON);
}


DeskButton::DeskButton()
	:
	BView(BRect(0, 0, 15, 15), "QuickLaunch", B_FOLLOW_NONE, B_WILL_DRAW)
{
	image_info info;

	if (our_image(info) == B_OK
			&& get_ref_for_path(info.name, &fRef) == B_OK) {
		fIcon = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);
		BNodeInfo::GetTrackerIcon(&fRef, fIcon, B_MINI_ICON);
	}
}


DeskButton::~DeskButton()
{
	delete fIcon;
}


// archiving overrides
DeskButton*
DeskButton::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "DeskButton"))
		return NULL;

	return new DeskButton(data);
}


void
DeskButton::AboutRequested()
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


status_t 
DeskButton::Archive(BMessage* data, bool deep) const
{
	BView::Archive(data, deep);
	
	data->AddRef("ref", &fRef);
	data->AddString("add_on", kApplicationSignature);
	return B_NO_ERROR;
}


void
DeskButton::AttachedToWindow()
{
	BView* parent = Parent();
	if (parent)
		SetViewColor(parent->ViewColor());

	BView::AttachedToWindow();
}


void 
DeskButton::Draw(BRect rect)
{
	BView::Draw(rect);

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	DrawBitmap(fIcon);
}


void
DeskButton::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_ABOUT_REQUESTED:
		{
			AboutRequested();
			break;
		}
		case OPEN_REF:
		{
			entry_ref ref;
			message->FindRef("refs", &ref);
			be_roster->Launch(&ref);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
DeskButton::MouseDown(BPoint point)
{
	uint32 mouseButtons = 0;
	if (Window()->CurrentMessage() != NULL)
		mouseButtons = Window()->CurrentMessage()->FindInt32("buttons");

	BPoint where = ConvertToScreen(point);

	if (mouseButtons & B_SECONDARY_MOUSE_BUTTON) {
		_GetFavoriteList();

		BPopUpMenu* menu = new BPopUpMenu("", false, false);
		menu->SetFont(be_plain_font);

		if (!fFavoriteList->IsEmpty()) {
			for (int i = 0; i < fFavoriteList->CountItems(); i++)
			{
				entry_ref* favorite = static_cast<entry_ref *>
					(fFavoriteList->ItemAt(i));
				BMessage* message = new BMessage(OPEN_REF);
				message->AddRef("refs", favorite);
				menu->AddItem(new BMenuItem(favorite->name, message));
			}
		menu->AddSeparatorItem();
		}
		BMessage* message = new BMessage(OPEN_REF);
		message->AddRef("refs", &fRef);
		menu->AddItem(new BMenuItem(B_TRANSLATE("Open QuickLaunch"), message));
		menu->AddItem(new BMenuItem(B_TRANSLATE("About QuickLaunch"),
			new BMessage(B_ABOUT_REQUESTED)));

		menu->SetTargetForItems(this);
		menu->Go(where, true, true, BRect(where - BPoint(4, 4), 
			where + BPoint(4, 4)));

		delete fFavoriteList;

	} else if (mouseButtons & B_PRIMARY_MOUSE_BUTTON)
		be_roster->Launch(&fRef);
}


void
DeskButton::_GetFavoriteList()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append("QuickLaunch_settings");
	BFile file(path.Path(), B_READ_ONLY);

	fFavoriteList = new BList();

	BMessage settings;
	if (file.InitCheck() == B_OK && settings.Unflatten(&file) == B_OK) {
		int32 i = 0;
		BString itemText;
		while (settings.FindString("favorite", i++, &itemText) == B_OK) {
			entry_ref favorite;
			status_t err = get_ref_for_path(itemText.String(), &favorite);
			if (err == B_OK)
				fFavoriteList->AddItem(new entry_ref(favorite));
		}
	}
}
