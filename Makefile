NAME= QuickLaunch
TYPE= APP
APP_MIME_SIG= application/x-vnd.QuickLaunch
SRCS= QLFilter.cpp QuickLaunch.cpp MainWindow.cpp MainListView.cpp MainListItem.cpp SetupWindow.cpp SetupListView.cpp QLSettings.cpp
RDEFS= QuickLaunch.rdef
LIBS= be localestub tracker $(STDCPPLIBS)
LIBPATHS=
SYSTEM_INCLUDE_PATHS=
LOCAL_INCLUDE_PATHS=
OPTIMIZE= FULL
#	specify any preprocessor symbols to be defined.  The symbols will not
#	have their values set automatically; you must supply the value (if any)
#	to use.  For example, setting DEFINES to "DEBUG=1" will cause the
#	compiler option "-DDEBUG=1" to be used.  Setting DEFINES to "DEBUG"
#	would pass "-DDEBUG" on the compiler's command line.
LOCALES= en de it ja nl pl
DEFINES=
#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS=
# Build with debugging symbols if set to TRUE
DEBUGGER=
SYMBOLS=
COMPILER_FLAGS=
LINKER_FLAGS=

## include the makefile-engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
