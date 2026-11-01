@echo off
rem SPDX-License-Identifier: MIT
rem Builds lib\wshared.lib. FairwayED and CaddieRig call this from their own builds.
rem Needs a Visual C++ install with the x86 tools.
setlocal
where cl >nul 2>&1 && goto havecl
if defined VCVARS goto havevc
set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto havevc
for /f "usebackq delims=" %%i in (`""%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -find VC\Auxiliary\Build\vcvarsall.bat"`) do set "VCVARS=%%i"
:havevc
call "%VCVARS%" x86 >nul || exit /b 1
:havecl

set ROOT=%~dp0
if not exist "%ROOT%lib" mkdir "%ROOT%lib"
if not exist "%ROOT%obj" mkdir "%ROOT%obj"
cl /c /nologo /W3 /O2 /MT /EHsc /utf-8 /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /Fo"%ROOT%obj\\" /Fd"%ROOT%obj\\" ^
  %ROOT%src\wcommon.cpp %ROOT%src\petfile.cpp %ROOT%src\texture.cpp %ROOT%src\fxsim.cpp %ROOT%src\fxdraw.cpp || exit /b 1
lib /nologo /OUT:"%ROOT%lib\wshared.lib" "%ROOT%obj\wcommon.obj" "%ROOT%obj\petfile.obj" "%ROOT%obj\texture.obj" "%ROOT%obj\fxsim.obj" "%ROOT%obj\fxdraw.obj" || exit /b 1
