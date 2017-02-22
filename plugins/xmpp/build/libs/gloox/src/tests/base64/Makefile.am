##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -g3 -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = base64_test

base64_test_SOURCES = base64_test.cpp
base64_test_LDADD = ../../base64.o
base64_test_CFLAGS = $(CPPFLAGS)

noinst_HEADERS =
