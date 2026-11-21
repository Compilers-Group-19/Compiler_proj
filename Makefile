# owner: PS/CSC/23/0215
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -static

# scanner = main.cpp + the scanner files (everything except the parser files)
# parser  = everything except the scanner's main.cpp
ALL_SRC      := $(wildcard src/*.cpp)
HDR          := $(wildcard src/*.h)
PARSER_FILES := src/main_parser.cpp src/ast_printer.cpp $(wildcard src/parser*.cpp)
SCANNER_SRC  := $(filter-out $(PARSER_FILES),$(ALL_SRC))
PARSER_SRC   := $(filter-out src/main.cpp,$(ALL_SRC))

# The parser program is built once its driver (src/main_parser.cpp) exists.
TARGETS := scanner
ifneq ($(wildcard src/main_parser.cpp),)
TARGETS += parser
endif

# Phase 3 lives in subfolders (src/semantic, src/ir, src/driver), which the
# scanner and parser builds above never look at. The compiler program is
# built only once every Phase 3 file is in the repository, so the parts can
# be pushed in any order without breaking the build.
PHASE3_FILES := src/semantic/types.h \
                src/semantic/symbol_table.h \
                src/semantic/symbol_table.cpp \
                src/semantic/semantic.h \
                src/semantic/semantic_core.cpp \
                src/semantic/semantic_decls.cpp \
                src/semantic/semantic_stmts.cpp \
                src/semantic/semantic_exprs.cpp \
                src/ir/ir.h \
                src/ir/ir_printer.cpp \
                src/ir/ir_gen.h \
                src/ir/ir_gen_stmts.cpp \
                src/ir/ir_gen_exprs.cpp \
                src/ir/ir_interpreter.h \
                src/ir/ir_interpreter.cpp \
                src/driver/main_compiler.cpp
PHASE3_SRC   := $(wildcard src/semantic/*.cpp src/ir/*.cpp src/driver/*.cpp)
PHASE3_HDR   := $(wildcard src/semantic/*.h src/ir/*.h)
COMPILER_SRC := $(filter-out src/main.cpp src/main_parser.cpp,$(ALL_SRC)) $(PHASE3_SRC)
ifeq ($(words $(wildcard $(PHASE3_FILES))),$(words $(PHASE3_FILES)))
TARGETS += compiler
endif

all: $(TARGETS)

scanner: $(SCANNER_SRC) $(HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(SCANNER_SRC)

parser: $(PARSER_SRC) $(HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(PARSER_SRC)

compiler: $(COMPILER_SRC) $(HDR) $(PHASE3_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(COMPILER_SRC)

# Scanner tests (Phase 1). errors.mini is supposed to fail, hence the leading '-'.
test: scanner
	./scanner tests/sample.mini
	-./scanner tests/errors.mini

# Parser tests (Phase 2). parse_errors.mini is supposed to fail.
test-parser: parser
	./parser tests/sample.mini
	./parser tests/parse_ok.mini
	./parser tests/parse_edge_ok.mini
	-./parser tests/parse_errors.mini
	-./parser tests/parse_recovery.mini

# Phase 3 tests. sem_errors.mini is supposed to fail.
test-compiler: compiler
	./compiler tests/sample.mini
	./compiler --run tests/run_functions.mini
	-./compiler tests/sem_errors.mini

# Every check, with expected results (exit code = number of failures).
check: $(TARGETS)
	bash tests/run_tests.sh

clean:
	rm -f scanner parser compiler scanner.exe parser.exe compiler.exe

.PHONY: all test test-parser test-compiler check clean
