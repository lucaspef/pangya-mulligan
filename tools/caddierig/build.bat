@echo off
rem SPDX-License-Identifier: MIT
rem Builds bin\CaddieRig.exe (GUI) and bin\crigc.exe (batch compiler).
rem Needs a Visual C++ install with the x86 tools. Win32 API, OpenGL 1.1 and GDI+ only.
rem Puppets, textures and effects come from the wshared submodule, which this builds first;
rem set WSHARED to build against another copy.
setlocal
rem The newest Visual Studio is not always the one with the C++ headers, so prefer
rem an explicit VCVARS, then the 2022 build tools, then whatever vswhere reports.
if defined VCVARS goto havevc
set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto havevc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`call "%VSWHERE%" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -find VC\Auxiliary\Build\vcvarsall.bat`) do set "VCVARS=%%i"
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
set CORE=%ROOT%src\wardrobe.cpp %ROOT%src\puppetscript.cpp %ROOT%src\pmpfile.cpp %ROOT%src\asefile.cpp %ROOT%src\compile.cpp "%WSHARED%\lib\wshared.lib"

cl %CFLAGS% /Fe"%ROOT%bin\crigc.exe" %ROOT%src\crigc.cpp %ROOT%src\document.cpp %ROOT%src\audio.cpp %CORE% user32.lib winmm.lib gdi32.lib gdiplus.lib ole32.lib opengl32.lib glu32.lib || exit /b 1
if "%1"=="core" exit /b 0

rc /nologo /fo "%ROOT%obj\caddierig.res" "%ROOT%src\caddierig.rc" || exit /b 1
cl %CFLAGS% /Fe"%ROOT%bin\CaddieRig.exe" %ROOT%src\main.cpp %ROOT%src\view.cpp %ROOT%src\document.cpp %ROOT%src\audio.cpp %CORE% "%ROOT%obj\caddierig.res" ^
  /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib opengl32.lib glu32.lib gdiplus.lib ole32.lib advapi32.lib winmm.lib || exit /b 1
