/*
 * Copyright (c) 2017-2021, 2023-2024 Arm Limited.
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
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEPoolingLayer.h"
#include "arm_compute/runtime/Tensor.h"
#include "tests/NEON/Accessor.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets/PoolingLayerDataset.h"
#include "tests/datasets/PoolingTypesDataset.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/PoolingLayerFixture.h"
namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
/** Input data sets for float data types */

const auto PoolingLayerDatasetFP = combine(combine(combine(datasets::PoolingTypes(), framework::dataset::make("PoolingSize", { Size2D(2, 2), Size2D(3, 3), Size2D(7, 7), Size2D(3, 7), Size2D(7, 8) })),
                                                   framework::dataset::make("PadStride", { PadStrideInfo(1, 1, 0, 0), PadStrideInfo(1, 2, 1, 1), PadStrideInfo(2, 2, 1, 0) })),
                                           framework::dataset::make("ExcludePadding", { true, false }));
const auto PoolingLayerDatasetFPSmall = combine(combine(combine(datasets::PoolingTypes(), framework::dataset::make("PoolingSize", { Size2D(2, 2), Size2D(3, 3) })),
                                                        framework::dataset::make("PadStride", { PadStrideInfo(1, 1, 0, 0), PadStrideInfo(2, 1, 0, 0) })),
                                                framework::dataset::make("ExcludePadding", { true, false }));

/** Input data sets for asymmetric data type */

const auto PoolingLayerDatasetQASYMM8Small = combine(combine(combine(framework::dataset::make("PoolingType", { PoolingType::MAX, PoolingType::AVG }), framework::dataset::make("PoolingSize", { Size2D(2, 2), Size2D(3, 3), Size2D(3, 7), Size2D(7, 7) })),
                                                             framework::dataset::make("PadStride", { PadStrideInfo(1, 1, 0, 0), PadStrideInfo(1, 2, 1, 1) })),
                                                     framework::dataset::make("ExcludePadding", { true }));

constexpr AbsoluteTolerance<float> tolerance_f32(0.001f); /**< Tolerance value for comparing reference's output against implementation's output for float types */
#ifdef ARM_COMPUTE_ENABLE_FP16
constexpr AbsoluteTolerance<float> tolerance_f16(0.01f);     /**< Tolerance value for comparing reference's output against implementation's output for float types */
#endif                                                       /* ARM_COMPUTE_ENABLE_FP16 */
constexpr AbsoluteTolerance<uint8_t> tolerance_qasymm8(1);   /**< Tolerance value for comparing reference's output against implementation's output for unsigned 8-bit asymmetric type */
constexpr AbsoluteTolerance<int8_t>  tolerance_qasymm8_s(1); /**< Tolerance value for comparing reference's output against implementation's output for signed 8-bit asymmetric type */
const auto                           pool_data_layout_dataset = framework::dataset::make("DataLayout", { DataLayout::NCHW, DataLayout::NHWC });

const auto qasymm8_in_qinfo_dataset  = framework::dataset::make("InputQuantInfo", { QuantizationInfo(.2f, 10) });
const auto qasymm8_out_qinfo_dataset = framework::dataset::make("OutputQuantInfo",
{
    QuantizationInfo(.2f, 10), // Same qinfo
    QuantizationInfo(.1f, 5),  // Multiplier <= 1
    QuantizationInfo(2.f, 3)   // Multiplier > 1
});

const auto qasymm8_signed_in_qinfo_dataset  = framework::dataset::make("InputQuantInfo", { QuantizationInfo(.2f, -10) });
const auto qasymm8_signed_out_qinfo_dataset = framework::dataset::make("OutputQuantInfo",
{
    QuantizationInfo(.2f, -10), // Same qinfo
    QuantizationInfo(.1f, -5),  // Multiplier <= 1
    QuantizationInfo(2.f, -3)   // Multiplier > 1
});

