# OWNER: PS/CSC/23/0213
# Phase 2: Parser (syntax analysis)

The parser takes the token stream produced by the scanner (Phase 1) and checks
that it follows the grammar of the mini language. If it does, the parser builds
an **abstract syntax tree (AST)**: a tree that shows the structure of the
program. The semantic analyzer (Phase 3) will walk this tree.

```
source text --> Scanner --> tokens --> Parser --> AST
```

It is a hand-written **recursive-descent** parser: there is one function for
each grammar rule, and each function calls the functions of the rules it is
made of.

## Using the parser from code

```cpp
Scanner scanner(source);
Parser parser(scanner.scanTokens());
std::unique_ptr<Program> program = parser.parse();

if (parser.errorCount() > 0) {
    for (const std::string& message : parser.errors()) { /* report */ }
}
```

`parse()` always returns a `Program`; if there were syntax errors it may be
incomplete, so check `errorCount()` first. The scanner's `ERROR` tokens must be
dealt with before parsing (the `parser` program does this: it lists lexical
errors and stops).

## Grammar (EBNF)

```
program     -> ( funcDecl | statement )* EOF
funcDecl    -> "func" type IDENT "(" [ param { "," param } ] ")" block
param       -> varType IDENT
type        -> "int" | "float" | "bool" | "string" | "void"
varType     -> "int" | "float" | "bool" | "string"
statement   -> varDecl | block | ifStmt | whileStmt | forStmt | returnStmt
             | printStmt | readStmt | exprStmt
varDecl     -> varType IDENT [ "[" INT "]" ] [ "=" expression ] ";"
block       -> "{" statement* "}"
ifStmt      -> "if" "(" expression ")" statement [ "else" statement ]
whileStmt   -> "while" "(" expression ")" statement
forStmt     -> "for" "(" ( varDecl | exprStmt | ";" )
               [ expression ] ";" [ expression ] ")" statement
returnStmt  -> "return" [ expression ] ";"
printStmt   -> "print" "(" expression { "," expression } ")" ";"
readStmt    -> "read" "(" IDENT [ "[" expression "]" ] ")" ";"
exprStmt    -> expression ";"
expression  -> assignment
assignment  -> ( IDENT | IDENT "[" expression "]" ) "=" assignment | logicOr
logicOr     -> logicAnd { "||" logicAnd }
logicAnd    -> equality { "&&" equality }
equality    -> comparison { ( "==" | "!=" ) comparison }
comparison  -> term { ( "<" | "<=" | ">" | ">=" ) term }
term        -> factor { ( "+" | "-" ) factor }
factor      -> unary { ( "*" | "/" | "%" ) unary }
unary       -> ( "!" | "-" ) unary | primary
primary     -> INT | FLOAT | STRING | "true" | "false"
             | IDENT | IDENT "(" [ args ] ")" | IDENT "[" expression "]"
             | "(" expression ")"
args        -> expression { "," expression }
```

Notes on the grammar:
- Functions can only be declared at the top level, not inside blocks.
- `void` is only allowed as a function return type (not for variables or parameters).
- An array declaration (`int a[5];`) needs a positive integer size and cannot have an initializer.
- An `else` belongs to the nearest `if` before it.
- A bare `;` is not a statement; an empty `for` clause is written `for (;;)`.

### Operator precedence

From loosest to tightest. All binary operators are left-associative; `=` is right-associative (`a = b = 3` means `a = (b = 3)`).

| Level | Operators |
|-------|-----------|
| 1 (loosest) | `=` |
| 2 | `\|\|` |
| 3 | `&&` |
| 4 | `==` `!=` |
| 5 | `<` `<=` `>` `>=` |
| 6 | `+` `-` |
| 7 | `*` `/` `%` |
| 8 | unary `!` `-` |
| 9 (tightest) | calls `f(x)`, indexing `a[i]`, literals, names, parentheses |

The levels 2 to 7 are handled by one function, `Parser::binary(level)`, which
reads the table `levels()` in `src/parser_expressions.cpp`. To change the
precedence of an operator, change that table.

## The syntax tree

Defined in `src/ast.h`. Every node has `line` and `column` (the position of the
first token of the construct) so later phases can report errors at the right
place. Nodes are owned through `std::unique_ptr`, so the whole tree is freed
automatically.

Expressions (`Expr`, with `kind`):

| Node | Meaning | Fields |
|------|---------|--------|
| `IntLiteral`, `FloatLiteral`, `StringLiteral`, `BoolLiteral` | constants | `value` |
| `VariableExpr` | a variable | `name` |
| `IndexExpr` | `a[i]` | `name`, `index` |
| `UnaryExpr` | `-x`, `!x` | `op`, `operand` |
| `BinaryExpr` | `a + b`, `a && b`, ... | `op`, `left`, `right` |
| `AssignExpr` | `x = v`, `a[i] = v` | `target`, `value` |
| `CallExpr` | `f(a, b)` | `callee`, `args` |

Statements (`Stmt`, with `kind`):

