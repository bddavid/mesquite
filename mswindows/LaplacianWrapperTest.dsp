# Microsoft Developer Studio Project File - Name="laplacian wrapper test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=laplacian wrapper test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LaplacianWrapperTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LaplacianWrapperTest.mak" CFG="laplacian wrapper test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "laplacian wrapper test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "laplacian wrapper test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "laplacian wrapper test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "LaplacianWrapper_Release"
# PROP Intermediate_Dir "LaplacianWrapper_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../include" /I "../include" /I "../src/Mesh" /I "../src/QualityMetric" /I "../src/QualityImprover" /I "../src/QualityImprover/VertexMover" /I "../src/Misc" /I "../src/Control" /I "../src/QualityAssessor" /I "../src/ObjectiveFunction" /I "../src/QualityMetric/Shape" /I "../src/QualityMetric/DFT" /I "../src/QualityMetric/Untangle" /I "../src/QualityImprover/VertexMover/FeasibleNewton" /I "../src/QualityImprover/VertexMover/ConjugateGradient" /I "../src/QualityImprover/VertexMover/SteepestDescent" /I "../src/QualityImprover/VertexMover/LaplacianSmoothers" /I "../src/TargetCalculator" /I "../src/QualityMetric/Smoothness" /I "../src/Control/Wrappers" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "MESQUITE_STATIC_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 mesquite.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"Release_Lib"

!ELSEIF  "$(CFG)" == "laplacian wrapper test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "LaplacianWrapper_Debug"
# PROP Intermediate_Dir "LaplacianWrapper_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../include" /I "../src/Mesh" /I "../src/QualityMetric" /I "../src/QualityImprover" /I "../src/QualityImprover/VertexMover" /I "../../include" /I "../src/Misc" /I "../src/Control" /I "../src/QualityAssessor" /I "../src/ObjectiveFunction" /I "../src/QualityMetric/Shape" /I "../src/QualityMetric/DFT" /I "../src/QualityMetric/Untangle" /I "../src/QualityImprover/VertexMover/FeasibleNewton" /I "../src/QualityImprover/VertexMover/ConjugateGradient" /I "../src/QualityImprover/VertexMover/SteepestDescent" /I "../src/QualityImprover/VertexMover/LaplacianSmoothers" /I "../src/TargetCalculator" /I "../src/QualityMetric/Smoothness" /I "../src/Control/Wrappers" /D "_DEBUG" /D "MESQUITE_STATIC_LIB" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mesquite.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"Debug_Lib"

!ENDIF 

# Begin Target

# Name "laplacian wrapper test - Win32 Release"
# Name "laplacian wrapper test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\testSuite\laplacian_wrapper_test\main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
