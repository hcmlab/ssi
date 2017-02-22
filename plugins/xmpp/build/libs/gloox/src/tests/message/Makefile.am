##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = message_test

message_test_SOURCES = message_test.cpp
message_test_LDADD = ../../tag.o ../../message.o ../../stanza.o ../../jid.o ../../prep.o ../../gloox.o \
                     ../../util.o ../../sha.o ../../base64.o ../../delayeddelivery.o
message_test_CFLAGS = $(CPPFLAGS)
