/*
 * Copyright 2010-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "QuickLaunch.h"
#include "QLFilter.h"


QLFilter::QLFilter()
		: BMessageFilter(B_PROGRAMMED_DELIVERY,
			B_ANY_SOURCE, B_KEY_DOWN)
{
}


QLFilter::~QLFilter()
{
}


filter_result
QLFilter::Filter(BMessage* message, BHandler** target)
{
	int32 rawchar, mod;
	message->FindInt32("raw_char", &rawchar);
	message->FindInt32("modifiers", &mod);

	switch (rawchar) {
		case B_SPACE: case B_LEFT_ARROW: case B_RIGHT_ARROW: case B_INSERT:
		case B_DELETE: /*case B_PAGE_UP: case B_PAGE_DOWN:*/
		case B_FUNCTION_KEY:
		{
			return B_DISPATCH_MESSAGE;
		}
		case B_TAB:
		{
			return B_SKIP_MESSAGE;
		}
		case B_UP_ARROW:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(CURSOR_UP);
				return B_SKIP_MESSAGE;
			}
		}
		case B_DOWN_ARROW:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(CURSOR_DOWN);
				return B_SKIP_MESSAGE;
			}
		}
		case B_PAGE_UP:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(PAGE_UP);
				return B_SKIP_MESSAGE;
			}
		}
		case B_PAGE_DOWN:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(PAGE_DOWN);
				return B_SKIP_MESSAGE;
			}
		}
		case B_HOME:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(HOME);
				return B_SKIP_MESSAGE;
			}
		}
		case B_END:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(END);
				return B_SKIP_MESSAGE;
			}
		}
		case B_RETURN:
		{
			int message;
			if (mod & B_SHIFT_KEY) 
				message = RETURN_SHIFT_KEY;
			else if (mod & B_CONTROL_KEY) 
				message = RETURN_CTRL_KEY;
			else
				message = RETURN_KEY;
		
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(message);
				return B_SKIP_MESSAGE;
			}
		}
		case 'h':
		{
			if (mod & B_COMMAND_KEY) {
				be_app->PostMessage(HELP_BUTTON);
				return B_SKIP_MESSAGE;
			}
		}
		case 's':
		{
			if (mod & B_COMMAND_KEY) {
				be_app->PostMessage(SETUP_BUTTON);
				return B_SKIP_MESSAGE;
			}
		}
		case 'f':
		{
			if (mod & B_COMMAND_KEY) {
				BLooper* loop = (*target)->Looper();
				if (loop) {
					BMessenger msgr(loop);
					msgr.SendMessage(ADDFAVORITE);
					return B_SKIP_MESSAGE;
				}
			}
		}
		case 'r':
		{
			if (mod & B_COMMAND_KEY) {
				BLooper* loop = (*target)->Looper();
				if (loop) {
					BMessenger msgr(loop);
					msgr.SendMessage(REMOVEFAVORITE);
					return B_SKIP_MESSAGE;
				}
			}
		}
		case 'i':
		{
			if (mod & B_COMMAND_KEY) {
				BLooper* loop = (*target)->Looper();
				if (loop) {
					BMessenger msgr(loop);
					msgr.SendMessage(ADDIGNORE);
					return B_SKIP_MESSAGE;
				}
			}
		}
		default:
		{
			BLooper* loop = (*target)->Looper();
			if (loop) {
				BMessenger msgr(loop);
				msgr.SendMessage(NEW_FILTER);
			}
		}
	}
	return B_DISPATCH_MESSAGE;
}	
