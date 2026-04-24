@echo off
setlocal enabledelayedexpansion

rem ============================================================================================================================
rem Variables

set OUTPUT=
set SEARCH_DIRS=
set PACKAGE_COUNT=0
set LIBRARY_COUNT=0

rem ============================================================================================================================
rem Parse Args

:SECTION_ParseArgs

if "%~1"=="" goto SECTION_DoneParsing

if "%~1"=="-build_tool_path" (
    set OUTPUT=%~2
    shift
    shift
    goto SECTION_ParseArgs
)

if "%~1"=="-modules_search_dir" (
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
    echo ERROR: No output file specified with -build_tool_path. Use as: *.bat -build_tool_path ^<file^> ^(no extension needed^). Press any key to exit...
    pause >nul
    exit /b 1
)

if "%SEARCH_DIRS%"=="" (
    echo ERROR: No search directories specified with -modules_search_dir. Use as: *.bat -modules_search_dir ^<dir^> ^(can be specified multiple times^). Press any key to exit...
    pause >nul
    exit /b 1
)

rem Ensure output directory exists
for %%O in ("%OUTPUT%") do (
    if not exist "%%~dpO" mkdir "%%~dpO"
)

rem Clear output file(s)
> "%OUTPUT%.cpp" echo #define BRAHMA_EXEC
> "%OUTPUT%.libs.tmp" echo BRAHMA_BEGIN_LISTING_LIBRARIES^(^)
> "%OUTPUT%.pkgs.tmp" echo BRAHMA_BEGIN_LISTING_PACKAGES^(^)
> "%OUTPUT%.pkgCnt.tmp" echo.
> "%OUTPUT%.libCnt.tmp" echo.

echo #include "%OUTPUT:\=/%.pkgCnt.tmp" >> "%OUTPUT%.cpp"
echo #include "%OUTPUT:\=/%.libCnt.tmp" >> "%OUTPUT%.cpp"

for %%D in (%SEARCH_DIRS%) do (
    for /d %%M in ("%%~D\*") do (
        for %%F in ("%%~M\*._lib.h") do (
            if exist "%%~F" (
                set "FILE=%%~fF"
                set "FILE=!FILE:\=/!"
                echo #include "!FILE!" >> "%OUTPUT%.cpp"

                set "LIBRARY_NAME=%%~nF"
                set "LIBRARY_NAME=!LIBRARY_NAME:._lib=!"
                echo BRAHMA_ADD_LIBRARY^(!LIBRARY_COUNT!, "!FILE!", !LIBRARY_NAME!^) >> "%OUTPUT%.libs.tmp"

                set /a LIBRARY_COUNT+=1
            )
        )
    )

    for %%F in ("%%~D\*._pkg.h") do (
        if exist "%%~F" (
            set "FILE=%%~fF"
            set "FILE=!FILE:\=/!"
            echo #include "!FILE!" >> "%OUTPUT%.cpp"

            set "PACKAGE_NAME=%%~nF"
            set "PACKAGE_NAME=!PACKAGE_NAME:._pkg=!"
            echo BRAHMA_ADD_PACKAGE^(!PACKAGE_COUNT!, "!FILE!", !PACKAGE_NAME!^) >> "%OUTPUT%.pkgs.tmp"

            set /a PACKAGE_COUNT+=1
        )
    )
)

echo #define BRAHMA_PACKAGE_COUNT !PACKAGE_COUNT! >> "%OUTPUT%.pkgCnt.tmp"
echo #define BRAHMA_LIBRARY_COUNT !LIBRARY_COUNT! >> "%OUTPUT%.libCnt.tmp"
echo BRAHMA_END_LISTING_LIBRARIES^(^) >> "%OUTPUT%.libs.tmp"
echo BRAHMA_END_LISTING_PACKAGES^(^) >> "%OUTPUT%.pkgs.tmp"

echo #include "%OUTPUT:\=/%.libs.tmp" >> "%OUTPUT%.cpp"
echo #include "%OUTPUT:\=/%.pkgs.tmp" >> "%OUTPUT%.cpp"

echo Brahma build-file written to `%OUTPUT%.cpp`.

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

cl /nologo /Wall /WX /Zc:preprocessor /std:c++14 /O2 /MT /DNDEBUG /GS- /fp:fast "%OUTPUT%.cpp" "/Fe:%OUTPUT%.exe" "/Fo:%OUTPUT%.obj"
if %ERRORLEVEL% neq 0 (
    echo Brahma build-file failed to compile. Press any key to exit...
    pause >nul
    exit /b 1
)

echo Brahma build-file compiled to `%OUTPUT%.exe`.

rem ============================================================================================================================
rem Build

"%OUTPUT%.exe" %*
if %ERRORLEVEL% neq 0 (
    echo Brahma build-process failed. Press any key to exit...
    pause >nul
    exit /b 1
)
