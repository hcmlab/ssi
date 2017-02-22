#-----------------------------------------------------------------------------
# Initialize project.

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	if(NOT DEFINED WITH_IN_SOURCE_BUILD)
		message(FATAL_ERROR
			"CMake generation for openssi is not allowed within the source directory!"
			"\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
			"\n "
			"\n rm CMakeCache.txt"
			"\n cd .."
			"\n mkdir cmake-make"
			"\n cd cmake-make"
			"\n cmake ../core"
			"\n "
			"\n Alternately define WITH_IN_SOURCE_BUILD to force this option (not recommended!)"
		)		
	endif()
endif()

#http://stackoverflow.com/questions/25609692/how-to-add-source-files-in-another-folder
#file(GLOB Dir1_Sources RELATIVE "Dir1" "*.cpp")
#http://www.opentissue.org/mediawiki/index.php/Using_CMake
cmake_minimum_required(VERSION 2.8)

project(gloox)

include_directories(./src  )



set(COMMON_SRC_FILES

src/adhoc.cpp
src/amp.cpp
src/annotations.cpp
src/atomicrefcount.cpp
src/attention.cpp
src/base64.cpp
src/bookmarkstorage.cpp
src/capabilities.cpp
src/carbons.cpp
src/chatstate.cpp
src/chatstatefilter.cpp
src/client.cpp
src/clientbase.cpp
src/component.cpp
src/compressiondefault.cpp
src/compressionzlib.cpp
src/connectionbosh.cpp
src/connectionhttpproxy.cpp
src/connectionsocks5proxy.cpp
src/connectiontcpbase.cpp
src/connectiontcpclient.cpp
src/connectiontcpserver.cpp
src/connectiontls.cpp
src/connectiontlsserver.cpp
src/dataform.cpp
src/dataformfield.cpp
src/dataformfieldcontainer.cpp
src/dataformitem.cpp
src/dataformreported.cpp
src/delayeddelivery.cpp
src/disco.cpp
src/dns.cpp
src/error.cpp
src/eventdispatcher.cpp
src/featureneg.cpp
src/flexoff.cpp
src/forward.cpp
src/gloox.cpp
src/gpgencrypted.cpp
src/gpgsigned.cpp
src/inbandbytestream.cpp
src/instantmucroom.cpp
src/iodata.cpp
src/iq.cpp
src/jid.cpp
src/jinglecontent.cpp
src/jinglefiletransfer.cpp
src/jingleiceudp.cpp
src/jinglepluginfactory.cpp
src/jinglesession.cpp
src/jinglesessionmanager.cpp
src/lastactivity.cpp
src/linklocalclient.cpp
src/linklocalmanager.cpp
src/logsink.cpp
src/md5.cpp
src/message.cpp
src/messageevent.cpp
src/messageeventfilter.cpp
src/messagefilter.cpp
src/messagesession.cpp
src/mucinvitationhandler.cpp
src/mucmessagesession.cpp
src/mucroom.cpp
src/mutex.cpp
src/nickname.cpp
src/nonsaslauth.cpp
src/oob.cpp
src/parser.cpp
src/prep.cpp
src/presence.cpp
src/privacyitem.cpp
src/privacymanager.cpp
src/privatexml.cpp
src/pubsubevent.cpp
src/pubsubitem.cpp
src/pubsubmanager.cpp
src/receipt.cpp
src/registration.cpp
src/rosteritem.cpp
src/rostermanager.cpp
src/search.cpp
src/sha.cpp
src/shim.cpp
src/simanager.cpp
src/siprofileft.cpp
src/socks5bytestream.cpp
src/socks5bytestreammanager.cpp
src/socks5bytestreamserver.cpp
src/softwareversion.cpp
src/stanza.cpp
src/stanzaextensionfactory.cpp
src/subscription.cpp
src/tag.cpp
src/tlsdefault.cpp
src/tlsgnutlsbase.cpp
src/tlsgnutlsclient.cpp
src/tlsgnutlsclientanon.cpp
src/tlsgnutlsserveranon.cpp
src/tlsopensslbase.cpp
src/tlsopensslclient.cpp
src/tlsopensslserver.cpp
src/tlsschannel.cpp
src/uniquemucroom.cpp
src/util.cpp
src/vcard.cpp
src/vcardmanager.cpp
src/vcardupdate.cpp
src/xhtmlim.cpp


)
 #set_source_files_properties(
 #PROPERTIES LANGUAGE CXX )

set(SRC_FILES ${COMMON_SRC_FILES} )

#find_library(OPENSSI_CORE_LIBRARY NAMES openssi-core HINTS ../../cmake-core/corelib/Debug/ ../../cmake-core/corelib/Release/)

IF(UNIX)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb  -fPIC" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -ggdb  -fPIC" )
ENDIF(UNIX)

add_library(gloox STATIC ${SRC_FILES} )

#add_executable(myapp main.c)

IF(UNIX)
target_link_libraries(gloox ssi ssiml -ldl )
ELSE(UNIX)
target_link_libraries(gloox ssi ssiml )
ENDIF(UNIX)
#rename targets if debug
set_target_properties(gloox PROPERTIES DEBUG_POSTFIX "d" PREFIX "")

#install target to ssi install path set in base directory
install(TARGETS gloox DESTINATION ${SSI_INSTALL}/${SSI_PLATFORM}/${SSI_COMPILER})
