##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = xpath_test

xpath_test_SOURCES = xpath_test.cpp
xpath_test_LDADD = ../../tag.o ../../gloox.o ../../util.o
xpath_test_CFLAGS = $(CPPFLAGS)
