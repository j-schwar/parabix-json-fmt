#include "indent_bixnum.hpp"

#include "../cli.hpp"

#include <pablo/bixnum/bixnum.h>
#include <pablo/builder.hpp>

using namespace pablo;

PabloAST *atStart(PabloBuilder &pb);
BixNum aliasBixNumVar(std::vector<Var *> const &bnv);
void writeBixNumToVar(PabloBuilder &pb, std::vector<Var *> const &bnv,
                      BixNum const &bn);
BixNum advanceBixNum(PabloBuilder &pb, BixNum const &bixnum, PabloAST *cursor);

namespace kernel {

IndentBixNumKernel::IndentBixNumKernel(KernelBuilder b, StreamSet *indentData,
                                       StreamSet *out)
    : PabloKernel(b,
                  "IndentBixNum<" + std::to_string(cli::BixNumWidth) + ", " +
                      std::to_string(cli::TabWidth) + ">",
                  {{"indentData", indentData}}, {{"out", out}}, {}) {}

void IndentBixNumKernel::generatePabloMethod() {
	PabloBuilder pb(getEntryScope());

	auto const indentData = getInputStreamVar("indentData");
	auto const out = getOutputStreamVar("out");

	auto const increase = pb.createExtract(indentData, 0);
	auto const decrease = pb.createExtract(indentData, 1);

	auto const cursor = pb.createVar("cursor", atStart(pb));
	std::vector<Var *> bixnumVar(cli::BixNumWidth);
	for (size_t i = 0; i < (size_t)cli::BixNumWidth; ++i) {
		bixnumVar[i] = pb.createVar("bixnum_" + std::to_string(i),
		                            (PabloAST *)pb.createZeroes());
	}

	// We can't use bixnumVar in BixNum operations as we can't implicitly cast
	// Var * to PabloAST * inside a vector so we create this pointer alias that
	// will work.
	auto const bixnumAlias = aliasBixNumVar(bixnumVar);

	auto wb = pb.createScope();

	// BEGIN WHILE
	BixNumCompiler wbnc(wb);
	auto const inc = wb.createAnd(increase, cursor);
	auto const dec = wb.createAnd(decrease, cursor);

	// Increment/decrement BixNum.
	auto const bnAfterInc = wbnc.AddModular(bixnumAlias, {inc});
	auto const bnAfterDec = wbnc.SubModular(bnAfterInc, {dec});

	// Advance BixNum
	auto const bnAfterAdvance = advanceBixNum(wb, bnAfterDec, cursor);

	// Write new bixnum back to var
	writeBixNumToVar(wb, bixnumVar, bnAfterAdvance);

	// Advance cursor
	auto const advancedCursor = wb.createAdvance(cursor, 1);
	wb.createAssign(cursor, wb.createInFile(advancedCursor));
	// END WHILE

	// loop while cursor remains `InFile`
	pb.createWhile(cursor, wb);

	// Multiply the indentation BixNum by the number of spaces we want to
	// insert per indentation
	BixNumCompiler bnc(pb);
	auto const mulBn = bnc.MulModular(bixnumAlias, cli::TabWidth);

	// Add an extra slot for the LF character
	auto const bixnum = bnc.AddModular(mulBn, 1);

	// Write to output stream set
	for (size_t i = 0; i < bixnum.size(); ++i) {
		auto const outVar = pb.createExtract(out, i);
		pb.createAssign(outVar, bixnum[i]);
	}
}

} // namespace kernel

PabloAST *atStart(PabloBuilder &pb) {
	auto const ones = (PabloAST *)pb.createOnes();
	auto const advanced = pb.createAdvance(ones, 1);
	auto const infile = pb.createInFile(advanced);
	return pb.createNot(infile);
}

BixNum aliasBixNumVar(std::vector<Var *> const &bnv) {
	auto &&cast = [](auto const &var) {
		return reinterpret_cast<PabloAST *>(var);
	};
	BixNum bn;
	bn.resize(bnv.size());
	std::transform(bnv.begin(), bnv.end(), bn.begin(), cast);
	return bn;
}

void writeBixNumToVar(PabloBuilder &pb, std::vector<Var *> const &bnv,
                      BixNum const &bn) {
	for (size_t i = 0; i < bnv.size(); ++i) {
		pb.createAssign(bnv[i], bn[i]);
	}
}

BixNum advanceBixNum(PabloBuilder &pb, BixNum const &bixnum, PabloAST *cursor) {
	BixNum advanced(bixnum.size());
	for (size_t i = 0; i < bixnum.size(); ++i) {
		auto const masked = pb.createAnd(cursor, bixnum[i]);
		auto const advancedMask = pb.createAdvance(masked, 1);
		advanced[i] = pb.createOr(advancedMask, bixnum[i]);
	}
	return advanced;
}
