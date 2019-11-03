#include <iostream>

#include <llvm/Support/CommandLine.h>

#define IS_POW_2(i) (((i) > 0) && (((i) & ((i) - 1)) == 0))

using namespace llvm;

namespace cli {

cl::OptionCategory Category("JSON Format Options"); //NOLINT(cert-err58-cpp)

std::string InputFile;

int32_t MaxIndentation;

int32_t TabWidth;

int32_t BixNumWidth;

bool validate() {
    const int32_t MAX_TAB_WIDTH = 8;
    auto const validMaxTabWidth = TabWidth >= 1 && TabWidth <= MAX_TAB_WIDTH;
    if (!validMaxTabWidth) {
        std::cerr << "json-fmt: Invalid 'tab-width'\n";
        return false;
    }

    const int32_t MAX_INDENTATION = 512; // larger values degrade performance
    auto const validMaxIndentation = (MaxIndentation >= TabWidth) &&
            IS_POW_2((uint32_t) MaxIndentation) &&
            (MaxIndentation <= MAX_INDENTATION);
    if (!validMaxIndentation) {
        std::cerr << "json-fmt: Invalid 'max-indent', must be a power of 2 greater than "
                  << TabWidth
                  << " and less than "
                  << MAX_INDENTATION
                  << "\n";
        return false;
    }

    return true;
}

void computeBixNumWidth() {
    int32_t i = 0;
    auto x = (uint32_t) cli::MaxIndentation;
    while (x != 0) {
        x = x >> 1U;
        i++;
    }
    if (i == 1) {
        std::cerr << "json-fmt: Failed to compute BixNumWidth\n";
        exit(1);
    }
    BixNumWidth = i - 1;
}

} // namespace cli


static cl::opt<std::string, true> InputTextOpt( // NOLINT(cert-err58-cpp)
        cl::Positional,
        cl::location(cli::InputFile),
        cl::desc("json file"),
        cl::cat(cli::Category),
        cl::init(""));

static cl::opt<int32_t, true> MaxIndentation( // NOLINT(cert-err58-cpp)
        "max-indent",
        cl::location(cli::MaxIndentation),
        cl::desc("The maximum indentation level. (default 128)"),
        cl::cat(cli::Category),
        cl::init(128));

static cl::opt<int32_t, true> TabWidth( // NOLINT(cert-err58-cpp)
        "tab-width",
        cl::location(cli::TabWidth),
        cl::desc("The width of indentation. [1-8] (default 2)"),
        cl::cat(cli::Category),
        cl::init(2));

static cl::alias TabWidthAlias( // NOLINT(cert-err58-cpp)
        "t",
        cl::desc("Alias for tab-width"),
        cl::aliasopt(TabWidth),
        cl::cat(cli::Category),
        cl::NotHidden);
