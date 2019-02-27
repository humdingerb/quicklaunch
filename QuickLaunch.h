/*
 * Copyright 2010-2019. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <Alert.h>
#include <Application.h>
#include <Messenger.h>

#include "MainWindow.h"
#include "QLSettings.h"
#include "SetupWindow.h"

static const char kVersion[] = "v1.3";
static const char kCopyright[] = "2010-2019";

#define my_app dynamic_cast<QLApp*>(be_app)


class QLApp : public BApplication {
public:
					QLApp();
	virtual			~QLApp();

	void			AboutRequested();
	void			MessageReceived(BMessage* message);
	virtual bool	QuitRequested();
	virtual void	ReadyToRun();

	QLSettings& 	Settings() { return fSettings; }

	SetupWindow*	fSetupWindow;
	MainWindow*		fMainWindow;

private:
	void			_AddToDeskbar();
	void			_RemoveFromDeskbar();
	void			_RestorePositionAndSelection();

	QLSettings		fSettings;
};

#endif	// QUICKLAUNCH_H
