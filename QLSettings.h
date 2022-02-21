/*
 * Copyright 2010-2019. All rights reserved.
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
	void	SetShowIgnore(int32 ignore) { fShowIgnore = ignore; };
	void	SetSortFavorites(int32 sortfavs) { fSortFavorites = sortfavs; };

	BRect	GetMainWindowFrame() { return fMainWindowFrame; };
	BRect	GetSetupWindowFrame() { return fSetupWindowFrame; };
	int32	GetDeskbar() { return fDeskbar; };
	int32	GetShowVersion() { return fShowVersion; };
	int32	GetShowPath() { return fShowPath; };
	int32	GetSearchStart() { return fSearchStart; };
	int32	GetSaveSearch() { return fSaveSearch; };
	BString	GetSearchTerm() { return fSearchTerm; };
	int32	GetShowIgnore() { return fShowIgnore; };
	int32	GetSortFavorites() { return fSortFavorites; };

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
	int32	fShowIgnore;
	int32	fSortFavorites;

	BLocker	fLock;
};

#endif	// QLSETTINGS_H
