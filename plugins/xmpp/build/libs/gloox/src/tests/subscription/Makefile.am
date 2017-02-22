##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = subscription_test

subscription_test_SOURCES = subscription_test.cpp
subscription_test_LDADD = ../../tag.o ../../subscription.o ../../stanza.o ../../jid.o ../../prep.o ../../gloox.o \
                          ../../util.o ../../sha.o ../../base64.o
subscription_test_CFLAGS = $(CPPFLAGS)
