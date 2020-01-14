#pragma once

#include <memory>

#include <pablo/parse/pablo_parser.h>
#include <pablo/parse/source_file.h>

extern std::shared_ptr<pablo::parse::PabloParser> PABLO_PARSER;

extern std::shared_ptr<pablo::parse::SourceFile> PABLO_SOURCE;
