/*
 * Copyright 2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdinger@mailbox.org
 */
#ifndef APPLISTITEM_H
#define APPLISTITEM_H


#include <Entry.h>
#include <String.h>


class AppListItem {
public:
	AppListItem(BEntry entry, bool localized);

	BString GetName() { return fName; };
	entry_ref* GetRef() { return &fRef; };
	
private:
	entry_ref fRef;
	BString fName;
};

#endif // APPLISTITEM_H
