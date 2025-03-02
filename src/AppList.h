/*
 * Copyright 2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 */
#ifndef APPLIST_H
#define APPLIST_H


#include "AppListItem.h"

#include <Looper.h>
#include <ObjectList.h>
#include <Volume.h>


typedef BObjectList<AppListItem, true> AppListItems;


class AppList : public BLooper {
public:
							AppList();
	virtual					~AppList();

	void					MessageReceived(BMessage* message);

	const AppListItems*		Items();

private:
	int						_AppendVolumeItems(const BVolume& volume, bool localized);
	void					_BuildAppList();

private:
	bool					fInit;
	AppListItems			fAppList;

};


#endif // APPLIST_H