// Cases where pooling region is completely outside the input tensor (excluding global pooling)
const auto pool_outside_input_dataset = zip(zip(zip(zip(
                                                        framework::dataset::make("Shape", { TensorShape{ 2U, 2U, 1U }, TensorShape{ 2U, 2U, 4U }, TensorShape{ 3U, 5U, 2U }, TensorShape{ 10U, 20U, 3U } }),
                                                        framework::dataset::make("PoolingType", { PoolingType::MAX, PoolingType::AVG, PoolingType::L2, PoolingType::MAX })),
                                                    framework::dataset::make("PoolingSize", { Size2D{ 2, 2 }, Size2D{ 3, 3 }, Size2D{ 2, 2 }, Size2D{ 3, 6 } })),
                                                framework::dataset::make("PadStride", { PadStrideInfo{ 1, 1, 2, 2 }, PadStrideInfo{ 1, 1, 4, 4 }, PadStrideInfo{ 1, 1, 3, 3 }, PadStrideInfo{ 1, 1, 2, 5 } })),
                                            framework::dataset::make("ExcludePadding", { false, false, false, false }));
} // namespace

TEST_SUITE(NEON)
TEST_SUITE(PoolingLayer)

// *INDENT-OFF*
// clang-format off
DATA_TEST_CASE(Validate, framework::DatasetMode::ALL, zip(zip(zip(
    framework::dataset::make("InputInfo", { TensorInfo(TensorShape(27U, 13U, 2U), 1, DataType::F32),     // Mismatching data type
                                            TensorInfo(TensorShape(27U, 13U, 2U), 1, DataType::F32),     // Window shrink
                                            TensorInfo(TensorShape(27U, 13U, 2U), 1, DataType::F32),     // Invalid pad/size combination
                                            TensorInfo(TensorShape(27U, 13U, 2U), 1, DataType::F32),     // Invalid pad/size combination
                                            TensorInfo(TensorShape(15U, 13U, 5U), 1, DataType::F32),     // Non-rectangular Global Pooling
                                            TensorInfo(TensorShape(13U, 13U, 5U), 1, DataType::F32),     // Invalid output Global Pooling
                                            TensorInfo(TensorShape(13U, 13U, 5U), 1, DataType::QASYMM8), // Invalid exclude_padding = false with quantized type, no actual padding and NHWC
                                            TensorInfo(TensorShape(13U, 13U, 5U), 1, DataType::F32),
                                            TensorInfo(TensorShape(1U, 16U, 1U),  1, DataType::F32),
                                            TensorInfo(TensorShape(112, 112, 64,1), 1, DataType::F32, DataLayout::NHWC), // Mismatching number of channels
                                            TensorInfo(TensorShape(112, 112, 64,1), 1, DataType::F32, DataLayout::NHWC), // Mismatching width
                                         }),
    framework::dataset::make("OutputInfo",{ TensorInfo(TensorShape(25U, 11U, 2U), 1, DataType::F16),
                                            TensorInfo(TensorShape(25U, 10U, 2U), 1, DataType::F32),
                                            TensorInfo(TensorShape(30U, 11U, 2U), 1, DataType::F32),
                                            TensorInfo(TensorShape(25U, 16U, 2U), 1, DataType::F32),
                                            TensorInfo(TensorShape(1U, 1U, 5U), 1, DataType::F32),
                                            TensorInfo(TensorShape(2U, 2U, 5U), 1, DataType::F32),
                                            TensorInfo(TensorShape(12U, 12U, 5U), 1, DataType::QASYMM8),
                                            TensorInfo(TensorShape(25U, 11U, 2U), 1, DataType::F32),
                                            TensorInfo(TensorShape(1U, 15U, 1U), 1, DataType::F32),
                                            TensorInfo(TensorShape(56, 56, 64,1), 1, DataType::F32, DataLayout::NHWC),
                                            TensorInfo(TensorShape(56, 51, 64,1), 1, DataType::F32, DataLayout::NHWC),

                                           })),
    framework::dataset::make("PoolInfo",  { PoolingLayerInfo(PoolingType::AVG, 3, DataLayout::NCHW, PadStrideInfo(1, 1, 0, 0)),
                                            PoolingLayerInfo(PoolingType::AVG, 3, DataLayout::NCHW, PadStrideInfo(1, 1, 0, 0)),
                                            PoolingLayerInfo(PoolingType::AVG, 2, DataLayout::NCHW, PadStrideInfo(1, 1, 2, 0)),
                                            PoolingLayerInfo(PoolingType::AVG, 2, DataLayout::NCHW, PadStrideInfo(1, 1, 0, 2)),
                                            PoolingLayerInfo(PoolingType::AVG, DataLayout::NCHW),
                                            PoolingLayerInfo(PoolingType::MAX, DataLayout::NCHW),
                                            PoolingLayerInfo(PoolingType::AVG, 2, DataLayout::NHWC, PadStrideInfo(), false),
                                            PoolingLayerInfo(PoolingType::AVG, DataLayout::NCHW),
                                            PoolingLayerInfo(PoolingType::MAX, 2, DataLayout::NHWC, PadStrideInfo(1, 1, 0, 0), false),
                                            PoolingLayerInfo(PoolingType::MAX,3,DataLayout::NHWC,PadStrideInfo(2,2,1,1)),
                                            PoolingLayerInfo(PoolingType::MAX,3,DataLayout::NHWC,PadStrideInfo(2,2,1,1)),

                                           })),
    framework::dataset::make("Expected", { false, false, false, false, true, false, true, false, false, false, false})),
    input_info, output_info, pool_info, expected)
{
    bool is_valid = bool(NEPoolingLayer::validate(&input_info.clone()->set_is_resizable(false), &output_info.clone()->set_is_resizable(false), pool_info));
    ARM_COMPUTE_EXPECT(is_valid == expected, framework::LogLevel::ERRORS);
}
// clang-format on
// *INDENT-ON*

