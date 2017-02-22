##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = carbons_test

carbons_test_SOURCES = carbons_test.cpp
carbons_test_LDADD = ../../jid.o ../../tag.o \
                        ../../logsink.o ../../prep.o ../../util.o \
                        ../../gloox.o ../../iq.o ../../stanza.o \
                        ../../error.o ../../message.o \
                        ../../forward.o ../../delayeddelivery.o \
                        ../../clientbase.o ../../client.o \
                        ../../connectiontcpbase.o ../../connectiontcpclient.o \
                        ../../disco.o ../../parser.o ../../base64.o \
                        ../../md5.o ../../tlsgnutlsclient.o ../../tlsopensslclient.o ../../tlsopensslbase.o ../../tlsopensslserver.o ../../tlsschannel.o \
                        ../../messagesession.o ../../compressionzlib.o \
                        ../../dns.o ../../stanzaextensionfactory.o \
                        ../../rostermanager.o ../../nonsaslauth.o ../../sha.o ../../dataform.o \
                        ../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
                        ../../dataformfield.o \
                        ../../rosteritem.o ../../privatexml.o ../../tlsgnutlsbase.o \
                        ../../tlsdefault.o ../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o \
                        ../../mutex.o ../../presence.o ../../subscription.o \
                        ../../capabilities.o ../../eventdispatcher.o \
                        ../../softwareversion.o \
                        ../../atomicrefcount.o ../../attention.o ../../carbons.o
carbons_test_CFLAGS = $(CPPFLAGS)
