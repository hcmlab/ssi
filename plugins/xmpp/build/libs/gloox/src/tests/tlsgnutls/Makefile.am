##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = tlsgnutls_test

tlsgnutls_test_SOURCES = tlsgnutls_test.cpp
tlsgnutls_test_LDADD = ../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o ../../tlsgnutlsbase.o ../../gloox.o ../../mutex.o
tlsgnutls_test_CFLAGS = $(CPPFLAGS)
