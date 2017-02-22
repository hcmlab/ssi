##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = jinglesession_test

jinglesession_test_SOURCES = jinglesession_test.cpp
jinglesession_test_LDADD = ../../tag.o ../../stanza.o ../../base64.o \
			../../prep.o ../../gloox.o \
			../../iq.o ../../util.o \
			../../sha.o ../../error.o ../../jid.o \
			../../jinglecontent.o ../../jinglepluginfactory.o
jinglesession_test_CFLAGS = $(CPPFLAGS) -g3