template <typename T>
using NEPoolingLayerIndicesFixture = PoolingLayerIndicesValidationFixture<Tensor, Accessor, NEPoolingLayer, T>;

template <typename T>
using NEPoolingLayerFixture = PoolingLayerValidationFixture<Tensor, Accessor, NEPoolingLayer, T>;
template <typename T>
using NEPoolingLayerMixedDataLayoutFixture = PoolingLayerValidationFixture<Tensor, Accessor, NEPoolingLayer, T, true>;

template <typename T>
using NESpecialPoolingLayerFixture = SpecialPoolingLayerValidationFixture<Tensor, Accessor, NEPoolingLayer, T>;

const auto PoolingLayerIndicesDatasetFPSmall = combine(combine(combine(framework::dataset::make("PoolType", { PoolingType::MAX }), framework::dataset::make("PoolingSize", { Size2D(2, 2) })),
                                                               framework::dataset::make("PadStride", { PadStrideInfo(1, 1, 0, 0), PadStrideInfo(2, 1, 0, 0) })),
                                                       framework::dataset::make("ExcludePadding", { true, false }));
const auto PoolingLayerKernelIndicesDatasetFPSmall = combine(combine(combine(framework::dataset::make("PoolType", { PoolingType::MAX }), framework::dataset::make("PoolingSize", { Size2D(2, 2), Size2D(3, 3), Size2D(7, 7) })),
                                                                     framework::dataset::make("PadStride", { PadStrideInfo(1, 1, 0, 0), PadStrideInfo(2, 1, 0, 0), PadStrideInfo(1, 1, 1, 1) })),
                                                             framework::dataset::make("ExcludePadding", { false }));

