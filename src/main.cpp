#include <iostream>

#include <kernel/core/idisa_target.h>
#include <kernel/pipeline/driver/cpudriver.h>
#include <toolchain/pablo_toolchain.h>
#include <toolchain/toolchain.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "cli.hpp"
#include "pipeline.hpp"

using namespace llvm;
using namespace pablo::parse;

int main(int argc, char **argv) {
    codegen::ParseCommandLineOptions(argc, argv, {
        &cli::Category,
        pablo::pablo_toolchain_flags(),
        codegen::codegen_flags()
    });

    int32_t fd = 0;
    if (!cli::InputFile.empty()) {
        struct stat sb{};
        fd = open(cli::InputFile.c_str(), O_RDONLY);
        if (LLVM_UNLIKELY(fd == -1)) {
            if (errno == EACCES) {
                std::cerr << "json-fmt: " << cli::InputFile << ": Permission denied.\n";
            } else if (errno == ENOENT) {
                std::cerr << "json-fmt: " << cli::InputFile << ": No such file.\n";
            } else {
                std::cerr << "json-fmt: " << cli::InputFile << ": Failed.\n";
            }
            return errno;
        }
        if (stat(cli::InputFile.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
            std::cerr << "json-fmt: " << cli::InputFile << ": Is a directory.\n";
            close(fd);
            return -1;
        }
    }

    CPUDriver pxDriver("json-fmt");
    auto fn = BuildPipeline(pxDriver);
    fn(fd);

    // Pipeline doesn't print out terminating LF so we do that here
    std::putchar('\n');
}
