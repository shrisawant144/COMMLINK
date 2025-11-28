param(
    [string]$QtPath = "C:\Qt\5.15.2\msvc2019_64"
)

Write-Host "Setting up CommLink dependencies for Windows..." -ForegroundColor Green

# Check for CMake
if (!(Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Installing via winget..." -ForegroundColor Yellow
    winget install Kitware.CMake
} else {
    Write-Host "CMake found: $(cmake --version | Select-Object -First 1)" -ForegroundColor Green
}

# Check for Qt
if (!(Test-Path $QtPath)) {
    Write-Host "Qt not found at $QtPath" -ForegroundColor Red
    Write-Host "Please install Qt from: https://www.qt.io/download-qt-installer" -ForegroundColor Yellow
    Write-Host "Or specify Qt path: .\setup-windows.ps1 -QtPath 'C:\path\to\qt'" -ForegroundColor Yellow
    exit 1
}

Write-Host "Qt found at: $QtPath" -ForegroundColor Green

# Set environment variable
[Environment]::SetEnvironmentVariable("CMAKE_PREFIX_PATH", $QtPath, "User")
Write-Host "CMAKE_PREFIX_PATH set to: $QtPath" -ForegroundColor Green

Write-Host "`nSetup complete! Restart your terminal and run:" -ForegroundColor Green
Write-Host "  .\scripts\build.sh (if using Git Bash/WSL)" -ForegroundColor Cyan
Write-Host "  Or manually: mkdir build && cd build && cmake .. && cmake --build . --config Release" -ForegroundColor Cyan