TEST_CASE(SimpleIntegerAvgPooling, framework::DatasetMode::ALL)
{
    const auto pool_info = PoolingLayerInfo(PoolingType::AVG, Size2D(1,1), DataLayout::NHWC);
    const auto shape = TensorShape(18U,1U,1U); // > 16 for channel dim. to stress vector and leftover loops
    const auto dtype = DataType::QASYMM8_SIGNED;
    const auto layout = DataLayout::NHWC;
    const auto qinfo = QuantizationInfo(1.f, 0);

    Tensor input = create_tensor<Tensor>(shape, dtype, 1, qinfo, layout);
    Tensor output = create_tensor<Tensor>(shape, dtype, 1, qinfo, layout);

    NEPoolingLayer pool;
    pool.configure(&input, &output, pool_info);

    input.allocator()->allocate();
    output.allocator()->allocate();

    std::vector<int8_t> values = {-9, -8, -7, -6, -5, -4, -3, -2, -1,
                                   0, 1, 2, 3, 4, 5, 6, 7, 8};

    ARM_COMPUTE_EXPECT(values.size() == shape.x(), framework::LogLevel::ERRORS);

    library->fill_static_values(Accessor(input), values);
    pool.run();

    for(unsigned int i = 0; i < values.size(); ++i)
    {
        const int8_t ref = values[i];
        const int8_t target = reinterpret_cast<int8_t *>(output.buffer())[i];
        ARM_COMPUTE_EXPECT(ref == target, framework::LogLevel::ERRORS);
    }
}

