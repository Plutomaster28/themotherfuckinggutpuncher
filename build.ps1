# GutPuncher Build Script (PowerShell)
# Builds using CMake with Ninja generator

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  GutPuncher CMake/Ninja Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "[ERROR] CMake not found in PATH!" -ForegroundColor Red
    Write-Host "Please install CMake: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "[INFO] CMake found: $($cmake.Source)" -ForegroundColor Green
cmake --version | Select-Object -First 1
Write-Host ""

# Check for Ninja
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if (-not $ninja) {
    Write-Host "[WARN] Ninja not found in PATH, will use default generator" -ForegroundColor Yellow
    $generator = ""
} else {
    Write-Host "[INFO] Ninja found: $($ninja.Source)" -ForegroundColor Green
    $generator = "-G Ninja"
}
Write-Host ""

# Check for GCC
$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    Write-Host "[ERROR] GCC not found in PATH!" -ForegroundColor Red
    Write-Host "Please install MinGW-w64 with UCRT64 and add to PATH" -ForegroundColor Yellow
    Write-Host "Download from: https://www.mingw-w64.org/" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "[INFO] GCC found: $($gcc.Source)" -ForegroundColor Green
gcc --version | Select-Object -First 1
Write-Host ""

# Create build directory
$buildDir = "build"
if (Test-Path $buildDir) {
    Write-Host "[INFO] Cleaning old build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

New-Item -ItemType Directory -Path $buildDir | Out-Null
Write-Host "[INFO] Created build directory: $buildDir" -ForegroundColor Green
Write-Host ""

# Configure
Write-Host "[CMAKE] Configuring project..." -ForegroundColor Cyan
Set-Location $buildDir

$cmakeArgs = @(
    ".."
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_C_COMPILER=gcc"
)

if ($generator) {
    $cmakeArgs += $generator
}

& cmake $cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] CMake configuration failed!" -ForegroundColor Red
    Set-Location ..
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""

# Build
Write-Host "[BUILD] Compiling gutpuncher..." -ForegroundColor Cyan
& cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] Build failed!" -ForegroundColor Red
    Set-Location ..
    Read-Host "Press Enter to exit"
    exit 1
}

Set-Location ..

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Show file info
$exePath = "build\bin\gutpuncher.exe"
if (Test-Path $exePath) {
    $fileInfo = Get-Item $exePath
    $sizeKB = [math]::Round($fileInfo.Length / 1KB, 2)
    Write-Host "Executable: $exePath" -ForegroundColor White
    Write-Host "File Size: $sizeKB KB" -ForegroundColor White
} else {
    Write-Host "Executable: Not found in expected location" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "To run: .\build\bin\gutpuncher.exe (as Administrator)" -ForegroundColor Cyan
Write-Host ""
Read-Host "Press Enter to exit"
