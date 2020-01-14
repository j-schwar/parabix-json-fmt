#include <pipeline.hpp>

#include <kernel/basis/p2s_kernel.h>
#include <kernel/io/source_kernel.h>
#include <kernel/io/stdout_kernel.h>
#include <kernel/streamutils/pdep_kernel.h>
#include <kernel/streamutils/stream_select.h>

#include <kernel_methods.hpp>

using namespace llvm;
using namespace kernel;

namespace su = streamutils;

PipelineFunction BuildPipeline(CPUDriver &driver) {
	auto &b = driver.getBuilder();
	auto P = driver.makePipeline({Binding{b->getInt32Ty(), "fd"}});

	// Get input stream and compute basis bits
	auto const inputStream = ReadSource(P, "fd");
	auto const basis = S2P(P, inputStream);

	// Lex streams
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
	auto const lex = su::Select(P, {{partialLex, su::Range(0, 4)},
	                                {splitQuotes, su::Range(0, 2)},
	                                {partialLex, {5}}});

	// Find locations to insert LF and increment/decrement indent
	auto const [lfData, indentData] = AnalyzeJson(P, lex);

	// Filter indent streams by the LF insertion mask
	auto const filteredIndentData = P->CreateStreamSet(2, 1);
	FilterByMask(P, lfData, indentData, filteredIndentData);

	// Use filtered indent streams to compute bit-insertion BixNum
	auto const filteredBixNum = IndentBixNum(P, filteredIndentData);

	// Spread BixNum back to the original stream size
	auto const bixnum = P->CreateStreamSet(cli::BixNumWidth, 1);
	SpreadByMask(P, lfData, filteredBixNum, bixnum);

	// Spread basis allowing us to insert LF and space characters in the proper
	// locations
	auto const spreadMask = InsertionSpreadMask(P, bixnum);
	auto const spreadBasis = P->CreateStreamSet(8, 1);
	SpreadByMask(P, spreadMask, basis, spreadBasis);

	// Compute where to insert LF and space characters
	auto const insertLocations = FindSpreadInsertLocations(P, spreadMask);

	// Insert LF then spaces into Basis
	auto const withLF =
	    InsertLF(P, su::Select(P, insertLocations, 0), spreadBasis);
	auto const withSpaces =
	    InsertSpace(P, su::Select(P, insertLocations, 1), withLF);

	// Print to stdout
	auto const u8bytes = P->CreateStreamSet(1, 8);
	P->CreateKernelCall<P2SKernel>(withSpaces, u8bytes);
	P->CreateKernelCall<StdOutKernel>(u8bytes);

	return reinterpret_cast<PipelineFunction>(P->compile());
}
