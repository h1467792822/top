
AC_INIT([top],[0.1],[1467792822@qq.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR
AC_PROG_CXX
AC_PROG_CC
LT_INIT

AC_ARG_WITH(cppunit_cflags,
	AS_HELP_STRING([--with-cppunit_cflags=CFLAGS]),
	[CPPUNIT_CFLAGS=$withval],
	[CPPUNIT_CFLAGS=`pkg-config --cflags cppunit`])
AC_SUBST([CPPUNIT_CFLAGS])

AC_ARG_WITH(cppunit_ldflags,
	AS_HELP_STRING([--with-cppunit_ldlags=LDLAGS]),
	[CPPUNIT_LDFLAGS=$withval],
	[CPPUNIT_LDFLAGS=`pkg-config --libs cppunit`])
AC_SUBST([CPPUNIT_LDFLAGS])

AC_ARG_WITH(notest,
	AS_HELP_STRING([--with-notest]),
	[notest=yes],
	[notest=no])
AM_CONDITIONAL([CHECK_TEST],[test "$notest" = no])

AC_CONFIG_FILES([Makefile \
	src/Makefile \
	src/top/Makefile \
	src/top/core/Makefile])

AC_OUTPUT
