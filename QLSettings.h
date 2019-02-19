/*
 * Copyright 2010-2018. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	Humdinger, humdingerb@gmail.com
 *  Kevin Adams
 */

#ifndef QLSETTINGS_H
#define QLSETTINGS_H

#include "IgnoreListView.h"

#include <List.h>
#include <Locker.h>
#include <Rect.h>
#include <String.h>


class QLSettings { 
public:
			QLSettings();
			~QLSettings();

	bool	Lock();
	void	Unlock();
	void	SaveSettings();

	void 	SetMainWindowFrame(BRect frame) { fMainWindowFrame = frame; };
	void 	SetSetupWindowFrame(BRect setupframe)
				{ fSetupWindowFrame = setupframe; };
	void	SetDeskbar(int32 deskbar) { fDeskbar = deskbar; };
	void	SetShowVersion(int32 version) { fShowVersion = version; };
	void	SetShowPath(int32 path) { fShowPath = path; };
	void	SetSearchStart(int32 searchstart) { fSearchStart = searchstart; };
	void	SetSaveSearch(int32 savesearch) { fSaveSearch = savesearch; };
	void	SetSearchTerm(BString searchterm) { fSearchTerm = searchterm; };
	void	SetOnTop(int32 ontop) { fOnTop = ontop; };
	void    SetShowTitlebar(int32 titlebar) { fShowTitlebar = titlebar; };
	void	SetSingleClick(bool singleclick) { fSingleClick = singleclick; };
	void	SetShowIgnore(int32 ignore) { fShowIgnore = ignore; };

	BRect	GetMainWindowFrame() { return fMainWindowFrame; };
	BRect	GetSetupWindowFrame() { return fSetupWindowFrame; };
	int32	GetDeskbar() { return fDeskbar; };
	int32	GetShowVersion() { return fShowVersion; };
	int32	GetShowPath() { return fShowPath; };
	int32	GetSearchStart() { return fSearchStart; };
	int32	GetSaveSearch() { return fSaveSearch; };
	BString	GetSearchTerm() { return fSearchTerm; };
	int32	GetSingleClick() { return fSingleClick; };
	int32	GetOnTop() { return fOnTop; };
	int32   GetShowTitlebar() { return fShowTitlebar; };
	int32	GetShowIgnore() { return fShowIgnore; };

	void			InitLists();
	IgnoreListView* IgnoreList() { return fIgnoreList; };

	BList*			fFavoriteList;
	IgnoreListView*	fIgnoreList;

private:
	BRect	fMainWindowFrame;
	BRect	fSetupWindowFrame;
	int32	fDeskbar;
	int32	fShowVersion;
	int32	fShowPath;
	int32	fSearchStart;
	int32	fSaveSearch;
	BString	fSearchTerm;
	int32	fSingleClick;
	int32	fOnTop;
	int32   fShowTitlebar;
	int32	fShowIgnore;

	BLocker	fLock;
};

#endif	// QLSETTINGS_H
