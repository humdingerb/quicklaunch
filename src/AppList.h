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


typedef BObjectList<AppListItem> AppListItems;


class AppList : public BLooper {
public:
							AppList();

	void					MessageReceived(BMessage* message);

	const AppListItems*		Items();

private:
	void					_BuildAppList();

private:
	bool					fInit;
	AppListItems			fAppList;

};


#endif // APPLIST_H
