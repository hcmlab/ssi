##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = zlib_test zlib_perf

zlib_test_SOURCES = zlib_test.cpp
zlib_test_LDADD = ../../compressionzlib.o ../../mutex.o
zlib_test_CFLAGS = $(CPPFLAGS)

zlib_perf_SOURCES = zlib_perf.cpp
zlib_perf_LDADD = ../../compressionzlib.o ../../mutex.o
zlib_perf_CFLAGS = $(CPPFLAGS)
