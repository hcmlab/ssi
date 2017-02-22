
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = rostermanagerquery_test

rostermanagerquery_test_SOURCES = rostermanagerquery_test.cpp
rostermanagerquery_test_LDADD = ../../rostermanager.o ../../connectiontcpclient.o ../../connectiontcpbase.o \
			../../disco.o ../../parser.o ../../tag.o ../../stanza.o ../../base64.o \
			../../md5.o ../../tlsgnutlsclient.o ../../tlsopensslclient.o ../../tlsopensslbase.o ../../tlsopensslserver.o ../../tlsschannel.o \
			../../logsink.o ../../messagesession.o ../../prep.o ../../compressionzlib.o \
			../../dns.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../tlsgnutlsbase.o ../../tlsdefault.o ../../privatexml.o \
			../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o ../../mutex.o \
			../../iq.o ../../presence.o ../../message.o ../../subscription.o ../../util.o \
			../../sha.o ../../error.o ../../clientbase.o ../../jid.o ../../rosteritem.o \
			../../capabilities.o ../../dataform.o \
			../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
			../../dataformfield.o ../../eventdispatcher.o\
			../../softwareversion.o \
			../../atomicrefcount.o
rostermanagerquery_test_CFLAGS = $(CPPFLAGS)
