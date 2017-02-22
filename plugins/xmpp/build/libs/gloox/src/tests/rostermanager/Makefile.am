##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = rostermanager_test

rostermanager_test_SOURCES = rostermanager_test.cpp
rostermanager_test_LDADD = ../../tag.o ../../stanza.o ../../base64.o \
			../../prep.o \
			../../gloox.o \
			../../iq.o ../../presence.o ../../message.o ../../subscription.o ../../util.o \
			../../sha.o ../../error.o ../../jid.o ../../rosteritem.o ../../dataform.o \
			../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
			../../dataformfield.o
rostermanager_test_CFLAGS = $(CPPFLAGS)
