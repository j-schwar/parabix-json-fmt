#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>

#include <kernel/core/idisa_target.h>
#include <kernel/pipeline/driver/cpudriver.h>
#include <llvm/Support/Host.h>
#include <pablo/parse/error.h>
#include <pablo/parse/rd_parser.h>
#include <pablo/parse/simple_lexer.h>
#include <toolchain/pablo_toolchain.h>
#include <toolchain/toolchain.h>

#include "cli.hpp"
#include "global.hpp"
#include "pipeline.hpp"

using namespace llvm;
using namespace pablo::parse;

std::shared_ptr<pablo::parse::PabloParser> PABLO_PARSER;

std::shared_ptr<pablo::parse::SourceFile> PABLO_SOURCE;

int main(int argc, char **argv) {
	codegen::ParseCommandLineOptions(argc, argv,
	                                 {&cli::Category,
	                                  pablo::pablo_toolchain_flags(),
	                                  codegen::codegen_flags()});

	if (!cli::validate()) {
		return 1;
	}
	cli::computeBixNumWidth();

	int32_t fd = 0;
	if (!cli::InputFile.empty()) {
		struct stat sb {};
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
	auto em = pablo::parse::ErrorManager::Create();
	PABLO_PARSER = RecursiveParser::Create(SimpleLexer::Create(em), em);
	PABLO_SOURCE = SourceFile::Relative("json_fmt.pablo");
	if (PABLO_SOURCE == nullptr) {
		std::cerr << "failed to load pablo source file: json_fmt.pablo\n";
		return 1;
	}

	auto fn = BuildPipeline(pxDriver);
	fn(fd);

	// Pipeline doesn't print out terminating LF so we do that here
	std::putchar('\n');
}
