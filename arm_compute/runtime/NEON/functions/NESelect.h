/*
 * Copyright (c) 2018-2021, 2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NESELECT_H
#define ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NESELECT_H

/** @file
 * @publicapi
 */

#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/INESimpleFunctionNoBorder.h"

namespace arm_compute
{
// Forward declarations
class ITensor;
class ITensorInfo;

/** Basic function to run @ref NESelect */
class NESelect : public INESimpleFunctionNoBorder
{
public:
    /** Initialise the kernel's inputs and output.
     *
     * Valid data layouts:
     * - All
     *
     * Valid data type configurations:
     * |src0           |src1           |src2   |dst            |
     * |:--------------|:--------------|:------|:--------------|
     * |U8             |All            |All    |All            |
     *
     * @param[in]  c      Condition input tensor. Data types supported: U8.
     * @param[in]  x      First input tensor. Data types supported: All.
     * @param[in]  y      Second input tensor. Data types supported: Same as @p x
     * @param[out] output Output tensor. Data types supported: Same as @p x.
     */
    void configure(const ITensor *c, const ITensor *x, const ITensor *y, ITensor *output);
    /** Static function to check if given info will lead to a valid configuration of @ref NESelect
     *
     * @param[in] c      Condition input tensor. Data types supported: U8.
     * @param[in] x      First input tensor. Data types supported: All.
     * @param[in] y      Second input tensor. Data types supported: Same as @p x
     * @param[in] output Output tensor. Data types supported: Same as @p x.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *c, const ITensorInfo *x, const ITensorInfo *y, const ITensorInfo *output);
};
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_RUNTIME_NEON_FUNCTIONS_NESELECT_H
