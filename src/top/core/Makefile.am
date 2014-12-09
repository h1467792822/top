
include $(top_srcdir)/build/Makefile.common

lib_LTLIBRARIES = libtop_core.la
libtop_core_la_SOURCES = \
	heap.c \
	avltree.c \
	rbtree.c \
	linux_rbtree.c \
	radix_tree.c

if CHECK_TEST

TESTS = coretestmain
check_PROGRAMS = $(TESTS)

coretestmain_SOURCES = test_main.cpp \
			test_timer.hpp \
		       test_heap.cpp \
			test_rbtree.cpp \
			test_linux_rbtree.cpp \
			test_avltree.cpp \
			test_list.cpp \
			test_hlist.cpp \
			test_radix_tree.cpp 

coretestmain_SOURCES1 = test_main.cpp \
			test_radix_tree.cpp 

coretestmain_LDFLAGS = @CPPUNIT_LDFLAGS@ $(top_builddir)/src/top/core/libtop_core.la

coretestmain_CPPFLAGS = $(AM_CPPFLAGS) @CPPUNIT_CFLAGS@

endif

