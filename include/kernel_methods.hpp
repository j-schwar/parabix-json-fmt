#pragma once

#include <cassert>
#include <tuple>
#include <vector>

#include <kernel/basis/s2p_kernel.h>
#include <kernel/io/source_kernel.h>
#include <kernel/pipeline/pipeline_builder.h>
#include <kernel/streamutils/deletion.h>
#include <kernel/streamutils/pdep_kernel.h>
#include <re/adt/re_cc.h>
#include <re/cc/cc_kernel.h>

#include "cli.hpp"
#include "global.hpp"
#include "kernel/indent_bixnum.hpp"
#include "kernel/lex_json.hpp"

using PipelineBuilder = std::unique_ptr<kernel::ProgramBuilder> &;

/**
 * \brief Wraps a call to Parabix's Serial To Parallel kernel.
 *
 * Converts a stream set of type <i8>[1] into on of type <i1>[8] by placing the
 * ith bit of each byte in the ith stream in the resultant stream set.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param input the <i8>[1] stream set to convert to parallel.
 * \return a stream set of type <i1>[8].
 */
inline kernel::StreamSet *S2P(PipelineBuilder P, kernel::StreamSet *input) {
	assert(input->getFieldWidth() == 8);
	assert(input->getNumElements() == 1);

	auto const out = P->CreateStreamSet(8, 1);
	P->CreateKernelCall<kernel::S2PKernel>(input, out);
	return out;
}

/**
 * \brief Wraps a call to Parabix's Read Source kernel.
 *
 * Captures input from some file descriptor using the `read` system call and
 * returns the results as a <i8>[1] stream set.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param fdScalarName the name of the pipeline's input scalar which holds the
 * file descriptor value.
 * \return A stream set of type <i8>[1].
 */
inline kernel::StreamSet *ReadSource(PipelineBuilder P,
                                     llvm::StringRef fdScalarName) {
	auto const out = P->CreateStreamSet(1, 8);
	P->CreateKernelCall<kernel::ReadSourceKernel>(P->getInputScalar(fdScalarName),
	                                              out);
	return out;
}

/**
 * \brief Performs initial processing on some basis bits.
 *
 * Returns a stream set of type <i1>[6] with the following streams by index:
 *
 *  0. Positions of JSON openers (i.e., '[' and '{')
 *  1. Positions of JSON closers (i.e., ']' and '}')
 *  2. Positions of commas (i.e., ',')
 *  3. Positions of colons (i.e., ':')
 *  4. Positions of double quotes (i.e., '"')
 *  5. A whitespace mask (i.e., any of: ' ', '\t', '\r', '\n')
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param basis the set of basis streams (<i1>[8]) for the input source.
 * \return A stream set of type <i1>[6].
 */
inline kernel::StreamSet *Lex(PipelineBuilder P, kernel::StreamSet *basis) {
	auto const out = P->CreateStreamSet(6, 1);
	P->CreateKernelCall<kernel::LexJsonKernel>(basis, out);
	return out;
}

/**
 * \brief Wraps a call to our `IndexBixNumKernel` kernel..
 *
 * Constructs a `BixNum` denoting the number of bits that need to be inserted to
 * allow the correct amount of whitespace to be inserted.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param poi a filtered stream set containing only the points of interest
 * needed to compute the index bixnum.
 * \return A `BixNum`.
 *
 * \see kernel::IndentBixNumKernel
 */
inline kernel::StreamSet *IndentBixNum(PipelineBuilder P,
                                       kernel::StreamSet *poi) {
	auto const out = P->CreateStreamSet(cli::BixNumWidth, 1);
	P->CreateKernelCall<kernel::IndentBixNumKernel>(poi, out);
	return out;
}

/**
 * \brief Splits a <i1>[1] stream set into a <i1>[2] stream set where every even
 * bit in the input stream is in the first stream of the output and every odd
 * bit in the second.
 *
 * Example:
 *
 *     Input : 1..1...1.1.1
 *
 *     Output: 1......1...1
 *           : ...1.....1..
 *
 * Note that this operation is quite expensive as it requires filtering and
 * spreading the input stream in order to get the desired output. It may be
 * necessary to see if there is other ways to get the desired output for our
 * problem domain.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param stream a stream set of type <i1>[1] to split.
 * \return A stream set of type <i1>[2].
 */
