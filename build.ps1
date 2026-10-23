# Owner: PS/CSC/23/0215
# Builds the scanner on Windows (PowerShell). Compiles every .cpp file in src\.
#
# Run from the project root:
#   .\build.ps1
# If PowerShell blocks the script, run:
#   powershell -ExecutionPolicy Bypass -File .\build.ps1

$sources = Get-ChildItem -Path "$PSScriptRoot\src" -Filter *.cpp | ForEach-Object { $_.FullName }

if (-not $sources) {
    Write-Host "No .cpp files found in src\" -ForegroundColor Red
    exit 2
}

g++ -std=c++17 -Wall -Wextra -O2 -static -o "$PSScriptRoot\scanner.exe" $sources

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build OK: scanner.exe" -ForegroundColor Green
} else {
    Write-Host "Build FAILED" -ForegroundColor Red
    exit 1
}
