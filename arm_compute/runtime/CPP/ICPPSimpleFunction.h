/*
 * Copyright (c) 2017-2019, 2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_RUNTIME_CPP_ICPPSIMPLEFUNCTION_H
#define ACL_ARM_COMPUTE_RUNTIME_CPP_ICPPSIMPLEFUNCTION_H

/** @file
 * @publicapi
 */

#include "arm_compute/core/CPP/ICPPKernel.h"
#include "arm_compute/runtime/IFunction.h"

#include <memory>

namespace arm_compute
{
/** Basic interface for functions which have a single CPP kernel */
class ICPPSimpleFunction : public IFunction
{
public:
    /** Constructor */
    ICPPSimpleFunction();

    // Inherited methods overridden:
    void run() override final;

protected:
    std::unique_ptr<ICPPKernel> _kernel; /**< Kernel to run */
};
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_RUNTIME_CPP_ICPPSIMPLEFUNCTION_H
