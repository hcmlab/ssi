# Microsoft Developer Studio Project File - Name="gloox 1.0" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gloox 1.0 - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE
!MESSAGE NMAKE /f "gloox.mak".
!MESSAGE
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE
!MESSAGE NMAKE /f "gloox.mak" CFG="gloox 1.0 - Win32 Debug"
!MESSAGE
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE
!MESSAGE "gloox 1.0 - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gloox 1.0 - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gloox 1.0 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOOX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOOX_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /dll

!ELSEIF  "$(CFG)" == "gloox 1.0 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOOX_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOOX_EXPORTS" /YX /FD /GZ /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /dll

!ENDIF

# Begin Target

# Name "gloox 1.0 - Win32 Release"
# Name "gloox 1.0 - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\adhoc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\amp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\annotations.cpp
# End Source File
# Begin Source File

SOURCE=.\src\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\src\bookmarkstorage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\capabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\carbons.cpp
# End Source File
# Begin Source File

SOURCE=.\src\chatstate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\chatstatefilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\client.cpp
# End Source File
# Begin Source File

SOURCE=.\src\clientbase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\component.cpp
# End Source File
# Begin Source File

SOURCE=.\src\compressionzlib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectionbosh.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectionhttpproxy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectionsocks5proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpbase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpclient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\connectiontls.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataform.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataformfield.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataformfieldcontainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataformitem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataformreported.cpp
# End Source File
# Begin Source File

SOURCE=.\src\delayeddelivery.cpp
# End Source File
# Begin Source File

SOURCE=.\src\disco.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dns.cpp
# End Source File
# Begin Source File

SOURCE=.\src\error.cpp
# End Source File
# Begin Source File

SOURCE=.\src\eventdispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\featureneg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\flexoff.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gloox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\forward.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gpgencrypted.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gpgsigned.cpp
# End Source File
# Begin Source File

SOURCE=.\src\inbandbytestream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\instantmucroom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\iq.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jid.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lastactivity.cpp
# End Source File
# Begin Source File

SOURCE=.\src\logsink.cpp
# End Source File
# Begin Source File

SOURCE=.\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\src\message.cpp
# End Source File
# Begin Source File

SOURCE=.\src\messageevent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\messageeventfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\messagefilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\messagesession.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mucmessagesession.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mucroom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nickname.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nonsaslauth.cpp
# End Source File
# Begin Source File

SOURCE=.\src\oob.cpp
# End Source File
# Begin Source File

SOURCE=.\src\parser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\prep.cpp
# End Source File
# Begin Source File

SOURCE=.\src\presence.cpp
# End Source File
# Begin Source File

SOURCE=.\src\privacyitem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\privacymanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\privatexml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pubsubevent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pubsubitem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pubsubmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\receipt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\registration.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rosteritem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rostermanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\search.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\src\shim.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\siprofileft.cpp
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestreammanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestreamserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\softwareversion.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stanza.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stanzaextensionfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\subscription.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tag.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsdefault.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsbase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsclient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsclientanon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsserveranon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslbase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslclient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlsschannel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\uniquemucroom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vcard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vcardmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vcardupdate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\version.rc
# End Source File
# Begin Source File

SOURCE=.\src\xhtmlim.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\adhoc.h
# End Source File
# Begin Source File

SOURCE=.\src\adhoccommandprovider.h
# End Source File
# Begin Source File

SOURCE=.\src\adhochandler.h
# End Source File
# Begin Source File

SOURCE=.\src\amp.h
# End Source File
# Begin Source File

SOURCE=.\src\annotations.h
# End Source File
# Begin Source File

SOURCE=.\src\annotationshandler.h
# End Source File
# Begin Source File

SOURCE=.\src\base64.h
# End Source File
# Begin Source File

SOURCE=.\src\bookmarkhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\bookmarkstorage.h
# End Source File
# Begin Source File

SOURCE=.\src\bytestream.h
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamdatahandler.h
# End Source File
# Begin Source File

SOURCE=.\src\bytestreamhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\capabilities.h
# End Source File
# Begin Source File

SOURCE=.\src\carbons.h
# End Source File
# Begin Source File

