/*
 * Copyright (c) 2018-2021, 2024 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEReorgLayer.h"

#include "arm_compute/core/Validate.h"

#include "src/common/utils/Log.h"
#include "src/core/NEON/kernels/NEReorgLayerKernel.h"

namespace arm_compute
{
void NEReorgLayer::configure(const ITensor *input, ITensor *output, int32_t stride)
{
    ARM_COMPUTE_LOG_PARAMS(input, output, stride);

    auto k = std::make_unique<NEReorgLayerKernel>();
    k->configure(input, output, stride);
    _kernel = std::move(k);
}

Status NEReorgLayer::validate(const ITensorInfo *input, const ITensorInfo *output, int32_t stride)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output);
    return NEReorgLayerKernel::validate(input, output, stride);
}
} // namespace arm_compute
