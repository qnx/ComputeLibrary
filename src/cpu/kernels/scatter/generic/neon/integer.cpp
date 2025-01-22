/*
 * Copyright (c) 2024-2025 Arm Limited.
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
#include "arm_compute/function_info/ScatterInfo.h"

#include "src/cpu/kernels/scatter/generic/neon/impl.h"

namespace arm_compute
{
namespace cpu
{
template <typename ScalarType>
void scatter_integer_generic(const ITensor         *src,
                             const ITensor         *indices,
                             ITensor               *dst,
                             const ScatterFunction &scatter_func,
                             const Window          &window,
                             const int              data_block_length)
{
    switch (scatter_func)
    {
        case ScatterFunction::Update:
            scatter_neon<ScatterFunction::Update, ScalarType>(src, indices, dst, window, data_block_length);
            break;
        case ScatterFunction::Add:
            scatter_neon<ScatterFunction::Add, ScalarType>(src, indices, dst, window, data_block_length);
            break;
        case ScatterFunction::Sub:
            scatter_neon<ScatterFunction::Sub, ScalarType>(src, indices, dst, window, data_block_length);
            break;
        case ScatterFunction::Max:
            scatter_neon<ScatterFunction::Max, ScalarType>(src, indices, dst, window, data_block_length);
            break;
        case ScatterFunction::Min:
            scatter_neon<ScatterFunction::Min, ScalarType>(src, indices, dst, window, data_block_length);
            break;
        default:
            ARM_COMPUTE_ERROR("Invalid reduction function for scatter.");
    }
    return;
}

void scatter_s32_neon(const ITensor         *src,
                      const ITensor         *indices,
                      ITensor               *dst,
                      const ScatterFunction &scatter_func,
                      const Window          &window,
                      const int              data_block_length)
{
    scatter_integer_generic<int32_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}

void scatter_s16_neon(const ITensor         *src,
                      const ITensor         *indices,
                      ITensor               *dst,
                      const ScatterFunction &scatter_func,
                      const Window          &window,
                      const int              data_block_length)
{
    scatter_integer_generic<int16_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}

void scatter_s8_neon(const ITensor         *src,
                     const ITensor         *indices,
                     ITensor               *dst,
                     const ScatterFunction &scatter_func,
                     const Window          &window,
                     const int              data_block_length)
{
    scatter_integer_generic<int8_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}

void scatter_u32_neon(const ITensor         *src,
                      const ITensor         *indices,
                      ITensor               *dst,
                      const ScatterFunction &scatter_func,
                      const Window          &window,
                      const int              data_block_length)
{
    scatter_integer_generic<uint32_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}

void scatter_u16_neon(const ITensor         *src,
                      const ITensor         *indices,
                      ITensor               *dst,
                      const ScatterFunction &scatter_func,
                      const Window          &window,
                      const int              data_block_length)
{
    scatter_integer_generic<uint16_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}

void scatter_u8_neon(const ITensor         *src,
                     const ITensor         *indices,
                     ITensor               *dst,
                     const ScatterFunction &scatter_func,
                     const Window          &window,
                     const int              data_block_length)
{
    scatter_integer_generic<uint8_t>(src, indices, dst, scatter_func, window, data_block_length);
    return;
}
} // namespace cpu
} // namespace arm_compute
