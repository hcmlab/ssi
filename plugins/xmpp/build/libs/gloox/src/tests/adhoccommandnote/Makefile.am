##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = adhoccommandnote_test

adhoccommandnote_test_SOURCES = adhoccommandnote_test.cpp
adhoccommandnote_test_LDADD = ../../adhoc.o ../../connectiontcpclient.o ../../connectiontcpbase.o \
			../../disco.o ../../parser.o ../../tag.o ../../stanza.o ../../base64.o \
			../../md5.o ../../tlsgnutlsclient.o ../../tlsopensslclient.o ../../tlsopensslbase.o ../../tlsopensslserver.o ../../tlsschannel.o \
			../../logsink.o ../../messagesession.o ../../prep.o ../../compressionzlib.o \
			../../dns.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../tlsgnutlsbase.o ../../tlsdefault.o \
			../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o ../../mutex.o \
			../../iq.o ../../presence.o ../../message.o ../../subscription.o ../../util.o \
			../../sha.o ../../error.o ../../clientbase.o ../../jid.o \
			../../dataform.o ../../dataformfieldcontainer.o ../../dataformreported.o \
			../../dataformitem.o ../../dataformfield.o ../../eventdispatcher.o ../../softwareversion.o \
			../../atomicrefcount.o ../../iodata.o
adhoccommandnote_test_CFLAGS = $(CPPFLAGS)
