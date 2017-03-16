/*
 * Copyright 2010-2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QLLISTVIEW_H
#define QLLISTVIEW_H

#include <ListView.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>

#include <stdio.h>

#define ADDIGNORE		'addi'
#define OPENLOCATION	'oloc'
#define POPCLOSED		'pmcl'


class MainListView : public BListView {
public:
					MainListView();
					~MainListView();

	virtual void	Draw(BRect rect);
	virtual	void	FrameResized(float w, float h);
	virtual bool	InitiateDrag(BPoint point, int32 index,
						bool wasSelected);
	virtual	void	MessageReceived(BMessage* message);
	void			MouseDown(BPoint position);

private:
	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
};

#endif // QLLISTVIEW_H
