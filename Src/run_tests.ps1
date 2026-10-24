# Owner: Member 10 - Tests
# Runs the scanner on the test programs and checks exit code and summary line.
# Build first with .\build.ps1, then run:
#   .\tests\run_tests.ps1

$root = Split-Path $PSScriptRoot -Parent
$exe  = Join-Path $root "scanner.exe"

if (-not (Test-Path $exe)) {
    Write-Host "scanner.exe not found. Run .\build.ps1 first." -ForegroundColor Red
    exit 2
}

$script:failures = 0

function Check($file, $expectedCode, $expectedText) {
    $path = Join-Path $PSScriptRoot $file
    $out  = & $exe $path | Out-String
    $code = $LASTEXITCODE
    if ($code -eq $expectedCode -and $out.Contains($expectedText)) {
        Write-Host "PASS  $file" -ForegroundColor Green
    } else {
        Write-Host "FAIL  $file (exit code $code, expected $expectedCode)" -ForegroundColor Red
        $script:failures++
    }
}

# file               exit code   text that must appear in the output
Check "sample.mini"  0           "82 tokens, 0 lexical error(s)"
Check "errors.mini"  1           "25 tokens, 5 lexical error(s)"

if ($script:failures -eq 0) {
    Write-Host "All tests passed." -ForegroundColor Green
} else {
    Write-Host "$($script:failures) test(s) failed." -ForegroundColor Red
}
exit $script:failures
