/*
 * Copyright 2010. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef QL_FILTER_H
#define QL_FILTER_H

#include <AppDefs.h>
#include <Handler.h>
#include <InterfaceDefs.h>
#include <Looper.h>
#include <MessageFilter.h>
#include <Messenger.h>

#include <stdio.h>

#define CURSOR_UP			'upar'
#define CURSOR_DOWN			'down'
#define PAGE_UP				'pgup'
#define PAGE_DOWN			'pgwn'
#define HOME				'home'
#define END					'ende'
#define RETURN_KEY			'rtrn'
#define RETURN_SHIFT_KEY	'rtsh'
#define RETURN_CTRL_KEY		'rtct'
#define NEW_FILTER			'fltr'


class QLFilter : public BMessageFilter {
public:
							QLFilter();
	virtual					~QLFilter();
	virtual filter_result 	Filter(BMessage* message, BHandler** target);
};

#endif // QL_FILTER_H
