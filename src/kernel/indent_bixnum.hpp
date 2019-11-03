#pragma once

#include <kernel/core/kernel_builder.h>
#include <pablo/pablo_kernel.h>


namespace kernel {

class IndentBixNumKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    IndentBixNumKernel(KernelBuilder b, StreamSet *indentData, StreamSet *out);

protected:
    void generatePabloMethod() override;
};

}
