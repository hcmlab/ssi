##
## Process this file with automake to produce Makefile.in
##

SUBDIRS = adhoc adhoccommand adhoccommandnote amprule amp base64 \
          capabilities carbons chatstatefilter client clientbase connectionbosh \
          dataform dataformfield \
          dataformreported dataformitem delayeddelivery discoinfo discoitems disco \
          error \
          featureneg flexoffline flexofflineoffline forward \
          gpgencrypted gpgsigned \
          inbandbytestreamibb inbandbytestream iodata iq \
          jid jingleiceudp jinglesession jinglesessionjingle jinglesessionmanager \
          lastactivity lastactivityquery \
          md5 message messageeventfilter \
          mucroommuc mucroommucadmin mucroommucowner mucroommucuser \
          nickname nonsaslauthquery nonsaslauth \
          oob \
          parser prep presence privacymanager privacymanagerquery \
          privatexml \
          pubsubmanagerpubsub pubsubmanager pubsubevent\
          receipt \
          registrationquery registration \
          rostermanagerquery rostermanager \
          searchquery search \
          sha shim \
          simanager simanagersi stanzaextensionfactory subscription \
          tag tlsgnutls \
          uniquemucroomunique \
          vcard vcardupdate \
          xpath \
          zlib util

check: test

test: ${SUBDIRS}
	@for i in $?; do \
	  ${MAKE} -C $$i all || exit; \
	done;
	@for i in $?; do \
	  $$i/$${i}_test;    \
	done;

leaktest: leakcheck

leakcheck: ${SUBDIRS}
	@for i in $?; do \
	  ${MAKE} -C $$i all || exit; \
	done;
	@for i in $?; do \
	  valgrind -q --leak-check=full --show-reachable=yes $$i/$${i}_test;    \
	done;
