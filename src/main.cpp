// Owner: PS/CSC/23/0209

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "scanner.h"

static std::string escapeForDisplay(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            case '\0': out += "\\0"; break;
            default: out += c;
        }
    }
    return out;
}

// When the program was started without a file argument (e.g. double-clicked),
// keep the window open so the result can be read.
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
        std::cout << "Mini language scanner (Phase 1)\n";
        std::cout << "Enter path to a source file (e.g. tests\\sample.mini): ";
        std::getline(std::cin, path);
        // Allow paths pasted with surrounding quotes
        if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
            path = path.substr(1, path.size() - 2);
        }
        std::cout << "\n";
    }

    std::ifstream in(path, std::ios::binary);
    if (path.empty() || !in) {
        std::cerr << "Error: cannot open file '" << path << "'\n";
        std::cerr << "Usage: scanner <source-file>\n";
        waitIfInteractive(interactive);
        return 2;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();

    Scanner scanner(buffer.str());
    std::vector<Token> tokens = scanner.scanTokens();

    std::cout << std::left << std::setw(10) << "LINE:COL" << std::setw(18) << "TOKEN"
              << "LEXEME\n";
    std::cout << std::string(50, '-') << "\n";

    for (const Token& t : tokens) {
        std::ostringstream pos;
        pos << t.line << ":" << t.column;
        std::cout << std::left << std::setw(10) << pos.str() << std::setw(18) << tokenTypeName(t.type)
                  << escapeForDisplay(t.lexeme) << "\n";
    }

    std::cout << std::string(50, '-') << "\n";
    std::cout << tokens.size() << " tokens, " << scanner.errorCount() << " lexical error(s)\n";

    int code = scanner.errorCount() > 0 ? 1 : 0;
    if (code == 0) {
        std::cout << "RESULT: SUCCESS (exit code 0)\n";
    } else {
        std::cout << "RESULT: FAILED - lexical errors found (exit code 1)\n";
    }
    waitIfInteractive(interactive);
    return code;
}
