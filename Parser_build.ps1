# Builds the project on Windows (PowerShell).
#
#   scanner.exe  - Phase 1: prints the token stream
#   parser.exe   - Phase 2: scanner + parser, prints the syntax tree
#                  (built automatically once src\main_parser.cpp exists)
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

# Files that belong to the parser program only.
function IsParserFile($f) {
    return ($f.Name -eq 'main_parser.cpp') -or ($f.Name -eq 'ast_printer.cpp') -or ($f.Name -like 'parser*.cpp')
}

# scanner.exe = main.cpp + the scanner files (everything except the parser files)
$scannerSources = @($all | Where-Object { -not (IsParserFile $_) } | ForEach-Object { $_.FullName })
# parser.exe = everything except the scanner's main.cpp
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
