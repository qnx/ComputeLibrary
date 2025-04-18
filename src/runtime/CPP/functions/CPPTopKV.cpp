/*
 * Copyright (c) 2019-2021, 2024 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/runtime/CPP/functions/CPPTopKV.h"

#include "arm_compute/core/CPP/kernels/CPPTopKVKernel.h"
#include "arm_compute/core/Validate.h"

#include "src/common/utils/Log.h"

namespace arm_compute
{
void CPPTopKV::configure(const ITensor *predictions, const ITensor *targets, ITensor *output, const unsigned int k)
{
    ARM_COMPUTE_LOG_PARAMS(predictions, targets, output, k);

    auto kernel = std::make_unique<CPPTopKVKernel>();
    kernel->configure(predictions, targets, output, k);
    _kernel = std::move(kernel);
}

Status CPPTopKV::validate(const ITensorInfo *predictions,
                          const ITensorInfo *targets,
                          ITensorInfo       *output,
                          const unsigned int k)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(predictions, targets, output);
    return CPPTopKVKernel::validate(predictions, targets, output, k);
}
} // namespace arm_compute
