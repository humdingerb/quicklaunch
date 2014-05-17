/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QLLISTITEM_H
#define QLLISTITEM_H

#include <MenuItem.h>
#include <ListItem.h>
#include <Font.h>
#include <Bitmap.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <Node.h>
#include <NodeInfo.h>
#include <String.h>
#include <InterfaceDefs.h>
#include <AppFileInfo.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


const int kBitmapSize = 32;

#define kDEFAULT_ITEM_HEIGHT	(kBitmapSize + 15)
#define kITEM_MARGIN			 10


class MainListItem : public BListItem {
public:
					MainListItem(BEntry* entry);
					~MainListItem();
	virtual void	DrawItem(BView*, BRect, bool);
	virtual	void	Update(BView*, const BFont*);
	BBitmap*		Bitmap() {return fIcon;};
	entry_ref*		Ref() {return &fRef;};
	char*			GetName() {return fName;};
		
private:
	char			fName[B_FILE_NAME_LENGTH];
	entry_ref		fRef;
	BPath			fPath;
	version_info	fVersionInfo;
	BBitmap			*fIcon;
};

#endif // QLLISTITEM_H
