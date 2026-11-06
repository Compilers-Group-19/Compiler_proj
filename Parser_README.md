# Mini Compiler (C++)

Term project: a compiler for the class mini language, built in five phases.
Group repository: COMPILER_GROUP_ONE

| Phase | Deliverable | Due | Status |
|-------|-------------|-----|--------|
| 1 | Scanner | 25 Oct 2026 | Done |
| 2 | Parser | 8 Nov 2026 | In progress |
| 3 | Semantic analysis / IR | 22 Nov 2026 | Not started |
| 4 | Code generation | 6 Dec 2026 | Not started |
| 5 | Optimizer + final demo | 13 Dec 2026 | Not started |

## Build and run

Requires g++ with C++17 support. Two programs are built:
`scanner` (Phase 1: prints tokens) and `parser` (Phase 2: scanner + parser, prints the syntax tree).

Windows (PowerShell), from the project root:
```
.\build.ps1
.\scanner.exe tests\sample.mini      tokens
.\parser.exe tests\sample.mini       syntax tree
.\tests\run_tests.ps1                all tests
```
If PowerShell blocks scripts: `powershell -ExecutionPolicy Bypass -File .\build.ps1`

Linux / macOS / MSYS2:
```
make
./scanner tests/sample.mini
./parser tests/sample.mini
make test test-parser
```

Running either program with no argument asks for a file path.
Exit code: 0 = success, 1 = lexical or syntax errors, 2 = file or usage error.

> **Phase 2 is being added in ten parts.** Until all ten are in the repository, `parser` reports
> "Parser is not finished yet" instead of printing a tree. The scanner is not affected.

## Phase 1: Scanner

Files: `src/token.h` (token types), `src/scanner.h` and the scanner source files `src/scanner*.cpp`, `src/main.cpp` (driver that prints tokens). Details: `docs/SCANNER.md`.

Each token records its type, lexeme, line and column.

**Token classes**
- Keywords: `int float bool string void if else while for return func print read true false`
- Identifiers: letter or `_`, followed by letters, digits or `_`
- Literals: integers (`42`), floats (`3.14`), strings (`"text"` with escapes `\n \t \r \0 \\ \"`)
- Operators: `+ - * / %`, `= == != < <= > >=`, `&& || !`
- Delimiters: `( ) { } [ ] , ; : .`
- Comments (skipped): `// line` and `/* block */`

**Error handling.** Lexical errors become `ERROR` tokens with a message and position, and scanning continues so several errors can be reported in one run.

**Adapting to the class grammar.** Edit the keyword table in the scanner source and the token list in `src/token.h`.

## Phase 2: Parser

| File | Contents |
|------|----------|
| `src/ast.h` | AST node types |
| `src/ast_printer.h`, `src/ast_printer.cpp` | prints the tree as an indented outline |
| `src/parser.h` | `Parser` class and the full grammar (EBNF) |
| `src/parser_util.h` | small helpers shared by the parser files |
| `src/parser_core.cpp` | token helpers, error messages, error recovery |
| `src/parser_primary.cpp` | unary operators, calls, indexing, literals, parentheses |
| `src/parser_expressions.cpp` | assignment and the binary operators (by precedence) |
| `src/parser_statements.cpp` | statement dispatcher, variable declarations, blocks, expression statements |
| `src/parser_control.cpp` | `if`, `while`, `for`, `return`, `print`, `read` |
| `src/parser_program.cpp` | the whole program and function declarations |
| `src/main_parser.cpp` | driver for the `parser` program |

The parser takes the `std::vector<Token>` from `Scanner::scanTokens()` and returns a `std::unique_ptr<Program>`:

```cpp
Scanner scanner(source);
Parser parser(scanner.scanTokens());
auto program = parser.parse();
if (parser.errorCount() > 0) { /* parser.errors() holds the messages */ }
```

Grammar summary (the full EBNF is in `src/parser.h` and `docs/PARSER.md`):
- Program = functions and top-level statements in any order
- Statements: variable declaration (optionally an array `int a[5];`), block, `if/else`, `while`, `for`, `return`, `print(a, b)`, `read(x)`, expression statement
- Expressions, lowest to highest precedence: `=` (right-assoc), `||`, `&&`, `== !=`, `< <= > >=`, `+ -`, `* / %`, unary `! -`, calls / indexing / literals / parentheses

**Error handling.** Lexical errors stop the pipeline before parsing (they are listed first). Syntax errors use panic-mode recovery (skip to the next `;` or statement keyword), so several errors are reported in one run. Messages include line, column and the offending token.

**AST.** Every node carries `line` and `column`. Expressions and statements each have a `kind` enum, so the semantic phase can `switch` on `kind` and `static_cast` to the concrete node (see `src/ast_printer.cpp` for an example traversal).

**Not used by the grammar yet:** the `:` and `.` tokens. Add rules in `Parser::primary()` / `Parser::statement()` if the class spec requires them.

## Tests

| File | Purpose |
|------|---------|
| `tests/sample.mini` | valid program covering every token class |
| `tests/errors.mini` | one example of each lexical error |
