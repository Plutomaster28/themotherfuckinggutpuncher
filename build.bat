@echo off
REM GutPuncher Build Script
REM Builds static executable with UCRT64

echo ========================================
echo   GutPuncher Build Script
echo ========================================
echo.

REM Check if gcc is available
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GCC not found in PATH!
    echo Please install MinGW-w64 with UCRT64 and add to PATH.
    echo.
    echo Download from: https://www.mingw-w64.org/
    echo.
    pause
    exit /b 1
)

echo [INFO] Compiler found: gcc
gcc --version | findstr gcc
echo.

REM Build the executable
echo [BUILD] Compiling gutpuncher.c...
gcc -O2 -Wall -Wextra -o gutpuncher.exe gutpuncher.c -static -static-libgcc -lpsapi -lshlwapi -ladvapi32 -lkernel32 -luser32

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build complete: gutpuncher.exe
echo.

REM Show file info
for %%A in (gutpuncher.exe) do (
    echo File size: %%~zA bytes
    set /a KB=%%~zA/1024
)
echo File size: %KB% KB
echo.

REM Strip symbols for smaller size
echo [STRIP] Removing debug symbols...
strip -s gutpuncher.exe 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Symbols stripped
    for %%A in (gutpuncher.exe) do (
        set /a KB=%%~zA/1024
    )
    echo New size: %KB% KB
) else (
    echo [WARN] Strip command not available or failed
)

echo.
echo ========================================
echo   Build Complete!
echo ========================================
echo.
echo To run: gutpuncher.exe (as Administrator)
echo.
pause
