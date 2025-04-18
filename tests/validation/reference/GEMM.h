/*
 * Copyright (c) 2017-2019, 2024-2025 Arm Limited.
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
#ifndef ACL_TESTS_VALIDATION_REFERENCE_GEMM_H
#define ACL_TESTS_VALIDATION_REFERENCE_GEMM_H

#include "tests/SimpleTensor.h"
#include "tests/validation/Helpers.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace reference
{
template <typename T, typename std::enable_if<is_floating_point<T>::value, int>::type = 0>
SimpleTensor<T> gemm(const SimpleTensor<T> &a, const SimpleTensor<T> &b, const SimpleTensor<T> &c, float alpha, float beta, bool fast_math = false);

template <typename T, typename Tout = T, typename std::enable_if<is_floating_point<T>::value, int>::type = 0>
SimpleTensor<Tout> gemm_mixed_precision(const SimpleTensor<T> &a, const SimpleTensor<T> &b, const SimpleTensor<T> &c, float alpha, float beta);

template <typename T, typename std::enable_if<is_floating_point<T>::value, int>::type = 0>
void gemm_accumulate(const SimpleTensor<T> &a, const SimpleTensor<T> &b, const SimpleTensor<T> &c, float alpha, float beta, SimpleTensor<T> &dst);

} // namespace reference
} // namespace validation
} // namespace test
} // namespace arm_compute
#endif // ACL_TESTS_VALIDATION_REFERENCE_GEMM_H
