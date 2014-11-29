
include $(top_srcdir)/build/Makefile.common

lib_LTLIBRARIES = libtop_core.la
libtop_core_la_SOURCES = heap.c

if CHECK_TEST

TEST = coretestmain
check_PROGRAMS = $(TEST)

coretestmain_SOURCES = test_heap.cpp

coretestmain_LDFLAGS = @CPPUNIT_LDFLAGS@ $(top_builddir)/src/top/core/libtop_core.la

coretestmain_CFLAGS = @CPPUNIT_CFLAGS@

endif

