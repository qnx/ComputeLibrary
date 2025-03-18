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
#ifndef ACL_ARM_COMPUTE_RUNTIME_CL_FUNCTIONS_CLRESHAPELAYER_H
#define ACL_ARM_COMPUTE_RUNTIME_CL_FUNCTIONS_CLRESHAPELAYER_H

/** @file
 * @publicapi
 */

#include "arm_compute/runtime/CL/ICLOperator.h"
#include "arm_compute/runtime/CL/ICLSimpleFunction.h"

#include <memory>

namespace arm_compute
{
class CLCompileContext;
class ICLTensor;
class ITensorInfo;

/** Basic function to run opencl::kernels::ClReshapeKernel */
class CLReshapeLayer : public IFunction
{
public:
    /** Default Constructor */
    CLReshapeLayer();
    /** Default Destructor */
    ~CLReshapeLayer();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    CLReshapeLayer(const CLReshapeLayer &) = delete;
    /** Default move constructor */
    CLReshapeLayer(CLReshapeLayer &&);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    CLReshapeLayer &operator=(const CLReshapeLayer &) = delete;
    /** Default move assignment operator */
    CLReshapeLayer &operator=(CLReshapeLayer &&);
    /** Initialise the kernel's inputs and outputs
     *
     * Valid data layouts:
     * - All
     *
     * Valid data type configurations:
     * |src            |dst            |
     * |:--------------|:--------------|
     * |All            |All            |
     *
     * @param[in]  input  First tensor input. Data type supported: All
     * @param[out] output Output tensor. Data type supported: Same as @p input
     */
    void configure(const ICLTensor *input, ICLTensor *output);
    /** Initialise the kernel's inputs and outputs
     *
     * @param[in]  compile_context The compile context to be used.
     * @param[in]  input           First tensor input. Data type supported: All
     * @param[out] output          Output tensor. Data type supported: Same as @p input
     */
    void configure(const CLCompileContext &compile_context, const ICLTensor *input, ICLTensor *output);

    /** Static function to check if given info will lead to a valid configuration of @ref CLReshapeLayer
     *
     * @param[in] input  First tensor info. Data type supported: All
     * @param[in] output Output tensor info. Data type supported: Same as @p input
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *output);

    // Inherited methods overridden:
    void run() override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_RUNTIME_CL_FUNCTIONS_CLRESHAPELAYER_H
