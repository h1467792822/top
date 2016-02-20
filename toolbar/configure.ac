AC_INIT([toolbar],[0.9],[hantingjun@huawei.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_RANLIB
AC_PROG_CXX
AC_PROG_CC
AC_ARG_WITH(cppunit_cflags,
			AS_HELP_STRING([--width-cppunit_cflags=CFLAGS]),
			[CPPUNIT_CFLAGS=$withval],
			[CPPUNIT_CFLAGS=`pkg-config --cflags cppunit`])
AC_SUBST([CPPUNIT_CFLAGS])
AC_ARG_WITH(cppunit_ldflags,
			AS_HELP_STRING([--width-cppunit_ldflags=LDFLAGS]),
			[CPPUNIT_LDFLAGS=$withval],
			[CPPUNIT_LDFLAGS=`pkg-config --libs cppunit`])
AC_SUBST([CPPUNIT_LDFLAGS])
AC_ARG_WITH(notest,
			AS_HELP_STRING([--with-notest]),
			[notest=yes],
			[notest=no])
AM_CONDITIONAL([CHECK_TEST],[test "$notest" = no])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
