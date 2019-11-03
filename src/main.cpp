#include <iostream>

#include <kernel/core/idisa_target.h>
#include <kernel/pipeline/driver/cpudriver.h>
#include <toolchain/pablo_toolchain.h>
#include <pablo/parse/error.h>
#include <pablo/parse/rd_parser.h>
#include <pablo/parse/simple_lexer.h>
#include <toolchain/toolchain.h>

#include "global.hpp"
#include "cli.hpp"
#include "pipeline.hpp"

using namespace llvm;
using namespace pablo::parse;

std::shared_ptr<pablo::parse::PabloParser> PABLO_PARSER;

std::shared_ptr<pablo::parse::SourceFile> PABLO_SOURCE;

int main(int argc, char **argv) {
    codegen::ParseCommandLineOptions(argc, argv, {
        &cli::Category,
//        pablo::pablo_toolchain_flags(),
//        codegen::codegen_flags()
    });

    CPUDriver pxDriver("json-fmt");
    auto em = pablo::parse::ErrorManager::Create();
    PABLO_PARSER = RecursiveParser::Create(SimpleLexer::Create(em), em);
    PABLO_SOURCE = SourceFile::Relative("json_fmt.pablo");
    if (PABLO_SOURCE == nullptr) {
        std::cerr << "failed to load pablo source file: json_fmt.pablo\n";
        return 1;
    }

    auto fn = BuildPipeline(pxDriver);
    fn(cli::InputText.c_str(), cli::InputText.length());

    // Pipeline doesn't print out terminating LF so we do that here
    std::cout << "\n";
}
