#include "split.hpp"

#include <pablo/builder.hpp>

using namespace pablo;

namespace kernel {

SplitKernel::SplitKernel(KernelBuilder b, StreamSet *in, StreamSet *out)
: PabloKernel(b, "SplitKernel",
        {{"in", in}},
        {{"out", out}})
{
    assert(in->getFieldWidth() == 1);
    assert(in->getNumElements() == 1);
    assert(out->getFieldWidth() == 1);
    assert(out->getNumElements() == 2);
}

void SplitKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());

    auto const in = pb.createExtract(getInputStreamVar("in"), 0);
    auto const out = getOutputStreamVar("out");

    auto const repeat = pb.createRepeat(1, 0x5555555555555555);
    auto const firsts = pb.createAnd(in, repeat);
    auto const seconds = pb.createAnd(in, pb.createNot(repeat));

    auto const out_0 = pb.createExtract(out, 0);
    auto const out_1 = pb.createExtract(out, 1);
    pb.createAssign(out_0, firsts);
    pb.createAssign(out_1, seconds);
}

}
