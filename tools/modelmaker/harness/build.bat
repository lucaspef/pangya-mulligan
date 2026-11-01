@echo off
rem Compila pethost.exe: o codigo REAL de client\src\engine + o harness. Requer o client ja compilado (terc.lib, wangreal.dll).
rem Variaveis: JP983_D3DX_INCLUDE e JP983_D3DX_LIB (client\COMPILAR.md) e CLIENT_BUILD (padrao client\build\vs2022\Release).
setlocal
set "HERE=%~dp0"
for %%i in ("%HERE%..\..\..") do set "REPO=%%~fi"
if not defined CLIENT_BUILD set "CLIENT_BUILD=%REPO%\client\build\vs2022\Release"
if not defined JP983_D3DX_INCLUDE set "JP983_D3DX_INCLUDE=%DXSDK_DIR%Include"
if not defined JP983_D3DX_LIB set "JP983_D3DX_LIB=%DXSDK_DIR%Lib\x86"
where cl >nul 2>&1 && goto havecl
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`call "%VSWHERE%" -latest -find VC\Auxiliary\Build\vcvarsall.bat`) do set "VCVARS=%%i"
call "%VCVARS%" x86 >nul || exit /b 1
:havecl
set "C=%REPO%\client"
set "E=%C%\src\engine"
if not exist "%HERE%obj" mkdir "%HERE%obj"
if not exist "%HERE%bin" mkdir "%HERE%bin"
set FLAGS=/c /nologo /W1 /O2 /Ob1 /Oy- /Gr /GS /EHsc /MD /Zm200 /Zc:wchar_t /permissive /Zc:strictStrings- /arch:IA32 /FI jp983_msvc_novo.h /D NDEBUG /D WIN32 /D _WINDOWS /D _MBCS /D REBANG_SEMANTIC_ONLY /D JP983_POSTEFFECT /D JP983_SERVER_CONFIG /D _CRT_SECURE_NO_WARNINGS /I"%C%\tools\vs\compat_msvc" /I"%WindowsSdkDir%Include\%WindowsSDKVersion%shared" /I"%VCToolsInstallDir%atlmfc\include" /I"%C%\include" /I"%C%\include\engine" /I"%E%\lib\freetype-2.2.1\include" /I"%C%\src\fresh\include" /I"%C%\src\projectg" /I"%C%\src\shared\thirties" /I"%C%\src\fresh\third_party\libpng-1.2.8" /I"%C%\src\fresh\third_party\zlib-1.2.2" /I"%JP983_D3DX_INCLUDE%" /Fo"%HERE%obj\\"
set SRC="%HERE%pethost.cpp" "%HERE%stubs.cpp" "%C%\src\shared\tea.cpp"
for %%f in (baseobject bitmap wbone wbone_physics wboneset wdevmng wdxtc westpak wlock wmath wmathutil wpuppet wresrcmng wview wvideo wxpuppet wxshadermtl wxtnlbuffer wvdshnames wblockmodel xunzip) do call set SRC=%%SRC%% "%E%\%%f.cpp"
cl %FLAGS% /MP %SRC% || exit /b 1
link /nologo /OUT:"%HERE%bin\pethost.exe" /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE "%HERE%obj\*.obj" "%CLIENT_BUILD%\terc.lib" "%CLIENT_BUILD%\ProjectG\implib\ijl15.lib" /FORCE:UNRESOLVED /LIBPATH:"%JP983_D3DX_LIB%" d3dx9.lib user32.lib gdi32.lib winmm.lib advapi32.lib ole32.lib %* || exit /b 1
