#pragma once

#include <string>

#include <llvm/Support/CommandLine.h>

namespace cli {
    extern llvm::cl::OptionCategory Category;

    extern std::string InputFile;
}
