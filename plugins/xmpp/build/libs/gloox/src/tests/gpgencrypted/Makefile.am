##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = gpgencrypted_test

gpgencrypted_test_SOURCES = gpgencrypted_test.cpp
gpgencrypted_test_LDADD = ../../gpgencrypted.o ../../tag.o ../../gloox.o ../../util.o
gpgencrypted_test_CFLAGS = $(CPPFLAGS)
