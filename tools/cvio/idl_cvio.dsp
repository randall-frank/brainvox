# Microsoft Developer Studio Project File - Name="idl_cvio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=idl_cvio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "idl_cvio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "idl_cvio.mak" CFG="idl_cvio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "idl_cvio - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "idl_cvio - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "idl_cvio - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDL_CVIO_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "zlib-1.1.3" /I "C:\rsi\IDL53\external" /I "C:\srcs\rsi\idl52\external" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDL_CVIO_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 zlib\zlib.lib idl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"idl_cvio.dll" /libpath:"c:\srcs\rsi\idl52" /libpath:"c:\rsi\idl53\bin\bin.x86"

!ELSEIF  "$(CFG)" == "idl_cvio - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDL_CVIO_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "zlib-1.1.3" /I "C:\rsi\IDL53\external" /I "C:\srcs\rsi\idl52\external" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IDL_CVIO_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 idl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"idl_cvio.dll" /pdbtype:sept /libpath:"c:\srcs\rsi\idl52" /libpath:"c:\rsi\idl53\bin\bin.x86"

!ENDIF 

# Begin Target

# Name "idl_cvio - Win32 Release"
# Name "idl_cvio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cvio_lib.c
# End Source File
# Begin Source File

SOURCE=.\idl_cvio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cvio_lib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Extras"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\idl_cvio.def
# End Source File
# Begin Source File

SOURCE=.\idl_cvio.dlm
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\zlib-1.1.3\adler32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\compress.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\crc32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\deflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\deflate.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\gzio.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infblock.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infblock.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infcodes.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infcodes.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffast.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffast.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffixed.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inftrees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inftrees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infutil.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\maketree.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\trees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\trees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\uncompr.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zconf.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zlib.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zutil.h"
# End Source File
# End Group
# End Target
# End Project
