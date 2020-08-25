#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([xl4unipac], [1.0], [shiro@excelfore.com])
AM_INIT_AUTOMAKE()
LT_PREREQ([2.2])
LT_INIT([static])

AM_PATH_PYTHON([3])

AC_ARG_ENABLE([understpl],
	AS_HELP_STRING([--enable-understpl],[enable understpl mode build]))
AC_ARG_ENABLE([thread],
	AS_HELP_STRING([--disable-thread],[disable building threadding programs]))

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])

AM_CONDITIONAL(BUILD_IN_STPL, [test x"$enable_understpl" = "xyes"])
AM_CONDITIONAL(NO_THREAD, [test x"$enable_thread" = "xno"])

AC_OUTPUT
