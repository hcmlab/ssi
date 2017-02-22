##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = jid_test jid_perf

jid_test_SOURCES = jid_test.cpp
jid_test_LDADD = ../../jid.o ../../prep.o ../../gloox.o ../../util.o
jid_test_CFLAGS = $(CPPFLAGS)

jid_perf_SOURCES = jid_perf.cpp
jid_perf_LDADD = ../../jid.o ../../prep.o ../../gloox.o ../../util.o
jid_perf_CFLAGS = $(CPPFLAGS)
