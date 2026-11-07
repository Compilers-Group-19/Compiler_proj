# owner: PS/CSC/23/0213
#!/usr/bin/env bash
# Runs the scanner and the parser on the test programs and checks the exit
# code and the summary line of each run (Linux, macOS, Git Bash, MSYS2).
# Build first with `make` (or .\build.ps1 on Windows), then run:
#   bash tests/run_tests.sh

cd "$(dirname "$0")/.." || exit 2

# On Windows (Git Bash) the programs are called scanner.exe / parser.exe
SCANNER=./scanner
PARSER=./parser
[ -f ./scanner.exe ] && SCANNER=./scanner.exe
[ -f ./parser.exe ] && PARSER=./parser.exe

if [ ! -f "$SCANNER" ] || [ ! -f "$PARSER" ]; then
    echo "scanner / parser not found. Build first (make)."
    exit 2
fi

failures=0

# check <program> <file> <expected exit code> <text that must appear>
check() {
    out=$("$1" "tests/$2" 2>&1)
    code=$?
    if [ "$code" -eq "$3" ] && printf '%s' "$out" | grep -qF -- "$4"; then
        echo "PASS  $(basename "$1") $2"
    else
        echo "FAIL  $(basename "$1") $2 (exit code $code, expected $3, looking for: $4)"
        failures=$((failures + 1))
    fi
}

check "$SCANNER" sample.mini       0 "82 tokens, 0 lexical error(s)"
check "$SCANNER" errors.mini       1 "25 tokens, 5 lexical error(s)"
check "$PARSER"  sample.mini       0 "RESULT: SUCCESS"
check "$PARSER"  parse_ok.mini     0 "RESULT: SUCCESS"
check "$PARSER"  parse_errors.mini 1 "9 syntax error(s)"
check "$PARSER"  errors.mini       1 "5 lexical error(s)"

if [ "$failures" -eq 0 ]; then
    echo "All tests passed."
else
    echo "$failures test(s) failed."
fi
exit "$failures"
