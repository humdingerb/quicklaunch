/*
 * Copyright 2003-2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Jérôme Duval
 */
#ifndef DESK_BUTTON_H
#define DESK_BUTTON_H


#include <View.h>
#include <Entry.h>


class DeskButton
	: public BView {
public:
	DeskButton(BRect frame, entry_ref* ref, const char* name,
		uint32 resizeMask = B_FOLLOW_ALL, 
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	DeskButton(BMessage* archive);
	DeskButton();
	virtual ~DeskButton();

	// archiving overrides
	static DeskButton* Instantiate(BMessage* data);
	virtual	status_t Archive(BMessage* data, bool deep = true) const;

	// misc BView overrides
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage* message);
	virtual void MouseDown(BPoint);
	virtual void Draw(BRect updateRect);

private:
	void		_GetFavoriteList();

	BBitmap*	fIcon;
	entry_ref	fRef;
	BList*		fFavoriteList;
};

#endif	// DESK_BUTTON_H
