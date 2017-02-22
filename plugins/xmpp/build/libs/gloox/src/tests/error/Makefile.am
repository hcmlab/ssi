##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = error_test

error_test_SOURCES = error_test.cpp
error_test_LDADD = ../../error.o ../../tag.o ../../gloox.o ../../util.o
error_test_CFLAGS = $(CPPFLAGS)

