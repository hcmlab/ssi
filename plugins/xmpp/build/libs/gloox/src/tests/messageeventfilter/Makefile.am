##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = messageeventfilter_test

messageeventfilter_test_SOURCES = messageeventfilter_test.cpp
messageeventfilter_test_LDADD = ../../tag.o ../../stanza.o \
 				../../jid.o ../../prep.o ../../gloox.o \
				../../message.o ../../util.o \
				../../sha.o ../../base64.o ../../messageevent.o
messageeventfilter_test_CPPFLAGS = $(CPPFLAGS)
