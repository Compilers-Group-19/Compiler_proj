# Owner: PS/CSC/23/0213
# Phase 1: Scanner (lexical analyzer)

The scanner reads the source text of a mini-language program and turns it into
a sequence of tokens. It is the first phase of the compiler; the parser
(Phase 2) will consume its output.

## Output

Each token records:

| Field | Meaning |
|-------|---------|
| type | kind of token (`TokenType` in `src/token.h`) |
| lexeme | the text of the token (for strings: the value without quotes, escapes resolved; for errors: the message) |
| line, column | position of the first character (both start at 1) |

The token list always ends with an `END_OF_FILE` token.

## Token classes

| Class | Tokens |
|-------|--------|
| Keywords | `int float bool string void if else while for return func print read true false` |
| Identifiers | letter or `_`, then letters, digits or `_` |
| Integer literals | `42` |
| Float literals | `3.14` (a `.` must be followed by a digit) |
| String literals | `"text"` with escapes `\n \t \r \0 \\ \"` |
| Arithmetic | `+ - * / %` |
| Assignment and comparison | `= == != < <= > >=` |
| Logical | `&& \|\| !` |
| Delimiters | `( ) { } [ ] , ; : .` |
| Comments (skipped) | `// to end of line` and `/* block */` |

## Design

The scanner is a hand-written, single-pass scanner. It looks at one character
at a time, using one or two characters of lookahead (`peek`) to choose between
tokens such as `=` and `==`.

Source files:

| File | Responsibility |
|------|----------------|
| `src/token.h` | token types, `Token` struct, `tokenTypeName()` |
| `src/scanner.h` | `Scanner` class declaration |
| `src/scanner_core.cpp` | constructor, `peek`/`advance`/`match`, `addToken`, main `scanTokens()` loop |
| `src/scanner_comments.cpp` | skipping whitespace and comments |
| `src/scanner_identifiers.cpp` | keyword table and identifier scanning |
| `src/scanner_numbers.cpp` | integer and float literals |
| `src/scanner_strings.cpp` | string literals and escapes |
| `src/scanner_operators.cpp` | `scanToken()`: dispatch on the first character, operators, delimiters |
| `src/scanner_errors.cpp` | recording lexical errors |
| `src/main.cpp` | driver: reads a file, prints tokens and a result line |

Scanning loop: skip whitespace and comments, then call `scanToken()`, which
reads one token. Repeat until the end of input, then add `END_OF_FILE`.

## Error handling

Lexical errors do not stop the scanner. Each one becomes an `ERROR` token with
a message and position, and scanning continues, so several errors can be
reported in one run.

| Error | Example |
|-------|---------|
| Unexpected character | `@` |
| Lone `&` or `\|` | `x & y` |
| Unterminated string | `"hello` (newline or end of file before the closing quote) |
| Unterminated block comment | `/* never closed` |
| Invalid escape sequence | `"a\q"` |
| Malformed number | `12abc` |

## Running

```
.\scanner.exe tests\sample.mini
```

Exit code: 0 = no lexical errors, 1 = lexical errors found, 2 = file or usage
error.

## Tests

| File | Purpose | Expected |
|------|---------|----------|
| `tests/sample.mini` | valid program using every token class | 82 tokens, 0 errors, exit 0 |
| `tests/errors.mini` | one example of each lexical error | 25 tokens, 5 errors, exit 1 |

`tests/run_tests.ps1` checks both automatically.

## Changing the language

To match the class mini language specification:
- keywords: edit the table in `src/scanner_identifiers.cpp` and the `KW_` entries in `src/token.h`
- new operators or symbols: add a `TokenType` in `src/token.h` and a `case` in `src/scanner_operators.cpp`
- literal forms: edit `src/scanner_numbers.cpp` or `src/scanner_strings.cpp`
