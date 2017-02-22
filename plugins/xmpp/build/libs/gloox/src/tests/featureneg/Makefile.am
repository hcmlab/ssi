##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = featureneg_test

featureneg_test_SOURCES = featureneg_test.cpp
featureneg_test_LDADD = ../../tag.o ../../dataform.o ../../dataformfieldcontainer.o ../../dataformreported.o \
                        ../../dataformitem.o ../../dataformfield.o ../../gloox.o ../../util.o \
                        ../../featureneg.o ../../stanzaextensionfactory.o ../../iq.o ../../message.o \
                        ../../stanza.o ../../jid.o ../../prep.o ../../mutex.o
featureneg_test_CFLAGS = $(CPPFLAGS)
