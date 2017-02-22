##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = md5_test

md5_test_SOURCES = md5_test.cpp
md5_test_LDADD = ../../md5.o
md5_test_CFLAGS = $(CPPFLAGS)
