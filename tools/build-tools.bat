@echo off
rem Compila fairwayc, FairwayED, fwcompile e gbincheck. Precisa de Visual C++ x86 e Python 3.
setlocal
set "TOOLS=%~dp0"
if defined VCVARS goto havevc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
"%VSWHERE%" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -find VC\Auxiliary\Build\vcvarsall.bat > "%TEMP%\fairwayed_vc.txt"
set /p VCVARS=<"%TEMP%\fairwayed_vc.txt"
del "%TEMP%\fairwayed_vc.txt"
:havevc
call "%TOOLS%build-fairwayed.bat" %* || exit /b 1
call "%VCVARS%" x86 >nul || exit /b 1
set "FW=%TOOLS%fairwayed"
if not exist "%TOOLS%bin\obj" mkdir "%TOOLS%bin\obj"
set "CF=/nologo /W3 /O2 /MT /EHsc /D_CRT_SECURE_NO_WARNINGS /Fo"%TOOLS%bin\obj\\" /Fd"%TOOLS%bin\obj\\""
cl %CF% /utf-8 /DUNICODE /D_UNICODE /I"%FW%\src" /I"%FW%\wshared\src" /Fe"%TOOLS%bin\fwcompile.exe" "%TOOLS%fwcompile\fwcompile.cpp" "%FW%\src\wep.cpp" "%FW%\src\gbin.cpp" "%FW%\src\props.cpp" "%FW%\src\areascript.cpp" "%FW%\src\course.cpp" "%FW%\wshared\lib\wshared.lib" user32.lib gdi32.lib gdiplus.lib ole32.lib opengl32.lib glu32.lib || exit /b 1
python "%TOOLS%gbincheck\make_parser.py" "%TOOLS%bin\obj" || exit /b 1
cl %CF% /I"%TOOLS%bin\obj" /Fe"%TOOLS%bin\gbincheck.exe" "%TOOLS%gbincheck\gbincheck.cpp" || exit /b 1
echo Pronto: tools\bin\fwcompile.exe, tools\bin\gbincheck.exe, tools\fairwayed\bin\
