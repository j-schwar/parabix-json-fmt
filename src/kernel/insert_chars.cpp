#include "insert_chars.hpp"

#include <pablo/builder.hpp>

using namespace pablo;

namespace kernel {

InsertCharsKernel::InsertCharsKernel(KernelBuilder b,
                                     StreamSet *basis,
                                     StreamSet *insertLocations,
                                     StreamSet *out)
: PabloKernel(b, "InsertChars",
        {{"basis", basis}, {"insert", insertLocations}},
        {{"out", out}})
{}

void InsertCharsKernel::generatePabloMethod() {
    PabloBuilder pb(getEntryScope());

    auto const basis = getInputStreamSet("basis");
    auto const insert = getInputStreamVar("insert");
    auto const lf = pb.createExtract(insert, 0);
    auto const space = pb.createExtract(insert, 1);
    auto const out = getOutputStreamVar("out");

    pb.createAssign(pb.createExtract(out, 0), basis[0]);
    pb.createAssign(pb.createExtract(out, 1), pb.createOr(basis[1], lf));
    pb.createAssign(pb.createExtract(out, 2), basis[2]);
    pb.createAssign(pb.createExtract(out, 3), pb.createOr(basis[3], lf));
    pb.createAssign(pb.createExtract(out, 4), basis[4]);
    pb.createAssign(pb.createExtract(out, 5), pb.createOr(basis[5], space));
    pb.createAssign(pb.createExtract(out, 6), basis[6]);
    pb.createAssign(pb.createExtract(out, 7), basis[7]);
}

}
