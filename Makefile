gccver= $(shell setgcc | tail -c -2)
NAME= QuickLaunch
TYPE= APP
SRCS= QuickLaunch.cpp QLFilter.cpp MainWindow.cpp MainListView.cpp MainListItem.cpp SetupWindow.cpp SetupListView.cpp QLSettings.cpp
RSRCS= QuickLaunch.rsrc
ifeq ($(gccver),2)
	LIBS= /boot/develop/lib/x86/libbe.so /boot/system/lib/libstdc++.r4.so /boot/develop/lib/x86/libroot_debug.so /boot/develop/lib/x86/libtracker.so
else
	LIBS= /boot/system/lib/gcc4/libbe.so /boot/system/lib/gcc4/libstdc++.so /boot/system/lib/gcc4/libroot_debug.so /boot/system/lib/gcc4/libtracker.so
endif
LIBPATHS=
SYSTEM_INCLUDE_PATHS= /boot/develop/headers/be /boot/develop/headers/cpp /boot/develop/headers/posix /boot/home/config/include
LOCAL_INCLUDE_PATHS=
OPTIMIZE=FULL
#	specify any preprocessor symbols to be defined.  The symbols will not
#	have their values set automatically; you must supply the value (if any)
#	to use.  For example, setting DEFINES to "DEBUG=1" will cause the
#	compiler option "-DDEBUG=1" to be used.  Setting DEFINES to "DEBUG"
#	would pass "-DDEBUG" on the compiler's command line.
DEFINES=
#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS =
# Build with debugging symbols if set to TRUE
SYMBOLS=
COMPILER_FLAGS=
LINKER_FLAGS=

## include the makefile-engine
include $(BUILDHOME)/etc/makefile-engine
