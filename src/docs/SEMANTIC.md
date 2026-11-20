<!-- Owner: PS/CSC/23/0207  (Phase 3, Part 2) -->
# Phase 3a: Semantic analysis

The parser (Phase 2) only checks that a program has the right *shape*. The
semantic analyzer checks that it makes *sense*: names are declared, types
fit together, functions are called correctly, and so on. If it finds no
errors, the intermediate code generator (Phase 3b, `docs/IR.md`) runs.

```
source --> Scanner --> tokens --> Parser --> AST --> SemanticAnalyzer --> IrGenerator --> IR
```

## Using it from code

```cpp
SemanticAnalyzer analyzer;
if (!analyzer.analyze(*program)) {
    for (const std::string& message : analyzer.errors()) { /* report */ }
}
IrGenerator generator(analyzer.info());   // types and symbols found by the analysis
```

`analyzer.info()` (a `SemanticInfo`) holds the type of every expression and
the symbol every name refers to. The IR generator uses it so it never has to
look names up or work out types again.

## How it works

1. **Collect functions.** Every function's name, return type and parameter
   types are put in the global scope first. A function can therefore be
   called before the line where it is written, and recursion works.
2. **Check in source order.** Top-level statements and function bodies are
   checked from top to bottom. A global variable is visible from its
   declaration onwards (also inside functions written after it).
3. **Report.** Errors are sorted by position. Analysis never stops at the
   first error. An expression that already caused an error gets the type
   `<error>`, which fits everywhere, so one mistake is reported once.

### Scopes (`src/semantic/symbol_table.*`)

| Scope | Opened by |
|-------|-----------|
| global | the program (functions and top-level variables) |
| function | a function: its parameters *and* the outermost `{ }` of its body |
| block | every other `{ }` |
| for | `for (...)`: the loop variable belongs to the loop only |

A name can be declared once per scope. An inner scope may reuse (shadow) a
name from an outer one. Each variable gets a unique name for the IR:
the second `x` in one function becomes `x.1`, the third `x.2`, and so on.

## The rules

### Names
- A variable must be declared before it is used: `int x = x;` is an error.
- No two declarations of the same name in one scope (variables, parameters, functions).
- A function name can not be used as a variable, and a variable can not be called.

### Types

| Types | int, float, bool, string, and 1-D arrays of them (`int a[10]`) |
|-------|-----|

A value can be stored where another type is expected (initializer,
assignment, argument, return) only if the types are the same, or it is an
**int going into a float** (widening). float to int is an error, since the
fraction would be lost.

| Operator | Operands | Result |
|----------|----------|--------|
| `+ - * /` | two numbers | float if either is float, else int (`7 / 2` is `3`) |
| `%` | two ints | int |
| `< <= > >=` | two numbers | bool |
| `== !=` | two numbers, two bools or two strings | bool |
| `&& \|\| !` | bools | bool |
| unary `-` | a number | the same type |

Strings can only be compared with `==`/`!=` and printed: there is no `+` for strings.

### Arrays
- Only one element at a time can be used: `a[i]`. A whole array can not be
  assigned, printed, read, passed or used in an expression.
- The index must be an int. A constant index outside `0 .. size-1` is an
  error at compile time. Other indexes are checked when the program runs.

### Statements
- `if`, `while` and `for` conditions must be bool. There is no "0 means false".
- A declaration can not be the whole body of `if`/`else`/`while`/`for`
  (`if (x) int y = 1;`). Use `{ }`.
- `return` only inside a function. A void function returns nothing, any
  other function must return a value of its type.
- A non-void function must return on every path. `if` without `else` does
  not count, but `while (true)` and `for (;;)` do, since they can only be
  left by a `return`.
- A call to a void function can be a statement on its own, but its "value"
  can not be used: `int x = greet();` is an error.
- Division by a constant zero (`x / 0`, `x % 0`) is an error.

## Error messages

Format: `Semantic error at line L, col C: message`

| Example | Message |
|---------|---------|
| `int a; int a;` | `'a' is already declared in this scope (line 1)` |
| `print(b);` | `'b' is not declared` |
| `int c = 2.5;` | `can not initialize an int variable 'c' with a float value` |
| `"x" + "y"` | `operator '+' needs two numbers (int or float), found string and string` |
| `5 % 2.0` | `operator '%' needs two ints, found int and float` |
| `if (a)` with int a | `the condition of 'if' must be bool, found int` |
| `arr = 5;` | `can not assign to the whole array 'arr'; ...` |
| `arr[1.5]` | `an array index must be int, found float` |
| `arr[3]` with `int arr[3]` | `index 3 is out of range for 'arr' (valid: 0 to 2)` |
| `a[0]` with int a | `'a' is not an array, so it can not be indexed` |
| `print(arr);` | `array 'arr' can not be used as a value; use one element, e.g. arr[0]` |
| `twice(1, 2)` | `function 'twice' takes 1 argument(s) but 2 were given` |
| `twice("no")` | `argument 1 of 'twice' must be int, found string` |
| `int h = hello();` | `function 'hello' is void and has no value to use` |
| `a = twice;` | `'twice' is a function; call it with twice(...)` |
| `a();` | `'a' is a variable, not a function` |
| `nope();` | `function 'nope' is not declared` |
| `return 1;` at top level | `'return' can only be used inside a function` |
| missing return | `function 'f' must return an int value on every path (add a return at the end)` |
| `return 5;` in void function | `function 'v' is void and can not return a value` |
| `return;` in int function | `function 'r' must return an int value` |
| `return true;` in float function | `function 'g' returns float, but this value is bool` |
| two functions `f` | `function 'f' is already declared (first at line 3)` |
| `func int f(int p, int p)` | `parameter 'p' is declared twice in function 'f'` |
| `if (x) int y = 1;` | `a declaration can not be the body of 'if'; put it inside { }` |
| `10 / 0` | `division by zero` |
| `read(arr);` | `can not read into the whole array; read into one element, e.g. read(a[i])` |

`tests/sem_errors.mini` has one example of every error (35 in one run).

## Source files (`src/semantic/`)

| File | Part | Contents |
|------|------|----------|
| `types.h` | 1 | `Type` and the `assignable()` rule |
| `symbol_table.h`, `symbol_table.cpp` | 1 | `Symbol`, scopes, unique IR names |
| `semantic.h` | 2 | `SemanticAnalyzer`, `SemanticInfo` |
| `semantic_core.cpp` | 2 | `analyze()`, collecting functions, error messages |
| `semantic_decls.cpp` | 3 | variable and function declarations, the "every path returns" check |
| `semantic_stmts.cpp` | 4 | statements |
| `semantic_exprs.cpp` | 5 | expressions and operator rules |
