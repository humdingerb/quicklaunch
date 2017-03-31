/*
 * Copyright 2010-2017. All rights reserved.
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

static const char kVersion[] = "v1.1";
static const char kCopyright[] = "2010-2017";


class QLApp : public BApplication {
public:
					QLApp();
	virtual			~QLApp();

	void			AboutRequested();
	void			MessageReceived(BMessage* message);
	virtual bool	QuitRequested();
	virtual void	ReadyToRun();

	void			SetWindowsFeel(int32 value);

	SetupWindow		*fSetupWindow;
	QLSettings		*fSettings;
	MainWindow		*fMainWindow;

private:
	void			_AddToDeskbar();
	void			_RemoveFromDeskbar();
};

#endif	// QUICKLAUNCH_H
