#include "lex_json.hpp"

#include <cassert>
#include <pablo/builder.hpp>
#include <re/cc/cc_compiler_target.h>

using namespace pablo;

namespace kernel {

LexJsonKernel::LexJsonKernel(KernelBuilder &b, StreamSet *basis, StreamSet *out)
: PabloKernel(b, "LexJson",
        {{"basis", basis}},
        {{"out", out}},
        {})
{
    assert(basis->getNumElements() == 8);
    assert(basis->getFieldWidth() == 1);
    assert(out->getNumElements() == 6);
    assert(out->getFieldWidth() == 1);
}

void LexJsonKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());
    std::vector<PabloAST *> basis = getInputStreamSet("basis");
    auto const out = getOutputStreamVar("out");
    cc::Parabix_CC_Compiler_Builder ccc(getEntryScope(), basis);

    // JSON Openers
    auto const cc_openers = re::makeCC(re::makeByte('{'), re::makeByte('['));
    auto const openers = ccc.compileCC(cc_openers);
    pb.createAssign(pb.createExtract(out, 0), openers);

    // JSON Closers
    auto const cc_closers = re::makeCC(re::makeByte('}'), re::makeByte(']'));
    auto const closers = ccc.compileCC(cc_closers);
    pb.createAssign(pb.createExtract(out, 1), closers);

    // Commas
    auto const cc_commas = re::makeByte(',');
    auto const commas = ccc.compileCC(cc_commas);
    pb.createAssign(pb.createExtract(out, 2), commas);

    // Colons
    auto const cc_colons = re::makeByte(':');
    auto const colons = ccc.compileCC(cc_colons);
    pb.createAssign(pb.createExtract(out, 3), colons);

    //  Quotes
    auto const cc_quote = re::makeByte('\"');
    auto const quote = ccc.compileCC(cc_quote);
    pb.createAssign(pb.createExtract(out, 4), quote);

    // Whitespace Mask
    auto cc_ws = re::makeCC(re::makeByte('\t', '\n'), re::makeByte('\r'));
    cc_ws = re::makeCC(re::makeByte(' '), cc_ws);
    auto const ws = ccc.compileCC(cc_ws);
    pb.createAssign(pb.createExtract(out, 5), ws);
}

}
