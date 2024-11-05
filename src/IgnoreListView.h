/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdinger@mailbox.org
 */

#ifndef SETUP_LISTVIEW_H
#define SETUP_LISTVIEW_H

#include <ListView.h>
#include <PopUpMenu.h>
#include <String.h>


class IgnoreListView : public BListView {
public:
					IgnoreListView();
					~IgnoreListView();

	virtual void	Draw(BRect rect);
	virtual	void	FrameResized(float w, float h);
	virtual	void	KeyDown(const char* bytes, int32 numBytes);
	virtual	void	MessageReceived(BMessage* message);
	void			MouseDown(BPoint position);
	virtual void	SelectionChanged();
	virtual void	MakeEmpty();

private:
	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
};


class ContextPopUp : public BPopUpMenu {
public:
					ContextPopUp(const char* name, BMessenger target);
	virtual 		~ContextPopUp();

private:
	BMessenger 		fTarget;
};

#endif // SETUP_LISTVIEW_H
