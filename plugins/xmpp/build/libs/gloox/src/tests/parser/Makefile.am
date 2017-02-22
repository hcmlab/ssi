##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -g3 -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = parser_test

parser_test_SOURCES = parser_test.cpp
parser_test_LDADD = ../../parser.o ../../tag.o ../../util.o ../../gloox.o
parser_test_CFLAGS = $(CPPFLAGS)
