##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = dataformitem_test

dataformitem_test_SOURCES = dataformitem_test.cpp
dataformitem_test_LDADD = ../../dataformreported.o ../../tag.o \
		../../dataform.o ../../gloox.o ../../dataformfieldcontainer.o ../../dataformfield.o ../../dataformitem.o \
		../../util.o
dataformitem_test_CFLAGS = $(CPPFLAGS)
