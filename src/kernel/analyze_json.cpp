#include "analyze_json.hpp"

#include <pablo/builder.hpp>

using namespace pablo;

namespace kernel {

AnalyzeJsonKernel::AnalyzeJsonKernel(KernelBuilder b,
                                     kernel::StreamSet *lex,
                                     kernel::StreamSet *out_lf,
                                     kernel::StreamSet *out_indent)
: PabloKernel(b, "AnalyzeJson",
              {{"lex", lex}},
              {{"lf", out_lf}, {"indent", out_indent}})
{
    assert(lex->getFieldWidth() == 1);
    assert(lex->getNumElements() == 7);
    assert(out_lf->getFieldWidth() == 1);
    assert(out_lf->getNumElements() == 1);
    assert(out_indent->getFieldWidth() == 1);
    assert(out_indent->getFieldWidth() == 2);
}

void AnalyzeJsonKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());

    auto const lex = getInputStreamVar("lex");
    auto const lf = getOutputStreamVar("lf");
    auto const indent = getOutputStreamVar("indent");

    auto const lex_openQuote = pb.createExtract(lex, 4);
    auto const lex_closeQuote = pb.createExtract(lex, 5);
    auto const lex_openers = pb.createExtract(lex, 0);
    auto const lex_closers = pb.createExtract(lex, 1);
    auto const lex_comma = pb.createExtract(lex, 2);

    auto const valueMask = pb.createIntrinsicCall(Intrinsic::InclusiveSpan, {lex_openQuote, lex_closeQuote});
    auto const notValueMask = pb.createNot(valueMask);
    auto const openers = pb.createAnd(lex_openers, notValueMask);
    auto const closers = pb.createAnd(lex_closers, notValueMask);
    auto const comma = pb.createAnd(lex_comma, notValueMask);

    auto const advancedOpeners = pb.createAdvance(openers, 1);
    auto const insertLFBefore = pb.createOr3(advancedOpeners, pb.createAdvance(comma, 1), closers);

    pb.createAssign(pb.createExtract(lf, 0), insertLFBefore);
    pb.createAssign(pb.createExtract(indent, 0), advancedOpeners);
    pb.createAssign(pb.createExtract(indent, 1), closers);
}

}
