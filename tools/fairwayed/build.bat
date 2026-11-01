@echo off
rem SPDX-License-Identifier: MIT
rem Builds bin\FairwayED.exe (the editor) and bin\fairwayc.exe (batch verifier).
rem Needs a Visual C++ install with the x86 tools. Win32 API, OpenGL 1.1 and GDI+ only.
rem Puppets, textures and effects come from the wshared submodule, which this builds first;
rem set WSHARED to build against another copy.
setlocal
if defined VCVARS goto havevc
set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto havevc
for /f "usebackq delims=" %%i in (`""%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -find VC\Auxiliary\Build\vcvarsall.bat"`) do set "VCVARS=%%i"
:havevc
call "%VCVARS%" x86 >nul || exit /b 1

set ROOT=%~dp0
if not defined WSHARED set "WSHARED=%ROOT%wshared"
if not exist "%WSHARED%\build.bat" (
  echo wshared is missing. Run: git submodule update --init
  exit /b 1
)
call "%WSHARED%\build.bat" || exit /b 1
if not exist "%ROOT%bin" mkdir "%ROOT%bin"
if not exist "%ROOT%obj" mkdir "%ROOT%obj"
set CFLAGS=/nologo /W3 /O2 /MT /EHsc /utf-8 /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /I"%WSHARED%\src" /Fo"%ROOT%obj\\" /Fd"%ROOT%obj\\"
set CORE=%ROOT%src\wep.cpp %ROOT%src\gbin.cpp %ROOT%src\props.cpp %ROOT%src\areascript.cpp %ROOT%src\course.cpp "%WSHARED%\lib\wshared.lib"

cl %CFLAGS% /Fe"%ROOT%bin\fairwayc.exe" %ROOT%src\fairwayc.cpp %CORE% user32.lib gdi32.lib gdiplus.lib ole32.lib opengl32.lib glu32.lib || exit /b 1
if "%1"=="core" exit /b 0

rc /nologo /fo "%ROOT%obj\fairwayed.res" "%ROOT%src\fairwayed.rc" || exit /b 1
cl %CFLAGS% /Fe"%ROOT%bin\FairwayED.exe" %ROOT%src\main.cpp %ROOT%src\panels.cpp %ROOT%src\viewport.cpp %ROOT%src\scene.cpp %ROOT%src\areafx.cpp %CORE% "%ROOT%obj\fairwayed.res" ^
  /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib opengl32.lib glu32.lib gdiplus.lib ole32.lib advapi32.lib || exit /b 1
