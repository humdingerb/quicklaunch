/*
 * Copyright 2010-2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdinger@mailbox.org
 */

#ifndef QL_WINDOW_H
#define QL_WINDOW_H

#include "AppListItem.h"
#include "MainListItem.h"
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
#include <ObjectList.h>
#include <Path.h>
#include <Query.h>
#include <Roster.h>
#include <Screen.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <Window.h>

#include <stdlib.h>
#include <strings.h>


#define SINGLE_CLICK		'1clk'
#define SETUP_MENU			'setb'
#define HELP_MENU			'help'
#define ADDIGNORE			'addi'


#define kMAX_DISPLAYED_ITEMS	10


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	void			MenusBeginning();
	void			MessageReceived(BMessage* message);
	bool			QuitRequested();

	void			BuildAppList();

	bool			IsFavoritesOnly() { return fSearchBox->TextView()->TextLength() == 0; };
	const char*		GetSearchString() { return fSearchBox->TextView()->Text(); };
	void			ResultsCountChanged();

private:
	static status_t _AppListThread(void* self);
	void			_BuildAppList();

	void			_RebuildResults();
	void			_FilterAppList();
	void			_ShowFavorites();

	float			_GetScrollPosition();
	void			_SetScrollPosition(float position);

	void			_LaunchApp(MainListItem* item);
	void			_AddDroppedAsFav(BMessage* message);

	thread_id		fThreadId;
	bool			fBusy;

	BObjectList<AppListItem> fAppList;
	int32			fIconHeight;

	BMenu*			fSelectionMenu;

	BMenuItem*		fAddRemoveFav;
	BMenuItem*		fAddToIgnore;
	BMenuItem*		fTempShowPath;
	BMenuItem*		fTempShowVersion;
	BMenuItem*		fTempSearchStart;
	BMenuItem*		fTempApplyIgnore;

	BTextControl*	fSearchBox;
	BScrollView*	fScrollView;
	MainListView*	fListView;

	BWindow*		fSetupWindow;
};

#endif // QL_WINDOW_H
