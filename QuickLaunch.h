/*
 * Copyright 2010. All rights reserved.
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
};

#endif	// QUICKLAUNCH_H
