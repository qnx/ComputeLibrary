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

#include "arm_compute/runtime/CL/functions/CLSpaceToDepthLayer.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/CL/CLScheduler.h"

#include "src/common/utils/Log.h"
#include "src/core/CL/kernels/CLSpaceToDepthLayerKernel.h"

namespace arm_compute
{
CLSpaceToDepthLayer::CLSpaceToDepthLayer() : _space_to_depth_kernel(std::make_unique<CLSpaceToDepthLayerKernel>())
{
}

CLSpaceToDepthLayer::~CLSpaceToDepthLayer() = default;

void CLSpaceToDepthLayer::configure(const ICLTensor *input, ICLTensor *output, int32_t block_shape)
{
    configure(CLKernelLibrary::get().get_compile_context(), input, output, block_shape);
}

void CLSpaceToDepthLayer::configure(const CLCompileContext &compile_context,
                                    const ICLTensor        *input,
                                    ICLTensor              *output,
                                    int32_t                 block_shape)
{
    ARM_COMPUTE_LOG_PARAMS(input, output, block_shape);
    _space_to_depth_kernel->configure(compile_context, input, output, block_shape);
}

Status CLSpaceToDepthLayer::validate(const ITensorInfo *input, const ITensorInfo *output, int32_t block_shape)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output);
    return CLSpaceToDepthLayerKernel::validate(input, output, block_shape);
}

void CLSpaceToDepthLayer::run()
{
    CLScheduler::get().enqueue(*_space_to_depth_kernel, true);
}
} // namespace arm_compute
