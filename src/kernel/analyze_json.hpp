#pragma once

#include <pablo/pablo_kernel.h>
#include <kernel/core/kernel_builder.h>

namespace kernel {

class AnalyzeJsonKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    AnalyzeJsonKernel(KernelBuilder b, StreamSet *lex, StreamSet *out_lf, StreamSet *out_indent);
protected:
    void generatePabloMethod() override;
};

}
