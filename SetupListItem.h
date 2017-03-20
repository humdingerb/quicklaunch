/*
 * Copyright 2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef SETUPLISTITEM_H
#define SETUPLISTITEM_H

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


class SetupListItem : public BListItem {
public:
					SetupListItem(BString item);
					~SetupListItem();

	virtual void	DrawItem(BView*, BRect, bool);
	
	BString			GetItem() { return fItemString; };
		
private:
	BPath*			fPath;
	BString			fItemString;
	bool			fIsDirectory;
};

#endif // SETUPLISTITEM_H
