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
#include <Message.h>
#include <NodeInfo.h>
#include <Roster.h>

#include <image.h>

extern const char *kApplicationSignature;
	// from QuickLaunch.cpp


DeskButton::DeskButton(BRect frame, entry_ref* ref, const char* name,
		uint32 resizeMask, uint32 flags)
	:
	BView(frame, name, resizeMask, flags),
	fRef(*ref)
{
	fIcon = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);
	BNodeInfo::GetTrackerIcon(&fRef, fIcon, B_MINI_ICON);
}


DeskButton::DeskButton(BMessage *message)
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
	// Black magic by AnEvilYak to avoid using the app_signature
	// with be_roster->FindApp(kApplicationSignature, &fRef);
	int32 cookie = 0;
	image_info info;

	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		if ((addr_t)B_CURRENT_IMAGE_SYMBOL < (addr_t)info.text
				|| (addr_t)B_CURRENT_IMAGE_SYMBOL
				> (addr_t)info.text + (addr_t)info.text_size)
			continue;

		get_ref_for_path(info.name, &fRef);
		break;
	}

	fIcon = new BBitmap(BRect(0, 0, 15, 15), B_RGBA32);
	BNodeInfo::GetTrackerIcon(&fRef, fIcon, B_MINI_ICON);
}


DeskButton::~DeskButton()
{
	delete fIcon;
}


// archiving overrides
DeskButton *
DeskButton::Instantiate(BMessage *data)
{
	if (!validate_instantiation(data, "DeskButton"))
		return NULL;

	return new DeskButton(data);
}


status_t 
DeskButton::Archive(BMessage *data, bool deep) const
{
	BView::Archive(data, deep);
	
	data->AddRef("ref", &fRef);
	data->AddString("add_on", kApplicationSignature);
	return B_NO_ERROR;
}


void
DeskButton::AttachedToWindow()
{
	BView *parent = Parent();
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
DeskButton::MouseDown(BPoint point)
{
	be_roster->Launch(&fRef);
}
