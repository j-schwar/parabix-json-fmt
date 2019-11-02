#pragma once

#include <kernel/pipeline/driver/cpudriver.h>
#include <pablo/parse/pablo_parser.h>
#include <pablo/parse/source_file.h>

using PipelineFunction = void(*)(const char *text, size_t length);

PipelineFunction BuildPipeline(CPUDriver &driver);
