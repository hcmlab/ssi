##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = util_test

util_test_SOURCES = util_test.cpp
util_test_LDADD = ../../util.o ../../gloox.o
util_test_CFLAGS = $(CPPFLAGS)