SOURCE=.\src\chatstate.h
# End Source File
# Begin Source File

SOURCE=.\src\chatstatefilter.h
# End Source File
# Begin Source File

SOURCE=.\src\chatstatehandler.h
# End Source File
# Begin Source File

SOURCE=.\src\client.h
# End Source File
# Begin Source File

SOURCE=.\src\clientbase.h
# End Source File
# Begin Source File

SOURCE=.\src\component.h
# End Source File
# Begin Source File

SOURCE=.\src\compressionbase.h
# End Source File
# Begin Source File

SOURCE=.\src\compressiondatahandler.h
# End Source File
# Begin Source File

SOURCE=.\src\compressionzlib.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionbase.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionbosh.h
# End Source File
# Begin Source File

SOURCE=.\src\connectiondatahandler.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionhttpproxy.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionlistener.h
# End Source File
# Begin Source File

SOURCE=.\src\connectionsocks5proxy.h
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpbase.h
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpclient.h
# End Source File
# Begin Source File

SOURCE=.\src\connectiontcpserver.h
# End Source File
# Begin Source File

SOURCE=.\src\connectiontls.h
# End Source File
# Begin Source File

SOURCE=.\src\dataform.h
# End Source File
# Begin Source File

SOURCE=.\src\dataformfield.h
# End Source File
# Begin Source File

SOURCE=.\src\dataformfieldcontainer.h
# End Source File
# Begin Source File

SOURCE=.\src\dataformitem.h
# End Source File
# Begin Source File

SOURCE=.\src\dataformreported.h
# End Source File
# Begin Source File

SOURCE=.\src\delayeddelivery.h
# End Source File
# Begin Source File

SOURCE=.\src\disco.h
# End Source File
# Begin Source File

SOURCE=.\src\discohandler.h
# End Source File
# Begin Source File

SOURCE=.\src\disconodehandler.h
# End Source File
# Begin Source File

SOURCE=.\src\dns.h
# End Source File
# Begin Source File

SOURCE=.\src\error.h
# End Source File
# Begin Source File

SOURCE=.\src\event.h
# End Source File
# Begin Source File

SOURCE=.\src\eventdispatcher.h
# End Source File
# Begin Source File

SOURCE=.\src\eventhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\featureneg.h
# End Source File
# Begin Source File

SOURCE=.\src\flexoff.h
# End Source File
# Begin Source File

SOURCE=.\src\flexoffhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\forward.h
# End Source File
# Begin Source File

SOURCE=.\src\gloox.h
# End Source File
# Begin Source File

SOURCE=.\src\gpgencrypted.h
# End Source File
# Begin Source File

SOURCE=.\src\gpgsigned.h
# End Source File
# Begin Source File

SOURCE=.\src\inbandbytestream.h
# End Source File
# Begin Source File

SOURCE=.\src\instantmucroom.h
# End Source File
# Begin Source File

SOURCE=.\src\iq.h
# End Source File
# Begin Source File

SOURCE=.\src\iqhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\jid.h
# End Source File
# Begin Source File

SOURCE=.\src\lastactivity.h
# End Source File
# Begin Source File

SOURCE=.\src\lastactivityhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\loghandler.h
# End Source File
# Begin Source File

SOURCE=.\src\logsink.h
# End Source File
# Begin Source File

SOURCE=.\src\macros.h
# End Source File
# Begin Source File

SOURCE=.\src\md5.h
# End Source File
# Begin Source File

SOURCE=.\src\message.h
# End Source File
# Begin Source File

SOURCE=.\src\messageevent.h
# End Source File
# Begin Source File

SOURCE=.\src\messageeventfilter.h
# End Source File
# Begin Source File

SOURCE=.\src\messageeventhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\messagefilter.h
# End Source File
# Begin Source File

SOURCE=.\src\messagehandler.h
# End Source File
# Begin Source File

SOURCE=.\src\messagesession.h
# End Source File
# Begin Source File

SOURCE=.\src\messagesessionhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\mucinvitationhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\mucmessagesession.h
# End Source File
# Begin Source File

SOURCE=.\src\mucroom.h
# End Source File
# Begin Source File

