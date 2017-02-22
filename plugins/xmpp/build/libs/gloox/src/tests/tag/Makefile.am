##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = tag_test tag_perf

tag_test_SOURCES = tag_test.cpp
tag_test_LDADD = ../../tag.o ../../gloox.o ../../util.o
tag_test_CFLAGS = $(CPPFLAGS)

tag_perf_SOURCES = tag_perf.cpp
tag_perf_LDADD = ../../tag.o ../../gloox.o ../../util.o
tag_perf_CFLAGS = $(CPPFLAGS)
