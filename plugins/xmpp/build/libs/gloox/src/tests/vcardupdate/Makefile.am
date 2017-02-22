##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = vcardupdate_test

vcardupdate_test_SOURCES = vcardupdate_test.cpp
vcardupdate_test_LDADD = ../../vcardupdate.o ../../tag.o ../../gloox.o ../../util.o
vcardupdate_test_CFLAGS = $(CPPFLAGS)
