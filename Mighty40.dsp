# Microsoft Developer Studio Project File - Name="Mighty40" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Mighty40 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Mighty40.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mighty40.mak" CFG="Mighty40 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mighty40 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Mighty40 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Mighty40 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib winmm.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Mighty40 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib winmm.lib nafxcw.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Mighty40 - Win32 Release"
# Name "Mighty40 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BmpMan.cpp
# End Source File
# Begin Source File

SOURCE=.\Board.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_DSB.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_Layout.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_SE.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_UI.cpp
# End Source File
# Begin Source File

SOURCE=.\Board_Wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\Card.cpp
# End Source File
# Begin Source File

SOURCE=.\D2MA.cpp
# End Source File
# Begin Source File

SOURCE=.\DAddRule.cpp
# End Source File
# Begin Source File

SOURCE=.\DBackSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\DConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\DDetailScore.cpp
# End Source File
# Begin Source File

SOURCE=.\DElection.cpp
# End Source File
# Begin Source File

SOURCE=.\DEtc.cpp
# End Source File
# Begin Source File

SOURCE=.\DMaiInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DOption.cpp
# End Source File
# Begin Source File

SOURCE=.\DReport.cpp
# End Source File
# Begin Source File

SOURCE=.\DRule.cpp
# End Source File
# Begin Source File

SOURCE=.\DSB.cpp
# End Source File
# Begin Source File

SOURCE=.\DSB_Fill.cpp
# End Source File
# Begin Source File

SOURCE=.\DScoreBoard.cpp
# End Source File
# Begin Source File

SOURCE=.\DStartUp.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MaiBSW.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MFSM.cpp
# End Source File
# Begin Source File

SOURCE=.\MFSM_AISupp.cpp
# End Source File
# Begin Source File

SOURCE=.\MFSM_Event.cpp
# End Source File
# Begin Source File

SOURCE=.\MFSM_Server.cpp
# End Source File
# Begin Source File

SOURCE=.\Mighty.cpp
# End Source File
# Begin Source File

SOURCE=.\MightyToolTip.cpp
# End Source File
# Begin Source File

SOURCE=.\Msg.cpp
# End Source File
# Begin Source File

SOURCE=.\Option.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerHuman.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerMai.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerNetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionAI.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionBackground.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionBosskey.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionCard.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionComm.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionDSB.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionRule.cpp
# End Source File
# Begin Source File

SOURCE=.\POptionSpeed.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleCard.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleConstrain.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleDealMiss.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleElection.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleScore.cpp
# End Source File
# Begin Source File

SOURCE=.\PRuleSpecial.cpp
# End Source File
# Begin Source File

SOURCE=.\Rule.cpp
# End Source File
# Begin Source File

SOURCE=.\SocketBag.cpp
# End Source File
# Begin Source File

SOURCE=.\State.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\ZSocket.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BmpMan.h
# End Source File
# Begin Source File

SOURCE=.\Board.h
# End Source File
# Begin Source File

SOURCE=.\BoardWrap.h
# End Source File
# Begin Source File

SOURCE=.\Card.h
# End Source File
# Begin Source File

SOURCE=.\D2MA.h
# End Source File
# Begin Source File

SOURCE=.\DAddRule.h
# End Source File
# Begin Source File

SOURCE=.\DBackSelect.h
# End Source File
# Begin Source File

SOURCE=.\DConnect.h
# End Source File
# Begin Source File

SOURCE=.\DDetailScore.h
# End Source File
# Begin Source File

SOURCE=.\DElection.h
# End Source File
# Begin Source File

SOURCE=.\DEtc.h
# End Source File
# Begin Source File

SOURCE=.\DMaiInfo.h
# End Source File
# Begin Source File

SOURCE=.\DOption.h
# End Source File
# Begin Source File

SOURCE=.\DReport.h
# End Source File
# Begin Source File

SOURCE=.\DRule.h
# End Source File
# Begin Source File

SOURCE=.\DSB.h
# End Source File
# Begin Source File

SOURCE=.\DScoreBoard.h
# End Source File
# Begin Source File

