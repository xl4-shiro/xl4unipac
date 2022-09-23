#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([xl4unipac], [1.0.1], [shiro@excelfore.com])
AC_SUBST(PACKAGE_DESC,"Excelfore Universal IPC and Configuration tools")

AM_INIT_AUTOMAKE()
LT_PREREQ([2.2])
LT_INIT([static])

# pkg-config dependency
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

AM_PATH_PYTHON([3])

AC_ARG_ENABLE([understpl],
	AS_HELP_STRING([--enable-understpl],[enable understpl mode build]))
AC_ARG_ENABLE([thread],
	AS_HELP_STRING([--disable-thread],[disable building threadding programs]))

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])

# Checks for libraries.
PKG_CHECK_MODULES([x4unibase],[xl4unibase],,
	[AC_CHECK_HEADERS([xl4unibase/unibase.h])]
)
PKG_CHECK_MODULES([x4combase],[xl4combase],,
	[AC_CHECK_HEADERS([xl4combase/combase.h])]
)

AM_CONDITIONAL(BUILD_IN_STPL, [test x"$enable_understpl" = "xyes"])
AM_CONDITIONAL(NO_THREAD, [test x"$enable_thread" = "xno"])

AC_OUTPUT([xl4unipac.pc])
