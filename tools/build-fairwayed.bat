@echo off
rem Compila tools\fairwayed sem alterar o build.bat do autor.
rem O build.bat original quebra quando precisa do vswhere (parenteses de "Program Files (x86)" dentro do for /f);
rem aqui o VCVARS ja vai definido, e ele pula esse trecho. Argumento opcional: core (so o fairwayc.exe).
setlocal
if defined VCVARS goto run
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo vswhere nao encontrado. Instale o Visual Studio 2022 com C++ ou defina VCVARS.
  exit /b 1
)
"%VSWHERE%" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -latest -find VC\Auxiliary\Build\vcvarsall.bat > "%TEMP%\fairwayed_vc.txt"
set /p VCVARS=<"%TEMP%\fairwayed_vc.txt"
del "%TEMP%\fairwayed_vc.txt"
:run
if not defined VCVARS (
  echo Visual C++ x86 nao encontrado. Defina VCVARS com o caminho do vcvarsall.bat.
  exit /b 1
)
call "%~dp0fairwayed\build.bat" %*
