/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QLLISTITEM_H
#define QLLISTITEM_H

#include <AppFileInfo.h>
#include <Bitmap.h>
#include <Entry.h>
#include <File.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <ListItem.h>
#include <MenuItem.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


class MainListItem : public BListItem {
public:
					MainListItem(BEntry* entry, BString name, int iconSize,	bool isFav = false);
					~MainListItem();

	virtual void	DrawItem(BView*, BRect, bool);
	virtual	void	Update(BView*, const BFont*);

	BBitmap*		Bitmap() { return fIcon; };
	char*			GetName() { return fName; };
	entry_ref*		Ref() { return &fRef; };
	bool			IsFavorite() { return fIsFavorite; };
	void			SetFavorite(bool state);
		
private:
	char			fName[B_FILE_NAME_LENGTH];
	entry_ref		fRef;
	BPath			fPath;
	version_info	fVersionInfo;
	BBitmap*		fIcon;
	BBitmap*		fFavoriteIcon;
	int				fIconSize;
	bool			fIsFavorite;
};

#endif // QLLISTITEM_H
