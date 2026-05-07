@echo off
REM Hunnu Language Installer - Windows
REM Usage: install.bat [PREFIX]
REM   PREFIX defaults to %ProgramFiles%\Hunnu, or %HUNNU_PREFIX% if set
REM
REM Requirements:
REM   - CMake (must be in PATH)
REM   - Visual Studio Build Tools or MinGW (gcc)

setlocal enabledelayedexpansion

REM Determine prefix
if not "%1"=="" (
    set "PREFIX=%1"
) else if not "%HUNNU_PREFIX%"=="" (
    set "PREFIX=%HUNNU_PREFIX%"
) else (
    set "PREFIX=%ProgramFiles%\Hunnu"
)

set "HUNNU_LIBDIR=%PREFIX%\lib\hunnu"
set "HUNNU_BINDIR=%PREFIX%\bin"
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"

echo.
echo   __ __ __  __  __  __  __  __  __
echo  ^|\  ^|\  ^|\  \^|\  \^|\  \^|\  \^|\  \^|\  \
echo  \ \ \ \ \ \  \ \  \ \  \ \  \ \  \ \  \
echo   \ \ \ \ \ \  \ \  \ \  \ \  \ \  \ \  \
echo    \ \_\ \_\ \__\ \__\ \__\ \__\ \__\ \__\
echo     \^|_\^|_\^|__\^|\__\^|\__\^|\__\^|\__\^|\__^|
echo          Hunnu Language Installer
echo.

echo Platform:  windows
echo Prefix:    !PREFIX!
echo Build dir: !BUILD_DIR!
echo.

REM Check for cmake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: CMake not found. Please install CMake and add it to PATH.
    exit /b 1
)

REM Detect compiler (MSVC or MinGW)
where cl.exe >nul 2>nul
if %errorlevel% equ 0 (
    set "GENERATOR="
    echo Using MSVC compiler
) else (
    where gcc.exe >nul 2>nul
    if %errorlevel% equ 0 (
        set "GENERATOR=-G ""MinGW Makefiles"""
        echo Using MinGW compiler
    ) else (
        where mingw32-make.exe >nul 2>nul
        if %errorlevel% equ 0 (
            set "GENERATOR=-G ""MinGW Makefiles"""
            echo Using MinGW compiler
        ) else (
            echo Error: No C compiler found. Install Visual Studio Build Tools or MinGW.
            exit /b 1
        )
    )
)

REM Step 1: Build
echo [1/3] Configuring and building hunnu...
if not exist "!BUILD_DIR!" mkdir "!BUILD_DIR!"
cd "!BUILD_DIR!"

if "!GENERATOR!"=="" (
    cmake "!PROJECT_DIR!" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="!PREFIX!"
) else (
    cmake "!PROJECT_DIR!" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="!PREFIX!" !GENERATOR!
)
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed.
    exit /b 1
)

REM Build
if "!GENERATOR!"=="" (
    cmake --build . --config Release
) else (
    mingw32-make -j%NUMBER_OF_PROCESSORS%
)
if %errorlevel% neq 0 (
    echo Error: Build failed.
    exit /b 1
)
echo.

REM Step 2: Install binary
echo [2/3] Installing binary to !HUNNU_BINDIR!...
if not exist "!HUNNU_BINDIR!" mkdir "!HUNNU_BINDIR!"

if exist "Release\hunnu.exe" (
    copy /Y "Release\hunnu.exe" "!HUNNU_BINDIR!\hunnu.exe" >nul
) else if exist "hunnu.exe" (
    copy /Y "hunnu.exe" "!HUNNU_BINDIR!\hunnu.exe" >nul
) else if exist "hunnu" (
    copy /Y "hunnu" "!HUNNU_BINDIR!\hunnu.exe" >nul
) else (
    echo Error: hunnu binary not found after build.
    dir /b *.exe *.out
    exit /b 1
)
echo   -^> !HUNNU_BINDIR!\hunnu.exe
echo.

REM Step 3: Install standard library
echo [3/3] Installing standard library to !HUNNU_LIBDIR!...
if not exist "!HUNNU_LIBDIR!\stdlib" mkdir "!HUNNU_LIBDIR!\stdlib"
xcopy /E /I /Y "!PROJECT_DIR!stdlib\*" "!HUNNU_LIBDIR!\stdlib\" >nul
echo   -^> !HUNNU_LIBDIR!\stdlib\
echo.

REM Verify
echo Verifying installation...
"!HUNNU_BINDIR!\hunnu.exe" --version
if %errorlevel% neq 0 (
    echo Warning: Binary installed but version check failed.
)

echo.
echo Installation complete!
echo   Binary: !HUNNU_BINDIR!\hunnu.exe
echo   Stdlib: !HUNNU_LIBDIR!\stdlib\
echo.
echo Add to your PATH (System Properties -^> Environment Variables):
echo   set HUNNU_STDLIB_PATH=!HUNNU_LIBDIR!
echo   set PATH=%%PATH%%;!HUNNU_BINDIR!
echo.
echo Quick test:
echo   hunnu --version
echo   hunnu run examples\main.hn
echo.

endlocal
