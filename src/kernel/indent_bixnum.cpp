//
// Created by Jeremy Schwartz on 2019-11-01.
//

#include "indent_bixnum.hpp"

#include <pablo/builder.hpp>
#include <pablo/bixnum/bixnum.h>

using namespace pablo;

PabloAST *atStart(PabloBuilder &pb);
BixNum advanceBixNum(PabloBuilder &pb, BixNum const &bixnum, PabloAST *cursor);

namespace kernel {

IndentBixNumKernel::IndentBixNumKernel(KernelBuilder b, StreamSet *poi, StreamSet *out)
: PabloKernel(b, "IndentBixNum",
      {{"poi", poi}},
      {{"out", out}},
      {})
{}

void IndentBixNumKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());

    auto const poi = getInputStreamVar("poi");
    auto const out = getOutputStreamVar("out");
    auto const openers = pb.createExtract(poi, 0);
    auto const closers = pb.createExtract(poi, 1);

    std::vector<Var *> bixnum(BIXNUM_WIDTH);
    for (size_t i = 0; i < bixnum.size(); ++i) {
        bixnum[i] = pb.createVar("bixnum_" + std::to_string(i), (PabloAST *) pb.createZeroes());
    }

    std::vector<PabloAST *> bn(bixnum.size());
    for (size_t i = 0; i < bixnum.size(); ++i) {
        bn[i] = (PabloAST *) bixnum[i];
    }

    auto cursor = pb.createVar("cursor", atStart(pb));

    auto wb = pb.createScope();
    BixNumCompiler bnc(wb);
    // begin while body
    auto const addAmount = wb.createAnd(openers, cursor, "add_amount");
    auto const subAmount = wb.createAnd(closers, cursor, "sub_amount");
    auto const add = bnc.AddModular(bn, {addAmount});
    auto const sub = bnc.SubModular(add, {subAmount});
    auto const advanced = advanceBixNum(wb, sub, cursor);
    for (size_t i = 0; i < bixnum.size(); ++i) {
        wb.createAssign(bixnum[i], advanced[i]);
    }
    auto const advancedCursor = wb.createAdvance(cursor, 1);
    wb.createAssign(cursor, wb.createInFile(advancedCursor));
    // end while body

    pb.createWhile(cursor, wb);

    BixNumCompiler outerBnc(pb);
    auto const correctedBn = outerBnc.SubModular(bn, {openers});
    auto const mulBn = outerBnc.MulModular(correctedBn, INDENT_WIDTH);

    for (size_t i = 0; i < bixnum.size(); ++i) {
        auto const outVar = pb.createExtract(out, i);
        pb.createAssign(outVar, mulBn[i]);
    }
}

}

PabloAST *atStart(PabloBuilder &pb) {
    auto const ones = (PabloAST *) pb.createOnes();
    auto const advanced = pb.createAdvance(ones, 1);
    auto const infile = pb.createInFile(advanced);
    return pb.createNot(infile);
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
