@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "CSHELL_ACTION=%~1"
if not defined CSHELL_ACTION set "CSHELL_ACTION=check"
set "CSHELL_PRESET=%~2"
if not defined CSHELL_PRESET set "CSHELL_PRESET=debug"
if /i not "%CSHELL_PRESET%"=="debug" if /i not "%CSHELL_PRESET%"=="release" goto usage
if /i "%CSHELL_ACTION%"=="check" goto init
if /i "%CSHELL_ACTION%"=="configure" goto init
if /i "%CSHELL_ACTION%"=="build" goto init
if /i "%CSHELL_ACTION%"=="test" goto init
if /i "%CSHELL_ACTION%"=="code" goto init
goto usage

:init
set "CSHELL_LOCK=%~dp0..\config\toolchain.json"
set "CSHELL_MSVC="
set "CSHELL_SDK="
for /f "tokens=1,* delims==" %%I in ('powershell.exe -NoProfile -Command "$ErrorActionPreference='Stop'; $taskConfig=Get-Content -Raw -Encoding UTF8 -LiteralPath $env:CSHELL_LOCK | ConvertFrom-Json; if ($taskConfig.msvc -notmatch '^[0-9.]+$' -or $taskConfig.windowsSdk -notmatch '^[0-9.]+$') { throw 'Invalid toolchain versions' }; 'MSVC='+$taskConfig.msvc; 'SDK='+$taskConfig.windowsSdk"') do set "CSHELL_%%I=%%J"
if not defined CSHELL_MSVC goto badlock
if not defined CSHELL_SDK goto badlock
set "CSHELL_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%CSHELL_VSWHERE%" goto missingvs
if not defined CSHELL_VS_DIR for /f "usebackq delims=" %%I in (`"%CSHELL_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "CSHELL_VS_DIR=%%I"
if not defined CSHELL_VS_DIR goto missingvs
if not exist "%CSHELL_VS_DIR%\VC\Tools\MSVC\%CSHELL_MSVC%\bin\Hostx64\x64\cl.exe" goto missingtoolset
call "%CSHELL_VS_DIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -vcvars_ver=%CSHELL_MSVC% -winsdk=%CSHELL_SDK% >nul
if errorlevel 1 goto initfailed
pushd "%~dp0.."
if errorlevel 1 exit /b 1
where cmake.exe >nul 2>nul
if errorlevel 1 goto missingcmake
cmake.exe -P scripts/check-environment.cmake
if errorlevel 1 goto failed
if /i "%CSHELL_ACTION%"=="check" goto success
if /i "%CSHELL_ACTION%"=="code" goto open_code
if /i "%CSHELL_ACTION%"=="test" goto test
cmake.exe --preset %CSHELL_PRESET%
if errorlevel 1 goto failed
if /i "%CSHELL_ACTION%"=="configure" goto success
cmake.exe --build --preset %CSHELL_PRESET%
if errorlevel 1 goto failed
goto success

:test
cmake.exe --preset %CSHELL_PRESET%
if errorlevel 1 goto failed
cmake.exe --build --preset %CSHELL_PRESET%
if errorlevel 1 goto failed
ctest.exe --preset %CSHELL_PRESET%
if errorlevel 1 goto failed
goto success

:open_code
where code.cmd >nul 2>nul
if errorlevel 1 goto missingcode
call code.cmd --new-window .
if errorlevel 1 goto failed
goto success

:success
popd
exit /b 0
:failed
popd
exit /b 1
:missingcmake
echo [ERROR] CMake is missing. See docs/ENVIRONMENT.md. 1>&2
goto failed
:missingcode
echo [ERROR] VS Code command is missing from PATH. See docs/ENVIRONMENT.md. 1>&2
goto failed
:badlock
echo [ERROR] Cannot read config/toolchain.json. 1>&2
exit /b 1
:missingvs
echo [ERROR] Install MSVC Build Tools and Windows SDK. See docs/ENVIRONMENT.md. 1>&2
exit /b 1
:missingtoolset
echo [ERROR] Required MSVC %CSHELL_MSVC% not found in "%CSHELL_VS_DIR%". 1>&2
echo Set CSHELL_VS_DIR to another installation or install the locked toolset. 1>&2
exit /b 1
:initfailed
echo [ERROR] MSVC/SDK initialization failed. See docs/ENVIRONMENT.md. 1>&2
exit /b 1
:usage
echo Usage: scripts\dev.cmd [check^|configure^|build^|test^|code] [debug^|release] 1>&2
exit /b 2
