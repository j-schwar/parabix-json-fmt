#include "pipeline.hpp"
#include "kernel_methods.hpp"

#include <kernel/basis/p2s_kernel.h>
#include <kernel/io/source_kernel.h>
#include <kernel/io/stdout_kernel.h>
#include <kernel/streamutils/pdep_kernel.h>
#include <kernel/streamutils/stream_select.h>
#include <kernel/streamutils/collapse.h>
#include <kernel/util/debug_display.h>

using namespace llvm;
using namespace kernel;

namespace su = streamutils;

PipelineFunction BuildPipeline(CPUDriver &driver) {
    auto &b = driver.getBuilder();
    auto P = driver.makePipeline({
        Binding{b->getInt8PtrTy(), "ptr"},
        Binding{b->getSizeTy(), "len"}
    });

    auto const inputStream = MemorySource(P, "ptr", "len");
    auto const basis = S2P(P, inputStream);

    auto const partialLex = Lex(P, basis);
    auto const quotes = su::Select(P, partialLex, 4);
    auto const splitQuotes = Split(P, quotes);

    // lex consists of:
    //  [0] JSON Openers ('{', '[')
    //  [1] JSON Closers ('}', ']')
    //  [2] Commas (',')
    //  [3] Colons (':')
    //  [4] Opening Quotes
    //  [5] Closing Quotes
    //  [6] Whitespace
    auto const lex = su::Select(P, {
        {partialLex, su::Range(0, 4)},
        {splitQuotes, su::Range(0, 2)},
        {partialLex, {5}}
    });

    auto const analyzedJson = AnalyzeJson(P, lex);

    auto const poi = su::Select(P, analyzedJson, su::Range(0, 3));
    auto const poiMask = su::Collapse(P, poi);
    auto const compressedPoi = P->CreateStreamSet(2, 1);
    FilterByMask(P, poiMask, poi, compressedPoi);

    // Compute indentation BixNum
    auto const compressedBixnum = IndentBixNum(P, compressedPoi);
    auto const bixnum = P->CreateStreamSet(BIXNUM_WIDTH, 1);
    SpreadByMask(P, poiMask, compressedBixnum, bixnum);

    // Spread Basis and LF insertion mask to make room for the indentation
    auto const spreadMask = InsertionSpreadMask(P, bixnum);
    auto const spreadBasis = P->CreateStreamSet(8, 1);
    SpreadByMask(P, spreadMask, basis, spreadBasis);

    // Insert spaces into Basis
    auto const withSpaces = InsertSpace(P, spreadMask, spreadBasis);
//    util::DebugDisplay(P, "spread mask", spreadMask);

    // TODO: Find a way to avoid re-lexing
    auto const partialLex2 = Lex(P, withSpaces);
    auto const quotes2 = su::Select(P, partialLex2, 4);
    auto const splitQuotes2 = Split(P, quotes2);

    // lex consists of:
    //  [0] JSON Openers ('{', '[')
    //  [1] JSON Closers ('}', ']')
    //  [2] Commas (',')
    //  [3] Colons (':')
    //  [4] Opening Quotes
    //  [5] Closing Quotes
    //  [6] Whitespace
    auto const lex2 = su::Select(P, {
            {partialLex2, su::Range(0, 4)},
            {splitQuotes2, su::Range(0, 2)},
            {partialLex2, {5}}
    });

    auto const analyzedJson2 = AnalyzeJson(P, lex2);

    auto const lfInsertMask = su::Select(P, analyzedJson2, 3);

    // Spread Basis again to make room for line feeds
    auto const lfSpreadMask = UnitInsertionSpreadMask(P, lfInsertMask, InsertPosition::Before);
    auto const lfSpreadBasis = P->CreateStreamSet(8, 1);
    SpreadByMask(P, lfSpreadMask, withSpaces, lfSpreadBasis);

    // Insert LF characters
    auto const withLF = InsertLF(P, lfSpreadMask, lfSpreadBasis);

    // Print to stdout
    StreamSet * const u8bytes = P->CreateStreamSet(1, 8);
    P->CreateKernelCall<P2SKernel>(withLF, u8bytes);
    P->CreateKernelCall<StdOutKernel>(u8bytes);

//    util::DebugDisplay(P, "split_quotes", splitQuotes);
//    util::DebugDisplay(P, "json_lex", lex);
//    util::DebugDisplay(P, "analyzed", analyzedJson);
//    util::DebugDisplay(P, "spread_mask", spreadMask);

    return reinterpret_cast<PipelineFunction>(P->compile());
}
