// Owner: PS/CSC/23/0209  (Phase 3, Part 9)
// Runs IR directly (`compiler --run file.mini`).
//
// This is how we test Phase 3: if the IR of a program prints the right
// output when it runs, the analysis and the IR generator are doing their
// job. Later phases can use it too: the optimizer (Phase 5) must not change
// what a program prints.

#ifndef IR_IR_INTERPRETER_H
#define IR_IR_INTERPRETER_H

#include <istream>
#include <ostream>
#include <string>
#include "ir.h"

class IrInterpreter {
public:
    IrInterpreter(const IrProgram& program, std::istream& in, std::ostream& out)
        : program_(program), in_(in), out_(out) {}

    // Runs the program. Returns false on a runtime error (see error()).
    bool run();

    // "Runtime error at line 7: array index 5 is out of range ..."
    const std::string& error() const { return error_; }

    // More nested calls than this is reported as a stack overflow.
    static constexpr size_t kMaxCallDepth = 100000;

private:
    const IrProgram& program_;
    std::istream& in_;
    std::ostream& out_;
    std::string error_;
};

#endif
