#include <fstream>
#include <iostream>
#include <sstream>
#include "ast_printer.h"
#include "parser.h"
#include "scanner.h"

// Same convention as the scanner driver: when started without a file argument
// (e.g. double-clicked), keep the window open so the result can be read.
static void waitIfInteractive(bool interactive) {
    if (!interactive) return;
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
}

int main(int argc, char* argv[]) {
    std::string path;
    bool interactive = false;

    if (argc >= 2) {
        path = argv[1];
    } else {
        interactive = true;
        std::cout << "Mini language parser (Phase 2)\n";
        std::cout << "Enter path to a source file (e.g. tests\\sample.mini): ";
        std::getline(std::cin, path);
        if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
            path = path.substr(1, path.size() - 2);
        }
        std::cout << "\n";
    }

    std::ifstream in(path, std::ios::binary);
    if (path.empty() || !in) {
        std::cerr << "Error: cannot open file '" << path << "'\n";
        std::cerr << "Usage: parser <source-file>\n";
        waitIfInteractive(interactive);
        return 2;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();

    // Phase 1: scan
    Scanner scanner(buffer.str());
    std::vector<Token> tokens = scanner.scanTokens();

    if (scanner.errorCount() > 0) {
        for (const Token& t : tokens) {
            if (t.type == TokenType::ERROR)
                std::cout << "Lexical error at line " << t.line << ", col " << t.column << ": "
                          << t.lexeme << "\n";
        }
        std::cout << "\n" << scanner.errorCount() << " lexical error(s)\n";
        std::cout << "RESULT: FAILED - fix lexical errors before parsing (exit code 1)\n";
        waitIfInteractive(interactive);
        return 1;
    }

    // Phase 2: parse
    Parser parser(std::move(tokens));
    std::unique_ptr<Program> program = parser.parse();

    if (parser.errorCount() > 0) {
        for (const std::string& e : parser.errors()) std::cout << e << "\n";
        std::cout << "\n" << parser.errorCount() << " syntax error(s)\n";
        std::cout << "RESULT: FAILED - syntax errors found (exit code 1)\n";
        waitIfInteractive(interactive);
        return 1;
    }

    std::cout << "Abstract Syntax Tree\n" << std::string(50, '-') << "\n";
    printAst(*program, std::cout);
    std::cout << std::string(50, '-') << "\n";
    std::cout << "RESULT: SUCCESS - program parsed with no errors (exit code 0)\n";
    waitIfInteractive(interactive);
    return 0;
}
