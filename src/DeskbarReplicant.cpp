/*
 * Copyright 2009-2021, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Weirauch, dev@m-phasis.de
 *
 * Adapted from Haiku's Bluetooth server by Humdinger, 2022
 *  Contributers:
 *	Máximo Castañeda
 */


#include "DeskbarReplicant.h"
#include "QuickLaunch.h"

#include <AboutWindow.h>
#include <Application.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <IconUtils.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <Roster.h>


extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth, float maxHeight);
status_t our_image(image_info& image);

// from QuickLaunch.cpp
extern const char* kApplicationSignature;
extern const char* kApplicationName;

const char* kClassName = "DeskbarReplicant";

#define OPEN_REF 'opre'
#define OPEN_QL 'opql'


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


//	#pragma mark -


DeskbarReplicant::DeskbarReplicant(BRect frame, int32 resizingMode)
	:
	BView(frame, kApplicationName, resizingMode,
		B_WILL_DRAW | B_TRANSPARENT_BACKGROUND | B_FRAME_EVENTS)
{
	_Init();
}


DeskbarReplicant::DeskbarReplicant(BMessage* archive)
	:
	BView(archive)
{
	_Init();
}


DeskbarReplicant::~DeskbarReplicant()
{
}


void
DeskbarReplicant::_Init()
{
	fIcon = NULL;

	image_info info;
	if (our_image(info) != B_OK)
		return;

	BFile file(info.name, B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return;

	BResources resources(&file);
	if (resources.InitCheck() < B_OK)
		return;

	size_t size;
	const void* data = resources.LoadResource(B_VECTOR_ICON_TYPE, "tray_icon", &size);
	if (data != NULL) {
		BBitmap* icon = new BBitmap(Bounds(), B_RGBA32);
		if (icon->InitCheck() == B_OK
			&& BIconUtils::GetVectorIcon((const uint8*)data, size, icon) == B_OK)
			fIcon = icon;
		else
			delete icon;
	}
}


DeskbarReplicant*
DeskbarReplicant::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, kClassName))
		return NULL;

	return new DeskbarReplicant(archive);
}


status_t
DeskbarReplicant::Archive(BMessage* archive, bool deep) const
{
	status_t status = BView::Archive(archive, deep);
	if (status == B_OK)
		status = archive->AddString("add_on", kApplicationSignature);
	if (status == B_OK)
		status = archive->AddString("class", kClassName);

	return status;
}


void
DeskbarReplicant::AboutRequested()
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
DeskbarReplicant::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();

	if (ViewUIColor() == B_NO_COLOR)
		SetLowColor(ViewColor());
	else
		SetLowUIColor(ViewUIColor());
}


void
DeskbarReplicant::Draw(BRect updateRect)
{
	if (!fIcon) {
		/* At least display something... */
		rgb_color lowColor = LowColor();
		SetLowColor(0, 113, 187, 255);
		FillRoundRect(Bounds().InsetBySelf(3.f, 0.f), 5.f, 7.f, B_SOLID_LOW);
		SetLowColor(lowColor);
	} else {
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fIcon);
		SetDrawingMode(B_OP_COPY);
	}
}


void
DeskbarReplicant::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_ABOUT_REQUESTED:
		{
			AboutRequested();
			break;
		}
		case OPEN_QL:
		{
			be_roster->Launch(kApplicationSignature);
			break;
		}
		case OPEN_REF:
		{
			entry_ref ref;
			msg->FindRef("refs", &ref);
			be_roster->Launch(&ref);
			break;
		}
		default:
			BView::MessageReceived(msg);
			break;
	}
}


void
DeskbarReplicant::MouseDown(BPoint where)
{
	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);

	if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		BObjectList<entry_ref>* favoriteList = _GetFavoriteList();

		BPopUpMenu* menu = new BPopUpMenu("", false, false);
		menu->SetFont(be_plain_font);

		if (favoriteList != NULL && !favoriteList->IsEmpty()) {
			bool localized = BLocaleRoster::Default()->IsFilesystemTranslationPreferred();
			for (int i = 0; i < favoriteList->CountItems(); i++) {
				entry_ref* favorite = favoriteList->ItemAt(i);
				BMessage* message = new BMessage(OPEN_REF);
				message->AddRef("refs", favorite);
				BString appName;
				if (!localized
					|| BLocaleRoster::Default()->GetLocalizedFileName(appName, *favorite) != B_OK)
					appName = favorite->name;
				menu->AddItem(new BMenuItem(appName, message));
			}
			menu->AddSeparatorItem();
		}
		menu->AddItem(new BMenuItem(B_TRANSLATE("Open QuickLaunch"), new BMessage(OPEN_QL)));
		menu->AddItem(
			new BMenuItem(B_TRANSLATE("About QuickLaunch"), new BMessage(B_ABOUT_REQUESTED)));

		menu->SetTargetForItems(this);
		ConvertToScreen(&point);
		menu->Go(point, true, true, BRect(where - BPoint(4, 4), point + BPoint(4, 4)));

		delete favoriteList;
		delete menu;
	} else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		be_roster->Launch(kApplicationSignature);
}


BObjectList<entry_ref>*
DeskbarReplicant::_GetFavoriteList()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;

	path.Append("QuickLaunch_settings");
	BFile file(path.Path(), B_READ_ONLY);

	BObjectList<entry_ref>* favoriteList = new BObjectList<entry_ref>(20, true);

	BMessage settings;
	if (file.InitCheck() == B_OK && settings.Unflatten(&file) == B_OK) {
		int32 i = 0;
		BString itemText;
		while (settings.FindString("favorite", i++, &itemText) == B_OK) {
			entry_ref favorite;
			status_t err = get_ref_for_path(itemText.String(), &favorite);
			if (err == B_OK)
				favoriteList->AddItem(new entry_ref(favorite));
		}
	}

	return favoriteList;
}


//	#pragma mark -


extern "C" _EXPORT BView*
instantiate_deskbar_item(float maxWidth, float maxHeight)
{
	return new DeskbarReplicant(BRect(0, 0, maxHeight - 1, maxHeight - 1), B_FOLLOW_NONE);
}


//	#pragma mark -


status_t
our_image(image_info& image)
{
	int32 cookie = 0;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &image) == B_OK) {
		if ((char*)our_image >= (char*)image.text
			&& (char*)our_image <= (char*)image.text + image.text_size)
			return B_OK;
	}
	BAlert* alert = new BAlert("image", "Image NOT OK", "NOT");
	alert->Show();
	return B_ERROR;
}