inline kernel::StreamSet *Split(PipelineBuilder P, kernel::StreamSet *stream) {
	auto const compressed = P->CreateStreamSet(1, 1);
	kernel::FilterByMask(P, stream, stream, compressed);

	auto const compressedSplit = P->CreateStreamSet(2, 1);
	P->CreateKernelCall<pablo::PabloSourceKernel>(
	    PABLO_PARSER, PABLO_SOURCE, "Split",
	    kernel::Bindings{kernel::Binding{"in", compressed}},
	    kernel::Bindings{kernel::Binding{"out", compressedSplit}});

	auto const split = P->CreateStreamSet(2, 1);
	kernel::SpreadByMask(P, stream, compressedSplit, split);
	return split;
}

/**
 * \brief Locates where to insert LF characters as well as where the indent
 * amount should increase and decrease (e.g., opening and closing of scopes).
 *
 * Returns the LF insertion stream set as the first element of a tuple with the
 * indent increment/decrement data being the second element.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param lex the resultant stream set of a call to `Lex`.
 * \return A tuple of stream sets with types <i1>[1] and <i1>[2].
 */
inline std::tuple<kernel::StreamSet *, kernel::StreamSet *>
AnalyzeJson(PipelineBuilder P, kernel::StreamSet *lex) {
	auto const lfData = P->CreateStreamSet(1, 1);
	auto const indentData = P->CreateStreamSet(2, 1);
	P->CreateKernelCall<pablo::PabloSourceKernel>(
	    PABLO_PARSER, PABLO_SOURCE, "AnalyzeJson",
	    kernel::Bindings{kernel::Binding{"lex", lex}},
	    kernel::Bindings{kernel::Binding{"lf", lfData},
	                     kernel::Binding{"indent", indentData}});
	return std::make_tuple(lfData, indentData);
}

/**
 * \brief Computes where to insert LF and space characters from a spread mask.
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param mask the spread mask used to insert the new stream positions for
 * whitespace.
 * \return A stream set of type <i1>[2] if LF insertion locations at index 0 and
 * space insertion locations at index 1.
 */
inline kernel::StreamSet *FindSpreadInsertLocations(PipelineBuilder P,
                                                    kernel::StreamSet *mask) {
	auto const insert = P->CreateStreamSet(2, 1);
	P->CreateKernelCall<pablo::PabloSourceKernel>(
	    PABLO_PARSER, PABLO_SOURCE, "FindSpreadInsertLocations",
	    kernel::Bindings{kernel::Binding{"mask", mask}},
	    kernel::Bindings{kernel::Binding{"insert", insert}});
	return insert;
}

/**
 * \brief Returns a modified basis stream set after setting each position marked
 * by a given mask to a LF character.
 *
 * It is assumed that location marked by mask is a 0-byte in the input basis
 * stream set.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param mask the locations to set to LF.
 * \param basis the basis stream set to set LF characters in.
 * \return A new basis stream set (<i1>[8]).
 */
inline kernel::StreamSet *InsertLF(PipelineBuilder P, kernel::StreamSet *mask,
                                   kernel::StreamSet *basis) {
	auto const out = P->CreateStreamSet(8, 1);
	P->CreateKernelCall<pablo::PabloSourceKernel>(
	    PABLO_PARSER, PABLO_SOURCE, "InsertLF",
	    kernel::Bindings{kernel::Binding{"mask", mask},
	                     kernel::Binding{"basis", basis}},
	    kernel::Bindings{kernel::Binding{"out", out}});
	return out;
}

/**
 * \brief Returns a modified basis stream set after setting each position marked
 * by a given mask to a space character.
 *
 * It is assumed that location marked by mask is a 0-byte in the input basis
 * stream set.
 *
 * \param P the pipeline builder to use when constructing the kernel call.
 * \param mask the locations to set to a space.
 * \param basis the basis stream set to set space characters in.
 * \return A new basis stream set (<i1>[8]).
 */
inline kernel::StreamSet *InsertSpace(PipelineBuilder P,
                                      kernel::StreamSet *mask,
                                      kernel::StreamSet *basis) {
	auto const out = P->CreateStreamSet(8, 1);
	P->CreateKernelCall<pablo::PabloSourceKernel>(
	    PABLO_PARSER, PABLO_SOURCE, "InsertSpace",
	    kernel::Bindings{kernel::Binding{"mask", mask},
	                     kernel::Binding{"basis", basis}},
	    kernel::Bindings{kernel::Binding{"out", out}});
	return out;
}
