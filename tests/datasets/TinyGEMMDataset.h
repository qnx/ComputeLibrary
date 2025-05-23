/*
 * Copyright (c) 2017-2018, 2025 Arm Limited.
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
#ifndef ACL_TESTS_DATASETS_TINYGEMMDATASET_H
#define ACL_TESTS_DATASETS_TINYGEMMDATASET_H

#include "tests/datasets/GEMMDataset.h"

namespace arm_compute
{
namespace test
{
namespace datasets
{
class TinyGEMMDataset final : public GEMMDataset
{
public:
    TinyGEMMDataset()
    {
        add_config(1, 23, 31, 1.0f, 0.0f);
        add_config(13, 33, 21, 1.0f, 0.0f);
    }
};
} // namespace datasets
} // namespace test
} // namespace arm_compute
#endif // ACL_TESTS_DATASETS_TINYGEMMDATASET_H
