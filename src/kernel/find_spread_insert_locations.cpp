#include "find_spread_insert_locations.hpp"

#include <pablo/builder.hpp>

using namespace pablo;

namespace kernel {

FindSpreadInsertLocationsKernel::FindSpreadInsertLocationsKernel(KernelBuilder b, StreamSet *in, StreamSet *out)
        : PabloKernel(b, "FindSpreadInsertLocations",
                      {{"in", in}},
                      {{"out", out}})
{
    assert(in->getFieldWidth() == 1);
    assert(in->getNumElements() == 1);
    assert(out->getFieldWidth() == 1);
    assert(out->getNumElements() == 2);
}

void FindSpreadInsertLocationsKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());

    auto const in = pb.createExtract(getInputStreamVar("in"), 0);
    auto const out = getOutputStreamVar("out");

    auto const notMask = pb.createNot(in);
    auto const advanced = pb.createAdvance(in, 1);
    auto const lf = pb.createAnd(advanced, notMask);
    auto const space = pb.createInFile(pb.createAnd(notMask, pb.createNot(lf)));

    pb.createAssign(pb.createExtract(out, 0), lf);
    pb.createAssign(pb.createExtract(out, 1), space);
}

}
