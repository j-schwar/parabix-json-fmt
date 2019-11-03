#pragma once

#include <pablo/pablo_kernel.h>
#include <kernel/core/kernel_builder.h>

namespace kernel {

class SplitKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    SplitKernel(KernelBuilder b, StreamSet *in, StreamSet *out);
protected:
    void generatePabloMethod() override;
};

}
