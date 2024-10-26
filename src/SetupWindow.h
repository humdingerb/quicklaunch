/*
 * Copyright 2010-2022. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 *  Chris Roberts
 */

#ifndef SETUP_WINDOW_H
#define SETUP_WINDOW_H

#include "IgnoreListView.h"

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

#define DESKBAR_CHK		'chdb'
#define VERSION_CHK		'chve'
#define PATH_CHK		'chpa'
#define SEARCHSTART_CHK	'chst'
#define SAVESEARCH_CHK	'chss'
#define SORTFAVS_CHK	'chsf'
#define IGNORE_CHK		'chig'
#define OPEN_SHORTCUTS	'opsc'
#define ADD_BUT			'addb'
#define REM_BUT			'remb'
#define FILEPANEL		'file'
#define POPCLOSE		'clpo'

class SetupWindow : public BWindow {
public:
					SetupWindow(BRect frame);
	virtual			~SetupWindow();

	bool			QuitRequested();
	void			MessageReceived(BMessage* message);

	BCheckBox*		fChkDeskbar;
	BCheckBox*		fChkVersion;
	BCheckBox*		fChkPath;
	BCheckBox*		fChkSearchStart;
	BCheckBox*		fChkSaveSearch;
	BCheckBox*		fChkSortFavorites;
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
