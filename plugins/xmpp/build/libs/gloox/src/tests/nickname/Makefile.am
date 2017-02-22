##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = nickname_test

nickname_test_SOURCES = nickname_test.cpp
nickname_test_LDADD = ../../nickname.o ../../gloox.o ../../tag.o ../../util.o
nickname_test_CFLAGS = $(CPPFLAGS)
