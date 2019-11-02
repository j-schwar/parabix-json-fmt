#pragma once

#include <cassert>
#include <vector>

#include "global.hpp"
#include "kernel/lex_json.hpp"
#include "kernel/indent_bixnum.hpp"

#include <kernel/pipeline/pipeline_builder.h>
#include <kernel/io/source_kernel.h>
#include <kernel/basis/s2p_kernel.h>
#include <kernel/streamutils/deletion.h>
#include <kernel/streamutils/pdep_kernel.h>
#include <re/adt/re_cc.h>
#include <re/cc/cc_kernel.h>

using PipelineBuilder = std::unique_ptr<kernel::ProgramBuilder> &;


inline kernel::StreamSet *S2P(PipelineBuilder P, kernel::StreamSet *input) {
    assert(input->getFieldWidth() == 8);
    assert(input->getNumElements() == 1);

    auto const out = P->CreateStreamSet(8, 1);
    P->CreateKernelCall<kernel::S2PKernel>(input, out);
    return out;
}


inline kernel::StreamSet *MemorySource(PipelineBuilder P,
                                       llvm::StringRef ptrScalarName,
                                       llvm::StringRef sizeScalarName) {
    auto const out = P->CreateStreamSet(1, 8);
    P->CreateKernelCall<kernel::MemorySourceKernel>(
            P->getInputScalar(ptrScalarName),
            P->getInputScalar(sizeScalarName),
            out);
    return out;
}


inline kernel::StreamSet *CompileCC(PipelineBuilder P,
                                    llvm::StringRef ccName,
                                    std::vector<re::CC *> const &cc,
                                    kernel::StreamSet *basis) {
    auto const out = P->CreateStreamSet(1, 1);
    P->CreateKernelCall<kernel::CharacterClassKernelBuilder>(ccName, cc, basis, out);
    return out;
}


inline kernel::StreamSet *CompileCC(PipelineBuilder P,
                                    llvm::StringRef ccName,
                                    re::CC * const cc,
                                    kernel::StreamSet *basis) {
    return CompileCC(P, ccName, std::vector{cc}, basis);
}


inline kernel::StreamSet *Lex(PipelineBuilder P, kernel::StreamSet *basis) {
    auto const out = P->CreateStreamSet(6, 1);
    P->CreateKernelCall<kernel::LexJsonKernel>(basis, out);
    return out;
}


inline kernel::StreamSet *IndentBixNum(PipelineBuilder P, kernel::StreamSet *poi) {
    auto const out = P->CreateStreamSet(BIXNUM_WIDTH, 1);
    P->CreateKernelCall<kernel::IndentBixNumKernel>(poi, out);
    return out;
}


inline kernel::StreamSet *Split(PipelineBuilder P, kernel::StreamSet *stream) {
    auto const compressed = P->CreateStreamSet(1, 1);
    kernel::FilterByMask(P, stream, stream, compressed);

    auto const compressedSplit = P->CreateStreamSet(2, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
            PABLO_PARSER,
            PABLO_SOURCE,
            "Split",
            kernel::Bindings {
                    kernel::Binding {"in", compressed}
            },
            kernel::Bindings {
                    kernel::Binding {"out", compressedSplit}
            }
    );

    auto const split = P->CreateStreamSet(2, 1);
    kernel::SpreadByMask(P, stream, compressedSplit, split);
    return split;
}


inline kernel::StreamSet *AnalyzeJson(PipelineBuilder P, kernel::StreamSet *lex) {
    auto const analyzedJson = P->CreateStreamSet(4, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
            PABLO_PARSER,
            PABLO_SOURCE,
            "AnalyzeJson",
            kernel::Bindings {
                    kernel::Binding {"lex", lex}
            },
            kernel::Bindings {
                    kernel::Binding {"out", analyzedJson}
            }
    );
    return analyzedJson;
}


inline kernel::StreamSet *InsertLF(PipelineBuilder P, kernel::StreamSet *mask, kernel::StreamSet *basis) {
    auto const out = P->CreateStreamSet(8, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
            PABLO_PARSER,
            PABLO_SOURCE,
            "InsertLF",
            kernel::Bindings {
                    kernel::Binding {"mask", mask},
                    kernel::Binding {"basis", basis}
            },
            kernel::Bindings {
                    kernel::Binding {"out", out}
            }
    );
    return out;
}


inline kernel::StreamSet *InsertSpace(PipelineBuilder P, kernel::StreamSet *mask, kernel::StreamSet *basis) {
    auto const out = P->CreateStreamSet(8, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
            PABLO_PARSER,
            PABLO_SOURCE,
            "InsertSpace",
            kernel::Bindings {
                    kernel::Binding {"mask", mask},
                    kernel::Binding {"basis", basis}
            },
            kernel::Bindings {
                    kernel::Binding {"out", out}
            }
    );
    return out;
}