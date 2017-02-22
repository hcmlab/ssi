##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = gpgsigned_test

gpgsigned_test_SOURCES = gpgsigned_test.cpp
gpgsigned_test_LDADD = ../../gpgsigned.o ../../tag.o ../../gloox.o ../../util.o
gpgsigned_test_CFLAGS = $(CPPFLAGS)
