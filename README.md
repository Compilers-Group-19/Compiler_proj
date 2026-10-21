# Mini Compiler (C++)

Term project: a compiler for the class mini language, built in five phases.
Group repository: COMPILER_GROUP_19

| Phase | Deliverable | Due | Status |
|-------|-------------|-----|--------|
| 1 | Scanner | 25 Oct 2026 | In progress |
| 2 | Parser | 8 Nov 2026 | Not started |
| 3 | Semantic analysis / IR | 22 Nov 2026 | Not started |
| 4 | Code generation | 6 Dec 2026 | Not started |
| 5 | Optimizer + final demo | 13 Dec 2026 | Not started |

## Repository layout

```
.gitignore
Makefile              build with make (Linux/macOS/MSYS2)
build.ps1             build on Windows PowerShell
README.md
docs/
  SCANNER.md          scanner design and token reference
src/
  token.h             token types
  scanner.h           Scanner class declaration
  scanner_core.cpp    constructor, character helpers, main scan loop
  scanner_comments.cpp     whitespace and comments
  scanner_identifiers.cpp  identifiers and keywords
  scanner_numbers.cpp      integer and float literals
  scanner_strings.cpp      string literals
  scanner_operators.cpp    operators and delimiters (token dispatch)
  scanner_errors.cpp       error tokens
  main.cpp                 driver: reads a file and prints the tokens
tests/
  sample.mini         valid program covering every token class
  errors.mini         one example of each lexical error
  run_tests.ps1       automated checks (Windows)
```

## Build

Requires g++ with C++17 support.

Windows (PowerShell), from the project root:
```
.\build.ps1
```
If PowerShell blocks scripts: `powershell -ExecutionPolicy Bypass -File .\build.ps1`

Or compile directly:
```
g++ -std=c++17 -Wall -Wextra -O2 -static -o scanner.exe src/main.cpp src/scanner_core.cpp src/scanner_comments.cpp src/scanner_identifiers.cpp src/scanner_numbers.cpp src/scanner_strings.cpp src/scanner_operators.cpp src/scanner_errors.cpp
```

Linux / macOS / MSYS2:
```
make
```

## Run

```
.\scanner.exe tests\sample.mini     (Windows)
./scanner tests/sample.mini         (Linux/macOS)
```

Running the program with no argument asks for a file path.
Exit code: 0 = no lexical errors, 1 = lexical errors found, 2 = file or usage error.

## Tests

```
.\tests\run_tests.ps1
```
Expected: both checks print PASS.

## Contributors

Each team member contributed one part of the scanner. See the commit history
for who wrote what.
