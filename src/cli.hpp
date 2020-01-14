#pragma once

#include <string>

#include <llvm/Support/CommandLine.h>

namespace cli {

extern llvm::cl::OptionCategory Category;

extern std::string InputFile;

extern int32_t MaxIndentation;

extern int32_t TabWidth;

extern int32_t BixNumWidth;

void computeBixNumWidth();
bool validate();

} // namespace cli
