##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = dataformreported_test

dataformreported_test_SOURCES = dataformreported_test.cpp
dataformreported_test_LDADD = ../../dataformreported.o ../../tag.o \
		../../dataform.o ../../gloox.o ../../dataformfieldcontainer.o ../../dataformfield.o ../../dataformitem.o \
		../../util.o
dataformreported_test_CFLAGS = $(CPPFLAGS)
