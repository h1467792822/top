
include $(top_srcdir)/build/Makefile.common

lib_LTLIBRARIES = libtop_mem.la

libtop_mem_la_SOURCES = \
	memory.c

if CHECK_TEST

TESTS = memtestmain
check_PROGRAMS = $(TESTS)

memtestmain_SOURCES = test_main.cpp \
			test_memory.cpp

memtestmain_LDFLAGS = @CPPUNIT_LDFLAGS@ $(top_builddir)/src/top/mem/libtop_mem.la

memtestmain_CPPFLAGS = $(AM_CPPFLAGS) @CPPUNIT_CFLAGS@

endif

