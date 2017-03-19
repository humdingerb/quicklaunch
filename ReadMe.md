![QuickLaunch icon](./images/quicklaunch_icon_64.png)
**QuickLaunch**
for [Haiku](http://www.haiku-os.org)

* * *

QuickLaunch is a small launcher tool that helps you to quickly start any installed application.

Simply start to enter the name of an application and QuickLaunch will find all programs matching these initial letters and show them in a list. You choose an app from that list with the _CursorUp/Down_ keys and launch it by hitting _RETURN_. _ESC_ quits QuickLaunch.

Here's the main window after searching for all applications starting with "me" and behind it the settings window when clicking the _Setup_ button (or pressing _ALT_+_S_):

![QuickLaunch windows](./images/quicklaunch.png)

In the main window the context menu was evoked via a right-click. The first item adds the selected app to the ignore list (works also with a drag & drop). More on that in the setup options discussed below. The second item opens the app's location in a Tracker window. The same can be achieved by pressing _CTRL_ + _RETURN_.

The setup window shows these options:

*	_Show Deskbar replicant_ — puts the QuickLaunch icon into the Deskbar tray. Click on it to start QuickLaunch.
*   _Show the version of an application_ — only really useful when having older and newer versions of an app installed.
*   _Show the path to an application_ — interesting when you have several copies of an application.
*   _Wait for a second letter before starting the search_ — starts QuickLaunch with the previous search.
*   _Remeber last search term_ — probably only useful for slow systems.
*   _Launch applications with a single click_ — instead of requiring the usual double-click.
*   _Window always on top_ — the window floats modally in front of all others. 
Useful if you start a few apps after another and don't want to lose QuickLauch under the newly opening windows.
*   _Ignore these files & folders (and their subfolders)_ that you can add and remove from the list with the buttons at the bottom.
You can also right-click an app in the main window's result list to add that specific app to the ignore list or drag & drop any application or folder from Tracker or an 	app from QuickLaunch's main window into it.
_DEL_ removes the selected items from the ignore list.

### Tips & Tricks

*   Assign a keycombo to QuickLaunch in Haiku's Shortcuts preferences, like _OPT_ + _SPACEBAR_, for even quicker QuickLaunching apps.
*   Use '\*' as wildcards, e.g. "\*play" for all apps with "play" in their name.
*   _SHIFT_ + _RETURN_ will keep the QuickLaunch window open after launching the app.
*   _CTRL_ + _RETURN_ will open an app's location in Tracker instead of launching it.
*   These folders are ignored by default:
`B_SYSTEM_ADDONS_DIRECTORY` and `B_TRASH_DIRECTORY`
In other words, applications in Trash are ignored, as well as add-ons.
`B_SYSTEM_BIN_DIRECTORY` used to be ignored, but since especially ported software often ends up in the /bin folder, that is a bad idea. You'll have to remove unwanted commandline apps manually now, e.g. via _Add to ignore list_ of the context menu.
*	If you happen to use an app or a script that doesn't show up in QuickLaunch (and is in a writable location), you can add these atrributes in Terminal to the app or script:
`addattr BEOS:TYPE application/x-vnd.Be-elfexecutable /path/to/your/app-or-script`
`addattr BEOS:APP_SIG  application/x-vnd.anything-unique /path/to/your/app-or-script`

### Download

QuickLaunch is directly available through HaikuDepot from the HaikuPorts repository. You can also build it yourself using [Haikuporter](https://github.com/haikuports). The source is hosted at [GitHub](https://github.com/humdingerb/quicklaunch).

### Bugreports & Feedback

Please use GitHubs's [issue tracker](https://github.com/humdingerb/quicklaunch/issues) if you experience unusual difficulties or email your general feedback to [me](mailto:humdingerb@gmail.com). Also, email me if you'd like to provide more localizations.

### Thanks

I have to thank DarkWyrm for some helpful hints and for some of the code I re-used from his RunProgram app and his EscapeCancelFilter. Equally useful was Robert Polic's EZLauncher of BeNewsletter 3-46 and the help I got on the #haiku IRC channel.
AnEvilYak provided valuable hints and code when I got stuck.
Finally, thanks to everyone that contributed translations for QuickLaunch.

### History

**1.0** - _20-03-2017:_

*	Add a setting to launch applications with a single click.
*	Add option to put a replicant in the Deskbar tray for a quick launch of QuickLaunch.
*	Have the icon size depend on the font size.
*	Fixed layout issues in the setup window and made default main and setup windows a bit bigger. Save the absolute position of the setup window.
*	Change resizing of the main window to always fit the number of list items; font sensitive.
*	Allow drag & drop of files from Tracker or items from the main window to the "ignore" list.
	Allow removing multiple items from the "ignore" list. Also via _DEL_ key or a context menu.
*	Improved page up/down scrolling.
*	Apply "Always on top" setting only to the main window when there's no setup window open. The "ignore" file dialog has issues when it's modal too: you couldn't drag & drop files/folders into it...
*	Open and close the Setup window with _ALT_+_S_.
*	Show an error dialog if the launch of an application has failed for some reason.
*	Added a tip to the ReadMe, showing how to add type and signature to apps and scripts that are missing them.
*	Added French localization, thanks Anarchos!
*	Added Russian localization, thanks Akujiism!
*	Added Spanish localization, thanks un_spacyar!
*	Added Ukrainian localization, thanks Lan72!

**0.9.12** - _03-09-2015:_

*	Make the "Window always on top" optional. Not everyone was pleased with	that change of v0.9.11...
*	Also fix the formerly not usable file dialog when adding files/folders to ignore. It was hidden behind the always-on-top window.
*	Added Italian localization, thanks Barrett!

**0.9.11** - _16-08-2015:_

*   Add "Open containing folder" to the context menu.
*   Add an option to always start with the previous search term.
*   Have the windows always on top of every other app.
*   Add horizontal stroke below every list item.
*   Cosmetics on resizing window so the height won't jump and flicker.

**0.9.10** - _15-06-2015:_

*   Allow specific files in the ignore list, not only complete folders (and their subfolders).
*   Show a context menu when right-clicking an app to quickly add it to the ignore list.
*   Scroll to original location in the app list instead of jumping to the top after adding to the ignore list.
*   Updated icon to show three app cubes as Q stroke.
*   Localization for German, Dutch, Japanese, Polish.

**0.9.9** - _15-01-2015:_

*   Don't ignore /system/bin/ by default. Now that more and more ports are available for Haiku, some GUI apps do appear in that folder...

**0.9.8** - _02-01-2015:_

*   Fixed a crash on quit. No idea how it has worked all these years, frankly.
*   Removed a never used variable and its get/set functions.

**0.9.7** - _17-05-2014:_

*   Added a setting to only start searching after entering a second letter of the application name.
*   Respecting custom colours set in Appearance preferences.
*   Better handling of bigger/smaller font sizes.
*   Renamed "jokers" to "wildcards".

**0.9.6** - _14-10-2013:_

*   Small changes to work under Haiku with package management.

**0.9.5** - _17-05-2010:_

*   Ignore Trash on all volumes, not just on /boot.
*   Be stricter on the apps' signature. They have to conform to the standard and 	start with "application/x-vnd". This avoids listing e.g, libraries.
*   PageUp and PageDown keys now work in the result list.
*   Case-insentive sorting.

**0.9.4** - _11-03-2010:_

*   Added Control+Return to open an app's location in Tracker.
*   Added live-truncating of strings on window resize.
*   The query now returns only app names beginning with the search string and now starts right after entering the first letter.
*   Better handling of different system font size settings.
*   Less restrictive window size limits.

**0.9.3** - _05-03-2010:_

*   Ignore path information wasn't loaded correctly.

**0.9.2** - _04-03-2010:_

*   Add setup window to toggle version and path information.
*   Ignore user definable folders.
*   Make main window horizontally resizable.
*   Remember window position and size.

**0.9.1** - _04-02-2010:_

*   Show version info to distinguish different versions of the same app.
*   Home/End key jump to first/last item.
*   Shift+Return will keep the QL window open.
*   Added script to add QL to the Deskbar tray. Remove via context menu (Removal will only work with Haiku > r35402, for lower revisions, open Terminal and enter "desklink --remove=DeskButton".)
	
**0.9.0** - _01-02-2010:_

*   Initial release.
