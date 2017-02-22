##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -Wno-long-long

noinst_PROGRAMS = client_test

client_test_SOURCES = client_test.cpp
client_test_LDADD = ../../client.o ../../connectiontcpbase.o ../../connectiontcpclient.o \
			../../disco.o ../../parser.o ../../tag.o ../../stanza.o ../../base64.o ../../jid.o \
			../../md5.o ../../tlsgnutlsclient.o ../../tlsopensslclient.o ../../tlsopensslbase.o ../../tlsopensslserver.o ../../tlsschannel.o \
			../../logsink.o ../../messagesession.o ../../prep.o ../../compressionzlib.o \
			../../dns.o ../../stanzaextensionfactory.o \
			../../rostermanager.o ../../nonsaslauth.o ../../sha.o ../../dataform.o \
			../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
			../../dataformfield.o \
			../../rosteritem.o ../../privatexml.o ../../gloox.o ../../tlsgnutlsbase.o \
			../../tlsdefault.o ../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o \
			../../mutex.o ../../iq.o ../../presence.o ../../message.o ../../subscription.o \
			../../util.o ../../error.o ../../capabilities.o ../../eventdispatcher.o \
			../../softwareversion.o \
			../../atomicrefcount.o
client_test_CFLAGS = $(CPPFLAGS)
