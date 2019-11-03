#pragma once

#include <pablo/pablo_kernel.h>
#include <kernel/core/kernel_builder.h>

namespace kernel {

class InsertCharsKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    InsertCharsKernel(KernelBuilder b, StreamSet *basis, StreamSet *insertLocations, StreamSet *out);
protected:
    void generatePabloMethod() override;
};

}
