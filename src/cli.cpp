#include "cli.hpp"

#include <llvm/Support/CommandLine.h>

using namespace llvm;

namespace cli {

cl::OptionCategory Category("JSON Format Options"); //NOLINT(cert-err58-cpp)

std::string InputText;

} // namespace cli


static cl::opt<std::string, true> __unused InputTextOpt( // NOLINT(cert-err58-cpp)
        cl::Positional,
        cl::location(cli::InputText),
        cl::desc("<json>"),
        cl::Required,
        cl::cat(cli::Category),
        cl::init(""));
