#OWNER: PS/CSC/23/02132
# Runs the scanner and the parser on the test programs and checks the exit
# code and the summary line of each run.
# Build first with .\build.ps1, then run:
#   .\tests\run_tests.ps1

$root    = Split-Path $PSScriptRoot -Parent
$scanner = Join-Path $root "scanner.exe"
$parser  = Join-Path $root "parser.exe"

if (-not (Test-Path $scanner) -or -not (Test-Path $parser)) {
    Write-Host "scanner.exe / parser.exe not found. Run .\build.ps1 first." -ForegroundColor Red
    exit 2
}

$script:failures = 0

function Check($exe, $file, $expectedCode, $expectedText) {
    $path  = Join-Path $PSScriptRoot $file
    $out   = & $exe $path | Out-String
    $code  = $LASTEXITCODE
    $label = (Split-Path $exe -Leaf) + " " + $file
    if ($code -eq $expectedCode -and $out.Contains($expectedText)) {
        Write-Host "PASS  $label" -ForegroundColor Green
    } else {
        Write-Host "FAIL  $label (exit code $code, expected $expectedCode, looking for: $expectedText)" -ForegroundColor Red
        $script:failures++
    }
}

# program    file                exit code   text that must appear in the output
Check $scanner "sample.mini"       0         "82 tokens, 0 lexical error(s)"
Check $scanner "errors.mini"       1         "25 tokens, 5 lexical error(s)"
Check $parser  "sample.mini"       0         "RESULT: SUCCESS"
Check $parser  "parse_ok.mini"     0         "RESULT: SUCCESS"
Check $parser  "parse_errors.mini" 1         "9 syntax error(s)"
Check $parser  "errors.mini"       1         "5 lexical error(s)"

if ($script:failures -eq 0) {
    Write-Host "All tests passed." -ForegroundColor Green
} else {
    Write-Host "$($script:failures) test(s) failed." -ForegroundColor Red
}
exit $script:failures
