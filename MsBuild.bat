@echo off
setlocal enabledelayedexpansion

rem MsBuild.bat - Build a .slnx (solution filter) using MSBuild found via vswhere
rem Usage:
rem   MsBuild.bat [path\to\solution.slnx] [additional msbuild args]
rem Examples:
rem   MsBuild.bat .\Albeoris.AsiNetLoader.slnx /p:Configuration=Release

rem Capture remaining args for MSBuild
set "ARGS=%*"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo [ERROR] vswhere not found at "%VSWHERE%".
  echo Install Visual Studio 2017+ or VS Build Tools, which include vswhere.
  exit /b 2
)

for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set "VSINST=%%I"
if not defined VSINST (
  echo [ERROR] Visual Studio with MSBuild not found via vswhere.
  exit /b 3
)

set "MSBUILD=%VSINST%\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" set "MSBUILD=%VSINST%\MSBuild\15.0\Bin\MSBuild.exe"
if not exist "%MSBUILD%" (
  echo [ERROR] MSBuild.exe not found under "%VSINST%".
  exit /b 4
)

echo [INFO] Using MSBuild: "%MSBUILD%"

rem Build with sensible defaults; user-supplied args appended (and override if repeated)
rem By default, enable restore so RID-specific assets are present.
set "RESTORE_ARG=/restore"
echo %ARGS% | findstr /I /C:"/restore" >nul && set "RESTORE_ARG="
echo %ARGS% | findstr /I /C:"/t:Restore" >nul && set "RESTORE_ARG="
echo %ARGS% | findstr /I /C:"/target:Restore" >nul && set "RESTORE_ARG="

echo [INFO] Building filter with MSBuild...
"%MSBUILD%" %RESTORE_ARG% %ARGS%
set "EXITCODE=%ERRORLEVEL%"

if %EXITCODE% NEQ 0 (
  echo [ERROR] Build failed with exit code %EXITCODE%.
  exit /b %EXITCODE%
)

echo [INFO] Build succeeded.
exit /b 0
