
include $(top_srcdir)/build/Makefile.common

lib_LTLIBRARIES = libtop_core.la
libtop_core_la_SOURCES = heap.c

if CHECK_TEST

TESTS = coretestmain
check_PROGRAMS = $(TESTS)

coretestmain_SOURCES = test_main.cpp \
		       test_heap.cpp

coretestmain_LDFLAGS = @CPPUNIT_LDFLAGS@ $(top_builddir)/src/top/core/libtop_core.la

coretestmain_CFLAGS = @CPPUNIT_CFLAGS@

endif

