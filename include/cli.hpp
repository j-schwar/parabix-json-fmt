#pragma once

#include <llvm/Support/CommandLine.h>
#include <string>

namespace cli {

extern llvm::cl::OptionCategory Category;

extern std::string InputFile;

extern int32_t MaxIndentation;

extern int32_t TabWidth;

extern int32_t BixNumWidth;

void computeBixNumWidth();
bool validate();

} // namespace cli