| Node | Meaning | Fields |
|------|---------|--------|
| `VarDecl` | `int x = 5;` `int a[10];` | `type`, `name`, `arraySize` (-1 if not an array), `init` |
| `BlockStmt` | `{ ... }` | `statements` |
| `IfStmt` | `if` / `else` | `condition`, `thenBranch`, `elseBranch` (may be null) |
| `WhileStmt` | `while` | `condition`, `body` |
| `ForStmt` | `for` | `init`, `condition`, `update`, `body` (the first three may be null) |
| `ReturnStmt` | `return` | `value` (may be null) |
| `PrintStmt` | `print(a, b)` | `args` |
| `ReadStmt` | `read(x)` | `target` |
| `ExprStmt` | expression used as a statement | `expr` |
| `FuncDecl` | `func int f(int a) { }` | `returnType`, `name`, `params`, `body` |

`Program` is the root: `items` holds the functions and top-level statements in
source order. To walk the tree, `switch` on `kind` and `static_cast` to the
concrete node; `src/ast_printer.cpp` is a complete example.

Example: the program

```
int x = 1 + 2 * 3;
if (x > 5) print("big", x);
```

gives (`*` binds tighter than `+`):

```
Program
  VarDecl int x
    init: Binary +
      Int 1
      Binary *
        Int 2
        Int 3
  If
    condition: Binary >
      Var x
      Int 5
    then: Print
      String "big"
      Var x
```

## Error handling

1. **Lexical errors come first.** If the scanner reports any, they are listed
   and parsing is not attempted.
2. **Syntax errors do not stop the parser.** A rule that fails throws an error;
   `parse()` (or `block()` for errors inside a block) records it and calls
   `synchronize()`, which skips tokens until just after a `;` or just before a
   keyword that starts a statement (or a `}`). Parsing then continues, so one
   run reports several errors. Parsing always moves forward after an error, so
   it cannot loop forever.
3. After an error at the top level, a leftover `}` is dropped quietly instead of
   producing a second, confusing error.

Messages have the form
`Syntax error at line 4, col 11: Expected ')' after if condition, found '{'`.
Besides the usual "Expected X" messages there are specific ones:

| Message | Cause |
|---------|-------|
| `'void' can only be used as a function return type` | `void z;` |
| `Functions can only be declared at the top level` | `func` inside a block |
| `Invalid assignment target` | `3 = x;` |
| `Array size must be greater than 0` / `Array size is out of range` | `int a[0];` |
| `Array declarations cannot have an initializer` | `int a[3] = 1;` |
| `Integer literal is out of range` / `Float literal is out of range` | number too large |

## Running

```
.\parser.exe tests\sample.mini        (Windows)
./parser tests/sample.mini            (Linux / macOS)
```

On success it prints the tree and `RESULT: SUCCESS - program parsed with no
errors (exit code 0)`. Exit codes: 0 = success, 1 = lexical or syntax errors,
2 = file or usage error. With no argument the program asks for a file path.

## Tests

`tests/run_tests.ps1` (Windows) or `tests/run_tests.sh` (Linux/macOS/Git Bash)
run these checks after building:

| Program | File | Expected |
|---------|------|----------|
| scanner | `sample.mini` | 82 tokens, 0 lexical errors, exit 0 |
| scanner | `errors.mini` | 25 tokens, 5 lexical errors, exit 1 |
| parser | `sample.mini` | tree, `RESULT: SUCCESS`, exit 0 |
| parser | `parse_ok.mini` | every grammar rule parses, exit 0 |
| parser | `parse_errors.mini` | 9 syntax errors reported in one run, exit 1 |
| parser | `errors.mini` | 5 lexical errors listed, parsing not attempted, exit 1 |

## Source files

| File | Contents |
|------|----------|
| `src/ast.h` | AST node types |
| `src/ast_printer.h`, `src/ast_printer.cpp` | prints the tree as an indented outline |
| `src/parser.h` | `Parser` class and the grammar |
| `src/parser_util.h` | helpers shared by the parser files (`node<T>()` creates a node stamped with a token's position) |
| `src/parser_core.cpp` | `advance`, `match`, `consume`, `makeError`, `report`, `synchronize` |
| `src/parser_primary.cpp` | `unary`, `primary`, `argumentList` |
| `src/parser_expressions.cpp` | `expression`, `assignment`, `binary` and the precedence table |
| `src/parser_statements.cpp` | `statement`, `varDecl`, `block`, `exprStmt` |
| `src/parser_control.cpp` | `ifStmt`, `whileStmt`, `forStmt`, `returnStmt`, `printStmt`, `readStmt` |
| `src/parser_program.cpp` | `parse`, `declaration`, `funcDecl` |
| `src/main_parser.cpp` | driver for the `parser` program |

## Changing the language

- **New statement:** add a node to `src/ast.h`, a `case` in `Parser::statement()`, a function in `src/parser_control.cpp` (declare it in `src/parser.h`), and a `case` in `src/ast_printer.cpp`.
- **New operator:** add the token in `src/token.h` and the scanner, then add it to the right row of `levels()` and to `opSymbol()` in `src/ast.h`.
- **Unused tokens:** `:` and `.` are produced by the scanner but not used by any rule. Add rules in `Parser::primary()` or `Parser::statement()` if the class specification needs them.

## Not checked here

The parser only checks structure. It does not check that variables are declared,
that types match, or that `return` agrees with the function's type; that is the
job of Phase 3 (semantic analysis).
