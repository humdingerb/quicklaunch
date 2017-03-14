/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "QuickLaunch.h"
#include "SetupListView.h"
#include "SetupWindow.h"

SetupListView::SetupListView()
		  : BListView("IgnoreList", B_MULTIPLE_SELECTION_LIST)
{
}


SetupListView::~SetupListView()
{
}


void
SetupListView::SelectionChanged()
{
	SetupWindow *window = dynamic_cast<SetupWindow *> (Window());
	if (CurrentSelection() < 0)
		window->fButRem->SetEnabled(false);
	else
		window->fButRem->SetEnabled(true);
}
