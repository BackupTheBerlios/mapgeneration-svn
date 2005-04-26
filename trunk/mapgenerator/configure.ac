#*******************************************************************************
# MapGeneration Project - Creating a road map for the world.                   *
#                                                                              *
# Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
# Licensed under the Academic Free License version 2.1                         *
#******************************************************************************/


# Process this file with autoconf to produce a configure script.
#AC_PREREQ(2.59)
AC_INIT(mapgenerator, 0.1)
AC_CONFIG_SRCDIR([README])
AC_CONFIG_AUX_DIR([./auxs])
AC_CONFIG_HEADER([config.h])


# Checking host os
AC_CANONICAL_HOST

if test "$host_os" = "cygwin"; then
	CXXFLAGS="$CXXFLAGS -DCYGWIN"
fi


#### Features ####

# Debug
AC_MSG_CHECKING(whether to enable debugging)
debug_default="yes"
AC_ARG_ENABLE(debug,
		AC_HELP_STRING([--enable-debug=[yes/no]],
                       [Turn debugging on/off [default=yes]]),
		, enable_debug=$debug_default)

if test "x$enable_debug" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -g -DDEBUG"
		AC_MSG_RESULT(yes)
else
		CXXFLAGS="$CXXFLAGS -DNDEBUG"
		AC_MSG_RESULT(no)
fi


# Optimization
AC_MSG_CHECKING(whether to enable compiler optimization)
optimization_default="no"
AC_ARG_ENABLE(optimization,
		AC_HELP_STRING([--enable-optimization=[yes/no]],
                       [Turn compiler optimization on/off [default=no]]),
		, enable_optimization=$optimization_default)

if test "x$enable_optimization" = "xyes"; then
		CXXFLAGS="$CXXFLAGS -O3"
		AC_MSG_RESULT(yes)
else
		CXXFLAGS="$CXXFLAGS -O0"
		AC_MSG_RESULT(no)
fi


# Tracelogs
AC_MSG_CHECKING(whether to enable tracelogs)
debug_tracelogs="yes"
AC_ARG_ENABLE(tracelogs,
		AC_HELP_STRING([--enable-tracelogs=[yes/no]],
                       [Turn tracelogs on/off [default=yes]]),
		, enable_tracelogs=$tracelogs_default)

if test "x$enable_tracelogs" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -DTRACELOGS"
		AC_MSG_RESULT(yes)
else
		AC_MSG_RESULT(no)
fi


# GUI
AC_MSG_CHECKING(whether to enable build the gui)
gui_default="yes"
AC_ARG_ENABLE(gui,
		AC_HELP_STRING([--enable-gui=[yes/no]],
                       [Turn compiling the gui on/off [default=no]]),
		, enable_gui=$gui_default)
if test "x$enable_gui" = "xyes"; then
		AC_MSG_RESULT(yes)
else
		AC_MSG_RESULT(no)
fi
AC_SUBST(enable_gui)


