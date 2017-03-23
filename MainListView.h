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

#define ADDFAVORITE		'addf'
#define REMOVEFAVORITE	'rmfa'
#define ADDIGNORE		'addi'
#define FAV_DRAGGED		'fvdr'
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
	void			MouseUp(BPoint position);
	virtual	void 	MouseMoved(BPoint where, uint32 transit,
						const BMessage* dragMessage);

private:
	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
	bool			fPrimaryButton;
	int32			fCurrentItemIndex;
	BRect			fDropRect;

};

#endif // QLLISTVIEW_H
