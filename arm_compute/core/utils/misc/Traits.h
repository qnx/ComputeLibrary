/*
 * Copyright (c) 2018-2020, 2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_CORE_UTILS_MISC_TRAITS_H
#define ACL_ARM_COMPUTE_CORE_UTILS_MISC_TRAITS_H

/** @file
 * @publicapi
 */

#include "arm_compute/core/Types.h"

#include <type_traits>

namespace arm_compute
{
namespace utils
{
namespace traits
{
template <typename T>
struct is_floating_point : public std::is_floating_point<T>
{
};

template <>
struct is_floating_point<half> : public std::true_type
{
};

#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
template <>
struct is_floating_point<__fp16> : public std::true_type
{
};
#endif /* __ARM_FEATURE_FP16_VECTOR_ARITHMETIC*/
} // namespace traits
} // namespace utils
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_CORE_UTILS_MISC_TRAITS_H
