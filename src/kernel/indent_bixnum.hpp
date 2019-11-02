//
// Created by Jeremy Schwartz on 2019-11-01.
//

#pragma once

#include <kernel/core/kernel_builder.h>
#include <pablo/pablo_kernel.h>

const uint32_t BIXNUM_WIDTH = 8;
const uint32_t INDENT_WIDTH = 1;

namespace kernel {

class IndentBixNumKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    IndentBixNumKernel(KernelBuilder b, StreamSet *poi, StreamSet *out);

protected:
    void generatePabloMethod() override;
};

}
