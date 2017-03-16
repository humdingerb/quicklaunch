/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef SETUP_LISTVIEW_H
#define SETUP_LISTVIEW_H

#include <ListView.h>
#include <String.h>

#include <stdio.h>

class SetupListView : public BListView {
public:
					SetupListView();
					~SetupListView();

	virtual	void	KeyDown(const char* bytes, int32 numBytes);
	virtual void	SelectionChanged();
};

#endif // SETUP_LISTVIEW_H
