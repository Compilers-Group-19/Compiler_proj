# Owner: PS/CSC/23/0215
# Builds the scanner and parser on Windows (PowerShell). Compiles every .cpp file in src\.
#
# Run from the project root:
#   .\build.ps1
# If PowerShell blocks the script, run:
#   powershell -ExecutionPolicy Bypass -File .\build.ps1

$src = Join-Path $PSScriptRoot "src"
$all = @(Get-ChildItem -Path $src -Filter *.cpp)

if ($all.Count -eq 0) {
    Write-Host "No .cpp files found in src\" -ForegroundColor Red
    exit 2
}

function IsParserFile($f) {
    return ($f.Name -eq 'main_parser.cpp') -or ($f.Name -eq 'ast_printer.cpp') -or ($f.Name -like 'parser*.cpp')
}

$scannerSources = @($all | Where-Object { -not (IsParserFile $_) } | ForEach-Object { $_.FullName })
$parserSources = @($all | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName })

function Build($output, $sources) {
    $exe = Join-Path $PSScriptRoot $output
    & g++ -std=c++17 -Wall -Wextra -O2 -static -o $exe $sources | Out-Host
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build FAILED: $output" -ForegroundColor Red
        return $false
    }
    Write-Host "Build OK: $output" -ForegroundColor Green
    return $true
}

$ok = Build "scanner.exe" $scannerSources
if (Test-Path (Join-Path $src "main_parser.cpp")) {
    $parserOk = Build "parser.exe" $parserSources
    $ok = $ok -and $parserOk
}

if (-not $ok) { exit 1 }