TEST_SUITE(Float)
TEST_SUITE(FP32)
FIXTURE_DATA_TEST_CASE(RunIndices, NEPoolingLayerIndicesFixture<float>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(datasets::SmallNoneUnitShapes(),
                                                                                                                   combine(PoolingLayerIndicesDatasetFPSmall,
                                                                                                                           framework::dataset::make("DataType", DataType::F32))),
                                                                                                                   framework::dataset::make("DataLayout", { DataLayout::NCHW, DataLayout::NHWC })),
                                                                                                                   framework::dataset::make("UseKernelIndices", { false })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
    validate(Accessor(_target_indices), _ref_indices);
}
FIXTURE_DATA_TEST_CASE(RunKernelIndices, NEPoolingLayerIndicesFixture<float>, framework::DatasetMode::ALL, combine(combine(combine(datasets::SmallNoneUnitShapes(),
                                                                                                                   combine(PoolingLayerKernelIndicesDatasetFPSmall,
                                                                                                                           framework::dataset::make("DataType", DataType::F32))),
                                                                                                                   framework::dataset::make("DataLayout", { DataLayout::NHWC })),
                                                                                                                   framework::dataset::make("UseKernelIndices", { true })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
    validate(Accessor(_target_indices), _ref_indices);
}
FIXTURE_DATA_TEST_CASE(RunSpecial, NESpecialPoolingLayerFixture<float>, framework::DatasetMode::ALL, datasets::PoolingLayerDatasetSpecial() * framework::dataset::make("DataType", DataType::F32))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
FIXTURE_DATA_TEST_CASE(RunSmall, NEPoolingLayerFixture<float>, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallNoneUnitShapes(), combine(PoolingLayerDatasetFPSmall,
                                                                                                                  framework::dataset::make("DataType",
                                                                                                                          DataType::F32))),
                                                                                                          pool_data_layout_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
FIXTURE_DATA_TEST_CASE(RunMixedDataLayout, NEPoolingLayerMixedDataLayoutFixture<float>, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallNoneUnitShapes(),
                       combine(combine(combine(combine(datasets::PoolingTypes(),
                                                       framework::dataset::make("PoolingSize", { Size2D(2, 2) })),
                                               framework::dataset::make("PadStride", { PadStrideInfo(2, 1, 0, 0) })),
                                       framework::dataset::make("ExcludePadding", { false })),
                               framework::dataset::make("DataType", DataType::F32))),
                       pool_data_layout_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEPoolingLayerFixture<float>, framework::DatasetMode::NIGHTLY, combine(combine(datasets::LargeShapes(), combine(PoolingLayerDatasetFP,
                                                                                                                framework::dataset::make("DataType",
                                                                                                                        DataType::F32))),
                                                                                                        pool_data_layout_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
TEST_SUITE(CornerCases)
FIXTURE_DATA_TEST_CASE(PoolRegionCompletelyOutsideInput, NEPoolingLayerFixture<float>, framework::DatasetMode::PRECOMMIT, combine(combine(pool_outside_input_dataset,
                       framework::dataset::make("DataType",
                                                DataType::F32)),
                       pool_data_layout_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
TEST_SUITE_END() // CornerCases
TEST_SUITE_END() // FP32

#ifdef ARM_COMPUTE_ENABLE_FP16
TEST_SUITE(FP16)
FIXTURE_DATA_TEST_CASE(RunIndices, NEPoolingLayerIndicesFixture<half>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(datasets::SmallNoneUnitShapes(),
                                                                                                                  combine(PoolingLayerIndicesDatasetFPSmall,
                                                                                                                          framework::dataset::make("DataType",
                                                                                                                                  DataType::F16))),
                                                                                                                  framework::dataset::make("DataLayout", { DataLayout::NCHW, DataLayout::NHWC })),
                                                                                                                  framework::dataset::make("UseKernelIndices", { false })))
{
    if(CPUInfo::get().has_fp16())
    {
        // Validate output
        validate(Accessor(_target), _reference, tolerance_f16);
        validate(Accessor(_target_indices), _ref_indices);
    }
    else
    {
        ARM_COMPUTE_TEST_INFO("Device does not support fp16 vector operations. Test SKIPPED.");
        framework::ARM_COMPUTE_PRINT_INFO();
    }
}
FIXTURE_DATA_TEST_CASE(RunSmall, NEPoolingLayerFixture<half>, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallNoneUnitShapes(), combine(PoolingLayerDatasetFPSmall,
                                                                                                                 framework::dataset::make("DataType", DataType::F16))),
                                                                                                         pool_data_layout_dataset))
{
    if(CPUInfo::get().has_fp16())
    {
        // Validate output
        validate(Accessor(_target), _reference, tolerance_f16);
    }
    else
    {
        ARM_COMPUTE_TEST_INFO("Device does not support fp16 vector operations. Test SKIPPED.");
        framework::ARM_COMPUTE_PRINT_INFO();
    }
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEPoolingLayerFixture<half>, framework::DatasetMode::NIGHTLY, combine(combine(datasets::LargeShapes(), combine(PoolingLayerDatasetFP,
                                                                                                               framework::dataset::make("DataType", DataType::F16))),
                                                                                                       pool_data_layout_dataset))
{
    if(CPUInfo::get().has_fp16())
    {
        // Validate output
        validate(Accessor(_target), _reference, tolerance_f16);
    }
    else
    {
        ARM_COMPUTE_TEST_INFO("Device does not support fp16 vector operations. Test SKIPPED.");
        framework::ARM_COMPUTE_PRINT_INFO();
    }
}
TEST_SUITE(CornerCases)
FIXTURE_DATA_TEST_CASE(PoolRegionCompletelyOutsideInput, NEPoolingLayerFixture<half>, framework::DatasetMode::PRECOMMIT, combine(combine(pool_outside_input_dataset,
                       framework::dataset::make("DataType",
                                                DataType::F16)),
                       pool_data_layout_dataset))
{
    if(CPUInfo::get().has_fp16())
    {
        // Validate output
        validate(Accessor(_target), _reference, tolerance_f16);
    }
    else
    {
        ARM_COMPUTE_TEST_INFO("Device does not support fp16 vector operations. Test SKIPPED.");
        framework::ARM_COMPUTE_PRINT_INFO();
    }
}
TEST_SUITE_END() // CornerCases
TEST_SUITE_END() // FP16
#endif           /* ARM_COMPUTE_ENABLE_FP16 */
TEST_SUITE_END() // Float

TEST_SUITE(Quantized)

template <typename T>
using NEPoolingLayerQuantizedFixture = PoolingLayerValidationQuantizedFixture<Tensor, Accessor, NEPoolingLayer, T>;
template <typename T>
using NEPoolingLayerQuantizedMixedDataLayoutFixture = PoolingLayerValidationQuantizedFixture<Tensor, Accessor, NEPoolingLayer, T, true>;