SOURCE=.\src\mucroomconfighandler.h
# End Source File
# Begin Source File

SOURCE=.\src\mucroomhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\mutex.h
# End Source File
# Begin Source File

SOURCE=.\src\mutexguard.h
# End Source File
# Begin Source File

SOURCE=.\src\nickname.h
# End Source File
# Begin Source File

SOURCE=.\src\nonsaslauth.h
# End Source File
# Begin Source File

SOURCE=.\src\oob.h
# End Source File
# Begin Source File

SOURCE=.\src\parser.h
# End Source File
# Begin Source File

SOURCE=.\src\prep.h
# End Source File
# Begin Source File

SOURCE=.\src\presence.h
# End Source File
# Begin Source File

SOURCE=.\src\presencehandler.h
# End Source File
# Begin Source File

SOURCE=.\src\privacyitem.h
# End Source File
# Begin Source File

SOURCE=.\src\privacylisthandler.h
# End Source File
# Begin Source File

SOURCE=.\src\privacymanager.h
# End Source File
# Begin Source File

SOURCE=.\src\privatexml.h
# End Source File
# Begin Source File

SOURCE=.\src\privatexmlhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsub.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsubevent.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsubeventhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsubitem.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsubmanager.h
# End Source File
# Begin Source File

SOURCE=.\src\pubsubresulthandler.h
# End Source File
# Begin Source File

SOURCE=.\src\receipt.h
# End Source File
# Begin Source File

SOURCE=.\src\registration.h
# End Source File
# Begin Source File

SOURCE=.\src\registrationhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\rosteritem.h
# End Source File
# Begin Source File

SOURCE=.\src\rosteritemdata.h
# End Source File
# Begin Source File

SOURCE=.\src\rosterlistener.h
# End Source File
# Begin Source File

SOURCE=.\src\rostermanager.h
# End Source File
# Begin Source File

SOURCE=.\src\search.h
# End Source File
# Begin Source File

SOURCE=.\src\searchhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\sha.h
# End Source File
# Begin Source File

SOURCE=.\src\shim.h
# End Source File
# Begin Source File

SOURCE=.\src\sihandler.h
# End Source File
# Begin Source File

SOURCE=.\src\simanager.h
# End Source File
# Begin Source File

SOURCE=.\src\siprofileft.h
# End Source File
# Begin Source File

SOURCE=.\src\siprofilefthandler.h
# End Source File
# Begin Source File

SOURCE=.\src\siprofilehandler.h
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestream.h
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestreammanager.h
# End Source File
# Begin Source File

SOURCE=.\src\socks5bytestreamserver.h
# End Source File
# Begin Source File

SOURCE=.\src\softwareversion.h
# End Source File
# Begin Source File

SOURCE=.\src\stanza.h
# End Source File
# Begin Source File

SOURCE=.\src\stanzaextension.h
# End Source File
# Begin Source File

SOURCE=.\src\stanzaextensionfactory.h
# End Source File
# Begin Source File

SOURCE=.\src\statisticshandler.h
# End Source File
# Begin Source File

SOURCE=.\src\subscription.h
# End Source File
# Begin Source File

SOURCE=.\src\subscriptionhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\tag.h
# End Source File
# Begin Source File

SOURCE=.\src\taghandler.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsbase.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsdefault.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsbase.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsclient.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsclientanon.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsgnutlsserveranon.h
# End Source File
# Begin Source File

SOURCE=.\src\tlshandler.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslbase.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslclient.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsopensslserver.h
# End Source File
# Begin Source File

SOURCE=.\src\tlsschannel.h
# End Source File
# Begin Source File

SOURCE=.\src\uniquemucroom.h
# End Source File
# Begin Source File

SOURCE=.\src\util.h
# End Source File
# Begin Source File

SOURCE=.\src\vcard.h
# End Source File
# Begin Source File

SOURCE=.\src\vcardhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\vcardmanager.h
# End Source File
# Begin Source File

SOURCE=.\src\vcardupdate.h
# End Source File
# Begin Source File

SOURCE=.\src\xhtmlim.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\config.h.win
# End Source File
# End Target
# End Project
