
include $(top_srcdir)/build/Makefile.common

lib_LTLIBRARIES = libtop_core.la
libtop_core_la_SOURCES = \
	heap.c \
	avltree.c 

if CHECK_TEST

TESTS = coretestmain
check_PROGRAMS = $(TESTS)

coretestmain_SOURCES = test_main.cpp \
		       test_heap.cpp \
			test_avltree.cpp 

coretestmain_LDFLAGS = @CPPUNIT_LDFLAGS@ $(top_builddir)/src/top/core/libtop_core.la

coretestmain_CPPFLAGS = @CPPUNIT_CFLAGS@ $(AM_CPPFLAGS)

endif

