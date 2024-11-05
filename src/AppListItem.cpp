/*
 * Copyright 2024. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdinger@mailbox.org
 */

#include "AppListItem.h"

#include <LocaleRoster.h>


AppListItem::AppListItem(BEntry entry, bool localized)
{
	entry.GetRef(&fRef);
	fName = fRef.name;

	if (localized) {
		if (BLocaleRoster::Default()->GetLocalizedFileName(fName, fRef) != B_OK)
			fName = fRef.name;
	}
}
