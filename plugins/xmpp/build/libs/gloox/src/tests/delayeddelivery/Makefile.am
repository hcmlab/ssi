##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = delayeddelivery_test

delayeddelivery_test_SOURCES = delayeddelivery_test.cpp
delayeddelivery_test_LDADD = ../../delayeddelivery.o ../../tag.o \
		../../jid.o ../../prep.o ../../gloox.o ../../util.o
delayeddelivery_test_CFLAGS = $(CPPFLAGS)
