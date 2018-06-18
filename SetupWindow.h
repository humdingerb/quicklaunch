/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef SETUP_WINDOW_H
#define SETUP_WINDOW_H

#include "IgnoreListView.h"
#include "QLSettings.h"

#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <Entry.h>
#include <FilePanel.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ListView.h>
#include <ListItem.h>
#include <Message.h>
#include <Path.h>
#include <Screen.h>
#include <ScrollView.h>
#include <Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DESKBAR_CHK		'chdb'
#define VERSION_CHK		'chve'
#define PATH_CHK		'chpa'
#define DELAY_CHK		'chde'
#define SAVESEARCH_CHK	'chss'
#define SINGLECLICK_CHK	'ch1c'
#define ONTOP_CHK		'chot'
#define IGNORE_CHK		'chig'
#define ADD_BUT			'addb'
#define REM_BUT			'remb'
#define FILEPANEL		'file'
#define POPCLOSE		'clpo'
#define SEARCHSTART_CHK	'chst' 

class SetupWindow : public BWindow {
public:
					SetupWindow(BRect frame);
	virtual			~SetupWindow();

	bool			QuitRequested();
	void			MessageReceived(BMessage* message);

	BCheckBox*		fChkDeskbar;
	BCheckBox*		fChkVersion;
	BCheckBox*		fChkPath;
	BCheckBox*		fChkDelay;
	BCheckBox*		fChkSaveSearch;
	BCheckBox*		fChkSearchStart;
	BCheckBox*		fChkSingleClick;
	BCheckBox*		fChkOnTop;
	BCheckBox*		fChkIgnore;
	BButton*		fButRem;

private:

			void	_GetSelectedItems(BList& indices);
			void	_RemoveSelected(); // uses RemoveItemList()
	virtual	void	_RemoveItemList(const BList& indices);

	BScrollView*	fIgnoreScroll;
	BButton*		fButAdd;
	BFilePanel*		fOpenPanel;

	IgnoreListView*	fIgnoreList;
};

#endif // SETUP_WINDOW_H
