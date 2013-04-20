/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */
 
#ifndef QLSETTINGS_H
#define QLSETTINGS_H

#include <Rect.h>
#include <String.h>


class QLSettings {
public:
				QLSettings();
				~QLSettings();

	void 		SetMainWindowFrame(BRect frame) {fMainWindowFrame = frame;};
	void 		SetSetupWindowBounds(BRect bounds) {fSetupWindowBounds = bounds;};
	void		SetShowVersion(int32 version) {fShowVersion = version;};
	void		SetShowPath(int32 path) {fShowPath = path;};
	void		SetShowIgnore(int32 ignore) {fShowIgnore = ignore;};
	void		SetItemCount(int32 itemcount) {fItemCount = itemcount;};
	
	BRect 		GetMainWindowFrame() {return fMainWindowFrame;};
	BRect 		GetSetupWindowBounds() {return fSetupWindowBounds;};
	int32		GetShowVersion() {return fShowVersion;};
	int32		GetShowPath() {return fShowPath;};
	int32		GetShowIgnore() {return fShowIgnore;};
	int32		GetItemCount() {return fItemCount;};
	void		InitIgnoreList();

private:
	BRect		fMainWindowFrame;
	BRect		fSetupWindowBounds;
	int32		fShowVersion;
	int32		fShowPath;
	int32		fShowIgnore;
	int32		fItemCount;
};

#endif	// QLSETTINGS_H
