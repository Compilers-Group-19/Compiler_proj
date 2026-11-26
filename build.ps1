# Owner: PS/CSC/23/0215
# Builds the scanner, parser and compiler on Windows (PowerShell). Compiles every .cpp file in src\.
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

$phase3Files = @(
    "src\semantic\types.h",
    "src\semantic\symbol_table.h",
    "src\semantic\symbol_table.cpp",
    "src\semantic\semantic.h",
    "src\semantic\semantic_core.cpp",
    "src\semantic\semantic_decls.cpp",
    "src\semantic\semantic_stmts.cpp",
    "src\semantic\semantic_exprs.cpp",
    "src\ir\ir.h",
    "src\ir\ir_printer.cpp",
    "src\ir\ir_gen.h",
    "src\ir\ir_gen_stmts.cpp",
    "src\ir\ir_gen_exprs.cpp",
    "src\ir\ir_interpreter.h",
    "src\ir\ir_interpreter.cpp",
    "src\driver\main_compiler.cpp"
)
$missing = @($phase3Files | Where-Object { -not (Test-Path (Join-Path $PSScriptRoot $_)) })
$compilerExe = Join-Path $PSScriptRoot "compiler.exe"

if ($missing.Count -eq 0) {
    $phase3Sources = @(Get-ChildItem -Path (Join-Path $src "semantic"), (Join-Path $src "ir"), (Join-Path $src "driver") -Filter *.cpp | ForEach-Object { $_.FullName })
    $compilerSources = @($all | Where-Object { $_.Name -ne 'main.cpp' -and $_.Name -ne 'main_parser.cpp' } | ForEach-Object { $_.FullName }) + $phase3Sources
    $compilerOk = Build "compiler.exe" $compilerSources
    $ok = $ok -and $compilerOk
} else {
    if (Test-Path $compilerExe) { Remove-Item $compilerExe }  # never test an old build
    Write-Host "compiler.exe not built yet: $($missing.Count) Phase 3 file(s) still missing:" -ForegroundColor Yellow
    $missing | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
}

if (-not $ok) { exit 1 }
