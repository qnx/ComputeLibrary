/*
 * Copyright (c) 2024 Arm Limited.
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
#pragma once
#if defined(ARM_COMPUTE_ENABLE_SME2)
#include "../std_transforms_sme.hpp"

#define ARGLIST  \
    const __fp16 *, const __fp16 *, \
    __fp16 *, size_t, size_t, \
    const __fp16 *, Activation, bool

namespace arm_gemm
{
void sme2_gemv_fp16_mla_16VL( ARGLIST );

class cls_sme2_gemv_fp16_mla_16VL
{
public:
    typedef __fp16 operand_type;
    typedef __fp16 result_type;

    typedef void (*kern_type)( ARGLIST );

    static unsigned int out_width()
    {
        return sme::get_vector_length<__fp16>() * 16;
    }

    static constexpr unsigned int k_unroll()
    {
        return 1;
    }

    static constexpr bool supports_accumulate()
    {
        return false;
    }

    static constexpr bool supports_bias()
    {
        return true;
    }

    static constexpr bool supports_activation()
    {
        return true;
    }


    StdTransformsSME<operand_type, result_type, 1, 16, 1> transforms = {};


    // Default to the generic kernel
    kern_type kernel=sme2_gemv_fp16_mla_16VL;
    cls_sme2_gemv_fp16_mla_16VL(const CPUInfo *)
    {
    }
};

} // namespace arm_gemm

#undef ARGLIST

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
