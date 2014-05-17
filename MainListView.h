/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QLLISTVIEW_H
#define QLLISTVIEW_H

#include <ListView.h>
#include <String.h>

#include <stdio.h>

class MainListView : public BListView {
public:
					MainListView();
					~MainListView();
	virtual void	Draw(BRect rect);
	virtual	void	FrameResized(float w, float h);
};

#endif // QLLISTVIEW_H
