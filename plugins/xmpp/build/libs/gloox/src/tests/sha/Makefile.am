##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = sha_test

sha_test_SOURCES = sha_test.cpp
sha_test_LDADD = ../../sha.o ../../gloox.o
sha_test_CFLAGS = $(CPPFLAGS)
