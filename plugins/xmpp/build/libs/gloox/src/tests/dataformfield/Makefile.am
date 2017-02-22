##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = dataformfield_test

dataformfield_test_SOURCES = dataformfield_test.cpp
dataformfield_test_LDADD = ../../tag.o ../../dataformfield.o ../../util.o ../../gloox.o
dataformfield_test_CFLAGS = $(CPPFLAGS)
