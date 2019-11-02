#pragma once

#include <kernel/core/kernel_builder.h>
#include <pablo/pablo_kernel.h>

namespace kernel {

/**
 * Computes the following streams:
 *  [0] JSON Openers ('{', '[')
 *  [1] JSON Closers ('}', ']')
 *  [2] Commas (',')
 *  [3] Colons (':')
 *  [4] Quotes
 *  [5] Whitespace
 */
class LexJsonKernel : public pablo::PabloKernel {
public:
    using KernelBuilder = const std::unique_ptr<KernelBuilder> &;

    LexJsonKernel(KernelBuilder b, StreamSet *basis, StreamSet *out);

protected:
    void generatePabloMethod() override;
};

}
