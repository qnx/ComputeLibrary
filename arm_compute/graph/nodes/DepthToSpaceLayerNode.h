/*
 * Copyright (c) 2020, 2025 Arm Limited.
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
#ifndef ACL_ARM_COMPUTE_GRAPH_NODES_DEPTHTOSPACELAYERNODE_H
#define ACL_ARM_COMPUTE_GRAPH_NODES_DEPTHTOSPACELAYERNODE_H

/** @file
 * @publicapi
 */

#include "arm_compute/graph/INode.h"

namespace arm_compute
{
namespace graph
{
/** DepthToSpace Layer node */
class DepthToSpaceLayerNode final : public INode
{
public:
    /** Default Constructor */
    DepthToSpaceLayerNode(int block_shape);
    /** Block shape policy accessor
     *
     * @return Block shape
     */
    int block_shape() const;
    /** Computes depth to space output descriptor
     *
     * @warning block_shape must be greater than or equal to 2
     *
     * @param[in] input_descriptor Input descriptor
     * @param[in] block_shape      Number of output neurons
     *
     * @return Output descriptor
     */
    static TensorDescriptor compute_output_descriptor(const TensorDescriptor &input_descriptor, int block_shape);

    // Inherited overridden methods:
    NodeType         type() const override;
    bool             forward_descriptors() override;
    TensorDescriptor configure_output(size_t idx) const override;
    void             accept(INodeVisitor &v) override;

private:
    int _block_shape;
};
} // namespace graph
} // namespace arm_compute
#endif // ACL_ARM_COMPUTE_GRAPH_NODES_DEPTHTOSPACELAYERNODE_H
