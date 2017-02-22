##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = dataform_test

dataform_test_SOURCES = dataform_test.cpp
dataform_test_LDADD = ../../tag.o ../../dataform.o ../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o ../../dataformfield.o ../../gloox.o ../../util.o
dataform_test_CFLAGS = $(CPPFLAGS)
