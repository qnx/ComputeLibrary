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
#include "arm_compute/runtime/CL/functions/CLFFT2D.h"

#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/CL/CLScheduler.h"

#include "src/common/utils/Log.h"
#include "src/core/CL/kernels/CLFFTDigitReverseKernel.h"
#include "src/core/CL/kernels/CLFFTRadixStageKernel.h"
#include "src/core/CL/kernels/CLFFTScaleKernel.h"

namespace arm_compute
{
CLFFT2D::CLFFT2D(std::shared_ptr<IMemoryManager> memory_manager)
    : _memory_group(memory_manager),
      _first_pass_func(memory_manager),
      _second_pass_func(memory_manager),
      _first_pass_tensor()
{
}

CLFFT2D::~CLFFT2D() = default;

void CLFFT2D::configure(const ICLTensor *input, ICLTensor *output, const FFT2DInfo &config)
{
    configure(CLKernelLibrary::get().get_compile_context(), input, output, config);
}

void CLFFT2D::configure(const CLCompileContext &compile_context,
                        const ICLTensor        *input,
                        ICLTensor              *output,
                        const FFT2DInfo        &config)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, output);
    ARM_COMPUTE_ERROR_THROW_ON(CLFFT2D::validate(input->info(), output->info(), config));
    ARM_COMPUTE_LOG_PARAMS(input, output, config);

    // Setup first pass
    FFT1DInfo first_pass_config;
    first_pass_config.axis      = config.axis0;
    first_pass_config.direction = config.direction;
    _memory_group.manage(&_first_pass_tensor);
    _first_pass_func.configure(compile_context, input, &_first_pass_tensor, first_pass_config);

    // Setup second pass
    FFT1DInfo second_pass_config;
    second_pass_config.axis      = config.axis1;
    second_pass_config.direction = config.direction;
    _second_pass_func.configure(compile_context, &_first_pass_tensor, output, second_pass_config);
    _first_pass_tensor.allocator()->allocate();
}

Status CLFFT2D::validate(const ITensorInfo *input, const ITensorInfo *output, const FFT2DInfo &config)
{
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(input, output);
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_NOT_IN(input, DataType::F16, DataType::F32);

    // Create intermediate tensor info
    TensorInfo first_pass_tensor(input->clone()->set_is_resizable(true).reset_padding().set_num_channels(2));

    // Validate first pass
    FFT1DInfo first_pass_config;
    first_pass_config.axis      = config.axis0;
    first_pass_config.direction = config.direction;
    ARM_COMPUTE_RETURN_ON_ERROR(CLFFT1D::validate(input, &first_pass_tensor, first_pass_config));

    // Validate second pass
    FFT1DInfo second_pass_config;
    second_pass_config.axis      = config.axis1;
    second_pass_config.direction = config.direction;
    ARM_COMPUTE_RETURN_ON_ERROR(CLFFT1D::validate(&first_pass_tensor, output, second_pass_config));

    // Checks performed when output is configured
    if ((output != nullptr) && (output->total_size() != 0))
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_SHAPES(input, output);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, output);
    }

    return Status{};
}

void CLFFT2D::run()
{
    MemoryGroupResourceScope scope_mg(_memory_group);

    _first_pass_func.run();
    _second_pass_func.run();
}
} // namespace arm_compute
