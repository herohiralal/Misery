@echo off
setlocal enabledelayedexpansion

rem ============================================================================================================================
rem Variables

set OUTPUT=
set SEARCH_DIRS=
set SHOW_HELP=0

rem ============================================================================================================================
rem Parse Args

:SECTION_ParseArgs

if "%~1"=="" goto SECTION_DoneParsing

if /I "%~1"=="-H" (
    set SHOW_HELP=1
    goto SECTION_DoneParsing
)

if /I "%~1"=="-h" (
    set SHOW_HELP=1
    goto SECTION_DoneParsing
)

if "%~1"=="-O" (
    set OUTPUT=%~2
    shift
    shift
    goto SECTION_ParseArgs
)

if "%~1"=="-D" (
    set SEARCH_DIRS=!SEARCH_DIRS! "%~2"
    shift
    shift
    goto SECTION_ParseArgs
)

shift
goto SECTION_ParseArgs

rem ============================================================================================================================
rem Process Parsed Args

:SECTION_DoneParsing

if "%OUTPUT%"=="" (
    echo ERROR: No output file specified with -O. Press any key to exit...
    pause >nul
    exit /b 1
)

if "%SEARCH_DIRS%"=="" (
    echo ERROR: No search directories specified with -D. Press any key to exit...
    pause >nul
    exit /b 1
)

rem Ensure output directory exists
for %%O in ("%OUTPUT%") do (
    if not exist "%%~dpO" mkdir "%%~dpO"
)

rem Clear output file
> "%OUTPUT%.c" echo // Generated file; do not edit!
echo #define BRAHMA_EXEC 1 >> "%OUTPUT%.c"

for %%D in (%SEARCH_DIRS%) do (
    for /d %%M in ("%%~D\*") do (
        for %%F in ("%%~M\*.module.h") do (
            if exist "%%~F" (
                set "FILE=%%~fF"
                set "FILE=!FILE:\=/!"
                echo #include "!FILE!" >> "%OUTPUT%.c"
            )
        )
    )
)

echo Brahma build-file written to `%OUTPUT%.c`.

rem ============================================================================================================================
rem Build

where /Q cl.exe >nul 2>&1
if errorlevel 1 (
    set __VSCMD_ARG_NO_LOGO=1
    for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VS=%%i
    if "!VS!" neq "" (
        call "!VS!\Common7\Tools\vsdevcmd.bat" -arch=x64 -host_arch=x64
    )
)

cl /Brepro /nologo /Wall /WX /Zc:preprocessor /std:c11 /O2 /MT /DNDEBUG /GS- /fp:fast "%OUTPUT%.c" "/Fe:%OUTPUT%.exe" "/Fo:%OUTPUT%.obj"
if %ERRORLEVEL% neq 0 (
    echo Brahma build-file failed to compile. Press any key to exit...
    pause >nul
    exit /b 1
)

echo Brahma build-file compiled to `%OUTPUT%.exe`.

rem ============================================================================================================================
rem Build

"%OUTPUT%.exe"
if %ERRORLEVEL% neq 0 (
    echo Brahma build-process failed. Press any key to exit...
    pause >nul
    exit /b 1
)
