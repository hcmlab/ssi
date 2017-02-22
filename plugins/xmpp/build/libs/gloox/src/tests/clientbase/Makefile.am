##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -Wno-long-long

noinst_PROGRAMS = clientbase_test

clientbase_test_SOURCES = clientbase_test.cpp
clientbase_test_LDADD = ../../clientbase.o ../../jid.o ../../connectiontcpclient.o ../../connectiontcpbase.o \
			../../disco.o ../../parser.o ../../tag.o ../../stanza.o ../../base64.o \
			../../md5.o ../../tlsgnutlsclient.o ../../tlsopensslclient.o ../../tlsopensslbase.o ../../tlsopensslserver.o ../../tlsschannel.o \
			../../logsink.o ../../messagesession.o ../../prep.o ../../compressionzlib.o \
			../../dns.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../tlsgnutlsbase.o ../../tlsdefault.o ../../dataform.o \
			../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
			../../dataformfield.o \
			../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o ../../mutex.o \
			../../iq.o ../../presence.o ../../message.o ../../subscription.o ../../util.o \
			../../sha.o ../../error.o ../../eventdispatcher.o ../../softwareversion.o \
			../../atomicrefcount.o
clientbase_test_CFLAGS = $(CPPFLAGS)
