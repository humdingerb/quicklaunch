/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */
 
#ifndef QL_WINDOW_H
#define QL_WINDOW_H

#include "MainListView.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <Path.h>
#include <Query.h>
#include <Roster.h>
#include <Screen.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>


#define SEARCH_BOX		'sbox'
#define SETUP_BUTTON	'setb'

#define kMAX_DISPLAYED_ITEMS		9


class MainWindow : public BWindow {
public:
					MainWindow(BRect rect);
	virtual			~MainWindow();
	
	bool			QuitRequested();
	void			MessageReceived(BMessage* message);
	void			BuildList(const char *string);
	float			GetScrollPosition();
	void			SetScrollPosition(float position);
	int				GetStringLength() {return fSearchBox->TextView()->TextLength();};
	const char		*GetSearchString() {return fSearchBox->TextView()->Text();};
	MainListView	*fListView;
	
private:
	BTextControl	*fSearchBox;
	BButton			*fSetupButton;
	BScrollView		*fScrollView;
};

#endif // QL_WINDOW_H