#### Extras ####
AC_ARG_WITH(extra-includes,
            AC_HELP_STRING([--with-extra-includes=DIR],
                       [Define an additional directory for include files]),
        [ if test -d "$withval" ; then
            CPPFLAGS="$CPPFLAGS -I$withval" 
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_ARG_WITH(extra-libraries,
            AC_HELP_STRING([--with-extra-libraries=DIR],
                       [Define an additional directory for library files]),
        [ if test -d "$withval" ; then
           LDFLAGS="$LDFLAGS -L$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )



AC_LANG([C++])

#### Checks for programs. ####
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

AC_DISABLE_SHARED
AC_PROG_LIBTOOL


#### Checks for libraries. ####
# Common C++
CCGNU2_CONFIG_PATH="$PATH"
AC_ARG_WITH(ccgnu2-config-path,
            AC_HELP_STRING([--with-ccgnu2-config-path=DIR],
                       [Define the path to the commoncpp2 program ccgnu2-config]),
        [ if test -d "$withval" ; then
			CCGNU2_CONFIG_PATH="$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_PATH_PROG(CCGNU2_CONFIG, ccgnu2-config,
		,
		$CCGNU2_CONFIG_PATH)

if test $CCGNU2_CONFIG; then
	LIBS="$LIBS `$CCGNU2_CONFIG --stdlibs`"
	CPPFLAGS="$CPPFLAGS `$CCGNU2_CONFIG --flags`"
else
	AC_MSG_ERROR([ccgnu2-config not found!])
fi


# ODBC
# ODBC: Explicit DLL filename for cygwin
AC_SUBST(ODBC32_DLL)
AC_ARG_WITH(odbc32-dll,
		AC_HELP_STRING([--with-odbc32-dll=FILE],
						[Define odbc32.dll file (cygwin)]),
		ODBC32_DLL="$withval")
if test "$host_os" = "cygwin" ; then
	if test -z $ODBC32_DLL; then
		AC_MSG_NOTICE([To use ODBC on cygwin you have to define --with-odbc32-dll=FILE!])
	fi
	AC_MSG_CHECKING([for odbc32.dll])
	AC_CHECK_FILE([$ODBC32_DLL],
		AC_MSG_RESULT([$ODBC32_DLL]),
		AC_MSG_RESULT([not found]))
fi

# ODBC: ODBC directory
AC_ARG_WITH(odbc-dir,
		AC_HELP_STRING([--with-odbc-dir=DIR],
                       [Define the path to odbc]),
        [ if test -d "$withval" ; then
			ODBC_DIR="$withval"
			ODBC_INCLUDE_DIR=$ODBC_DIR/include
			ODBC_LIB_DIR=$ODBC_DIR/lib		
			CPPFLAGS="$CPPFLAGS -I$ODBC_INCLUDE_DIR"
			LIBS="-L$ODBC_LIB_DIR $LIBS"
          else
			AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )
          
#ODBC: System parameters
AC_MSG_CHECKING([for odbc-system])
if test "$host_os" = "cygwin"; then
	ODBC_NAME="odbc32"
	SQL_INCLUDES="#include <windows.h>"
	AC_MSG_RESULT([Cygwin])
else
	ODBC_NAME="odbc"
	AC_MSG_RESULT([unixODBC])
fi

# ODBC: sql.h Header
AC_CHECK_HEADER(sql.h,
	[sql_h=yes],
	[sql_h=no],
	[$SQL_INCLUDES])

# ODBC: Library	
AC_CHECK_LIB($ODBC_NAME, main,
	[sql_header=yes],
	[sql_header=no]
	)

if test "$sql_h" = "yes" -a "$sql_header" = "yes"
then
	LIBS="-l$ODBC_NAME $LIBS"
	have_odbc=yes
	AC_DEFINE(HAVE_ODBC, [1], [Define to 1 if odbc support was found.])
	AC_MSG_NOTICE([ODBC support enabled])
else
	have_odbc=no
	AC_MSG_NOTICE([ODBC support disabled])
fi
AC_SUBST(have_odbc)


# WxWidgets	
WX_CONFIG_PATH="$PATH"
AC_ARG_WITH(wx-config-path,
            AC_HELP_STRING([--with-wx-config-path=DIR],
                       [Define the path to the WxWidgets program wx-config]),
        [ if test -d "$withval" ; then
			WX_CONFIG_PATH="$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )
          
if test "x$enable_gui" = "xyes"; then
	AC_SUBST(WX_LIBS)
	AC_SUBST(WX_CPPFLAGS)
	AC_PATH_PROG(WX_CONFIG, wx-config,
			,
			$WX_CONFIG_PATH)

	if test $WX_CONFIG; then
		LIBS="$LIBS `$WX_CONFIG --libs`"
		CPPFLAGS="$CPPFLAGS `$WX_CONFIG --cxxflags`"
	else
		AC_MSG_ERROR([wx-config not found!])
	fi

	#WxWidgets XRC support: Header
	AC_CHECK_HEADER(wx/xrc/xmlres.h,
		,
		[ AC_MSG_ERROR([XRC-Header (wx/xrc/xmlres.h) not found!]) ],
		)

	#WxWidgets XRC support: Library
	WX_XRC_LIBRARY_NAME="`$WX_CONFIG --basename`_xrc-`$WX_CONFIG --release`"
	AC_CHECK_LIB($WX_XRC_LIBRARY_NAME, main,
		WX_LIBS="$WX_LIBS -l`$WX_CONFIG --basename`_xrc-`$WX_CONFIG --release`",
		[ AC_MSG_ERROR([XRC-Library ($WX_XRC_LIBRARY_NAME) not found!])]
		)
fi


#### Checks for header files. ####

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_STRTOD


AC_CONFIG_FILES([Makefile bin/Makefile bin/tests/Makefile bin/export/Makefile bin/gui/Makefile bin/gpstrans/Makefile])
AC_OUTPUT

