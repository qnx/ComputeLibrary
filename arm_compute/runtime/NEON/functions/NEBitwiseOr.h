/*
 * Copyright (c) 2017-2021, 2024-2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NEBITWISEOR_H
#define ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NEBITWISEOR_H

/** @file
 * @publicapi
 */

#include "arm_compute/runtime/NEON/INESimpleFunctionNoBorder.h"

namespace arm_compute
{
class ITensor;

/** Basic function to run NEBitwiseOrKernel */
class NEBitwiseOr : public INESimpleFunctionNoBorder
{
public:
    /** Initialise the kernel's inputs and output
     *
     * Valid data layouts:
     * - All
     *
     * Valid data type configurations:
     * |src            |dst            |
     * |:--------------|:--------------|
     * |U8             |U8             |
     *
     * @param[in]  input1 First tensor input. Data type supported: U8.
     * @param[in]  input2 Second tensor input. Data type supported: U8.
     * @param[out] output Output tensor. Data type supported: U8.
     */
    void configure(const ITensor *input1, const ITensor *input2, ITensor *output);
};
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NEBITWISEOR_H