SOURCE=.\DStartUp.h
# End Source File
# Begin Source File

SOURCE=.\InfoBar.h
# End Source File
# Begin Source File

SOURCE=.\MaiBSW.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MFSM.h
# End Source File
# Begin Source File

SOURCE=.\MFSM_Notify.h
# End Source File
# Begin Source File

SOURCE=.\Mighty.h
# End Source File
# Begin Source File

SOURCE=.\MightyToolTip.h
# End Source File
# Begin Source File

SOURCE=.\Msg.h
# End Source File
# Begin Source File

SOURCE=.\Option.h
# End Source File
# Begin Source File

SOURCE=.\Play.h
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\PlayerHuman.h
# End Source File
# Begin Source File

SOURCE=.\PlayerMai.h
# End Source File
# Begin Source File

SOURCE=.\PlayerNetwork.h
# End Source File
# Begin Source File

SOURCE=.\PlayerSocket.h
# End Source File
# Begin Source File

SOURCE=.\POptionAI.h
# End Source File
# Begin Source File

SOURCE=.\POptionBackground.h
# End Source File
# Begin Source File

SOURCE=.\POptionBosskey.h
# End Source File
# Begin Source File

SOURCE=.\POptionCard.h
# End Source File
# Begin Source File

SOURCE=.\POptionComm.h
# End Source File
# Begin Source File

SOURCE=.\POptionDSB.h
# End Source File
# Begin Source File

SOURCE=.\POptionGeneral.h
# End Source File
# Begin Source File

SOURCE=.\POptionRule.h
# End Source File
# Begin Source File

SOURCE=.\POptionSpeed.h
# End Source File
# Begin Source File

SOURCE=.\PRuleCard.h
# End Source File
# Begin Source File

SOURCE=.\PRuleConstrain.h
# End Source File
# Begin Source File

SOURCE=.\PRuleDealMiss.h
# End Source File
# Begin Source File

SOURCE=.\PRuleElection.h
# End Source File
# Begin Source File

SOURCE=.\PRuleGeneral.h
# End Source File
# Begin Source File

SOURCE=.\PRuleScore.h
# End Source File
# Begin Source File

SOURCE=.\PRuleSpecial.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Rule.h
# End Source File
# Begin Source File

SOURCE=.\SocketBag.h
# End Source File
# Begin Source File

SOURCE=.\State.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ZSocket.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Hand.cur
# End Source File
# Begin Source File

SOURCE=.\res\iex.ico
# End Source File
# Begin Source File

SOURCE=.\res\Mighty.ico
# End Source File
# Begin Source File

SOURCE=.\Mighty.rc
# End Source File
# Begin Source File

SOURCE=.\res\mycom.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=".\res\게임시작.wav"
# End Source File
# Begin Source File

SOURCE=".\res\공약.wav"
# End Source File
# Begin Source File

SOURCE=".\res\공지.wav"
# End Source File
# Begin Source File

SOURCE=".\res\득점.wav"
# End Source File
# Begin Source File

SOURCE=".\res\마이티.wav"
# End Source File
# Begin Source File

SOURCE=".\res\삑.wav"
# End Source File
# Begin Source File

SOURCE=".\res\섞기.wav"
# End Source File
# Begin Source File

SOURCE=".\res\승리.wav"
# End Source File
# Begin Source File

SOURCE=".\res\조커.wav"
# End Source File
# Begin Source File

SOURCE=".\res\조커콜.wav"
# End Source File
# Begin Source File

SOURCE=".\res\죽은조커.wav"
# End Source File
# Begin Source File

SOURCE=".\res\찍기.wav"
# End Source File
# Begin Source File

SOURCE=".\res\철컥.wav"
# End Source File
# Begin Source File

SOURCE=".\res\카드.wav"
# End Source File
# Begin Source File

SOURCE=".\res\카드받기.wav"
# End Source File
# Begin Source File

SOURCE=".\res\카드셋.wav"
# End Source File
# Begin Source File

SOURCE=".\res\패배.wav"
# End Source File
# Begin Source File

SOURCE=".\res\포기.wav"
# End Source File
# End Target
# End Project