TEST_SUITE(QASYMM8)
FIXTURE_DATA_TEST_CASE(RunSmallNCHW, NEPoolingLayerQuantizedFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(datasets::SmallNoneUnitShapes(),
                       combine(PoolingLayerDatasetQASYMM8Small,
                               framework::dataset::make("DataType", DataType::QASYMM8))),
                       framework::dataset::make("DataLayout", { DataLayout::NCHW })),
                       qasymm8_in_qinfo_dataset),
                       qasymm8_in_qinfo_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_qasymm8);
}
FIXTURE_DATA_TEST_CASE(RunSmall, NEPoolingLayerQuantizedFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(datasets::SmallNoneUnitShapes(),
                                                                                                                     combine(PoolingLayerDatasetQASYMM8Small,
                                                                                                                             framework::dataset::make("DataType", DataType::QASYMM8))),
                                                                                                                     framework::dataset::make("DataLayout", { DataLayout::NHWC })),
                                                                                                                     qasymm8_in_qinfo_dataset),
                                                                                                                     qasymm8_out_qinfo_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_qasymm8);
}
FIXTURE_DATA_TEST_CASE(RunMixedDataLayout, NEPoolingLayerQuantizedMixedDataLayoutFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(datasets::SmallNoneUnitShapes(),
                       combine(combine(combine(combine(framework::dataset::make("PoolingType", { PoolingType::MAX, PoolingType::AVG }),
                                                       framework::dataset::make("PoolingSize", { Size2D(2, 2) })),
                                               framework::dataset::make("PadStride", { PadStrideInfo(1, 2, 1, 1) })),
                                       framework::dataset::make("ExcludePadding", { true })),
                               framework::dataset::make("DataType", DataType::QASYMM8))),
                       framework::dataset::make("DataLayout", { DataLayout::NHWC, DataLayout::NCHW })),
                       framework::dataset::make("InputQuantInfo", { QuantizationInfo(1.f / 255.f, 10) })),
                       framework::dataset::make("OutputQuantInfo", { QuantizationInfo(1.f / 255.f, 5) })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_qasymm8);
}
TEST_SUITE_END() // QASYMM8
TEST_SUITE(QASYMM8_SIGNED)
FIXTURE_DATA_TEST_CASE(RunSmall, NEPoolingLayerQuantizedFixture<int8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(datasets::SmallNoneUnitShapes(),
                                                                                                                    combine(PoolingLayerDatasetQASYMM8Small,
                                                                                                                            framework::dataset::make("DataType", DataType::QASYMM8_SIGNED))),
                                                                                                                    framework::dataset::make("DataLayout", { DataLayout::NCHW, DataLayout::NHWC })),
                                                                                                                    qasymm8_signed_in_qinfo_dataset),
                                                                                                                    qasymm8_signed_in_qinfo_dataset))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_qasymm8_s);
}
FIXTURE_DATA_TEST_CASE(RunMixedDataLayout, NEPoolingLayerQuantizedMixedDataLayoutFixture<int8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(datasets::SmallNoneUnitShapes(),
                       combine(combine(combine(combine(framework::dataset::make("PoolingType", { PoolingType::MAX, PoolingType::AVG }),
                                                       framework::dataset::make("PoolingSize", { Size2D(2, 2) })),
                                               framework::dataset::make("PadStride", { PadStrideInfo(1, 2, 1, 1) })),
                                       framework::dataset::make("ExcludePadding", { true })),
                               framework::dataset::make("DataType", DataType::QASYMM8_SIGNED))),
                       framework::dataset::make("DataLayout", { DataLayout::NHWC, DataLayout::NCHW })),
                       framework::dataset::make("InputQuantInfo", { QuantizationInfo(1.f / 127.f, -10) })),
                       framework::dataset::make("OutputQuantInfo", { QuantizationInfo(1.f / 127.f, -10) })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_qasymm8_s);
}
TEST_SUITE_END() // QASYMM8_SIGNED
TEST_SUITE_END() // Quantized
TEST_SUITE_END() // PoolingLayer
TEST_SUITE_END() // Neon
} // namespace validation
} // namespace test
} // namespace arm_compute
