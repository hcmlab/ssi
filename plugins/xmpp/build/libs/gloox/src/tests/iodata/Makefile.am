##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = iodata_test

iodata_test_SOURCES = iodata_test.cpp
iodata_test_LDADD = ../../iodata.o ../../tag.o ../../gloox.o \
                    ../../util.o
iodata_test_CFLAGS = $(CPPFLAGS)
