#pragma once

#include <cassert>
#include <tuple>
#include <vector>

#include "global.hpp"
#include "kernel/lex_json.hpp"
#include "kernel/indent_bixnum.hpp"
#include "kernel/split.hpp"

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


inline kernel::StreamSet *ReadSource(PipelineBuilder P, llvm::StringRef fdScalarName) {
    auto const out = P->CreateStreamSet(1, 8);
    P->CreateKernelCall<kernel::ReadSourceKernel>(P->getInputScalar(fdScalarName), out);
    return out;
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
    P->CreateKernelCall<kernel::SplitKernel>(compressed, compressedSplit);

    auto const split = P->CreateStreamSet(2, 1);
    kernel::SpreadByMask(P, stream, compressedSplit, split);
    return split;
}


inline std::tuple<kernel::StreamSet *, kernel::StreamSet *> AnalyzeJson(PipelineBuilder P, kernel::StreamSet *lex) {
    auto const lfData = P->CreateStreamSet(1, 1);
    auto const indentData = P->CreateStreamSet(2, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
        PABLO_PARSER,
        PABLO_SOURCE,
        "AnalyzeJson",
        kernel::Bindings {
            kernel::Binding {"lex", lex}
        },
        kernel::Bindings {
            kernel::Binding {"lf", lfData},
            kernel::Binding {"indent", indentData}
        }
    );
    return std::make_tuple(lfData, indentData);
}


inline kernel::StreamSet *FindSpreadInsertLocations(PipelineBuilder P, kernel::StreamSet *mask) {
    auto const insert = P->CreateStreamSet(2, 1);
    P->CreateKernelCall<pablo::PabloSourceKernel>(
        PABLO_PARSER,
        PABLO_SOURCE,
        "FindSpreadInsertLocations",
        kernel::Bindings {
            kernel::Binding {"mask", mask}
        },
        kernel::Bindings {
            kernel::Binding {"insert", insert}
        }
    );
    return insert;
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
