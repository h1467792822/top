
bin_PROGRAMS = toolbar
lib_LIBRARIES = libtoolbar.a
SOURCES = mem.c ctx.c buffer.c state.c parse_req.c
libtoolbar_a_SOURCES = $(SOURCES)

toolbar_SOURCES= main.c 
toolbar_LDADD = libtoolbar.a

if CHECK_TEST

TESTS = test_toolbar
check_PROGRAMS = test_toolbar

test_toolbar_SOURCES = test_main.cpp \
					   test_buffer.cpp \
					   test_state.cpp \
					   test_parse_req.cpp \
					   test_ctx.cpp $(SOURCES)

test_toolbar_LDFLAGS = @CPPUNIT_LDFLAGS@ 

test_toolbar_CXXFLAGS = @CPPUNIT_CFLAGS@ 

endif
