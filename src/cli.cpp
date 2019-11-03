#include "cli.hpp"

#include <llvm/Support/CommandLine.h>

using namespace llvm;

namespace cli {

cl::OptionCategory Category("JSON Format Options"); //NOLINT(cert-err58-cpp)

std::string InputFile;

} // namespace cli


static cl::opt<std::string, true> InputTextOpt( // NOLINT(cert-err58-cpp)
        cl::Positional,
        cl::location(cli::InputFile),
        cl::desc("json file"),
        cl::cat(cli::Category),
        cl::init(""));
