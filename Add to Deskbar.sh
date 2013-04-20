#!/bin/sh

# Add QuickLaunch to the Deskbar tray

cd $(dirname "$0")
desklink "cmd=Remove replicant:desklink --remove=QuickLaunch" `pwd`/QuickLaunch
alert --info "QuickLaunch was added to the Deskbar.

You can remove it again with its context menu (a right-click on its icon)." OK
