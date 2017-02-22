##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = connectionbosh_test

connectionbosh_test_SOURCES = connectionbosh_test.cpp
connectionbosh_test_LDADD = ../../connectionbosh.o ../../parser.o ../../tag.o ../../logsink.o \
                            ../../gloox.o ../../prep.o ../../util.o
connectionbosh_test_CFLAGS = $(CPPFLAGS)
