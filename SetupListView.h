/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef SETUP_LISTVIEW_H
#define SETUP_LISTVIEW_H

#include <ListView.h>
#include <PopUpMenu.h>
#include <String.h>

#include <stdio.h>

class SetupListView : public BListView {
public:
					SetupListView();
					~SetupListView();

	virtual void	Draw(BRect rect);
	virtual	void	FrameResized(float w, float h);
	virtual	void	KeyDown(const char* bytes, int32 numBytes);
	virtual	void	MessageReceived(BMessage* message);
	void			MouseDown(BPoint position);
	virtual void	SelectionChanged();

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
