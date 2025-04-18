/*
 * Copyright (c) 2017-2021, 2023-2025 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEDeconvolutionLayer.h"

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/Utils.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/NEON/NEScheduler.h"

#include "src/common/utils/Log.h"
#include "src/core/helpers/AutoConfiguration.h"

using namespace arm_compute::misc::shape_calculator;

namespace arm_compute
{
namespace
{
std::pair<PadStrideInfo, bool>
compute_upsample_info(const PadStrideInfo &info, uint32_t deconv_pad_x, uint32_t deconv_pad_y)
{
    const unsigned int pad_left   = info.pad_left();
    const unsigned int pad_right  = info.pad_right();
    const unsigned int pad_top    = info.pad_top();
    const unsigned int pad_bottom = info.pad_bottom();
    const unsigned int stride_x   = info.stride().first;
    const unsigned int stride_y   = info.stride().second;

    // Find the upsampled dimensions and the padding needed for the convolution with stride 1 in order to match output shape
    unsigned int deconv_pad_left  = pad_right > pad_left ? pad_right - pad_left : 0;
    unsigned int deconv_pad_right = pad_left > pad_right ? pad_left - pad_right : 0;

    bool negative_padding = (deconv_pad_x < deconv_pad_left + deconv_pad_right);

    deconv_pad_x -= deconv_pad_left + deconv_pad_right;
    ARM_COMPUTE_ERROR_ON((deconv_pad_x % 2) != 0);
    deconv_pad_left += deconv_pad_x / 2;
    deconv_pad_right += deconv_pad_x / 2;

    unsigned int deconv_pad_top    = pad_bottom > pad_top ? pad_bottom - pad_top : 0;
    unsigned int deconv_pad_bottom = pad_top > pad_bottom ? pad_top - pad_bottom : 0;

    negative_padding = negative_padding || (deconv_pad_y < deconv_pad_top + deconv_pad_bottom);

    deconv_pad_y -= deconv_pad_top + deconv_pad_bottom;
    ARM_COMPUTE_ERROR_ON((deconv_pad_y % 2) != 0);
    deconv_pad_top += deconv_pad_y / 2;
    deconv_pad_bottom += deconv_pad_y / 2;

    return std::make_pair(PadStrideInfo(stride_x, stride_y, deconv_pad_left, deconv_pad_right, deconv_pad_top,
                                        deconv_pad_bottom, DimensionRoundingType::FLOOR),
                          negative_padding);
}
} // namespace

NEDeconvolutionLayer::NEDeconvolutionLayer(std::shared_ptr<IMemoryManager> memory_manager) // NOLINT
    : _memory_group(memory_manager),
      _conv_f(memory_manager),
      _upsample_f(),
      _flip_weights(),
      _scaled_output(),
      _weights_flipped(),
      _flip_axis(),
      _original_weights(nullptr),
      _input(nullptr),
      _info(),
      _is_prepared(false),
      _do_upsampling(true)
{
}

Status NEDeconvolutionLayer::validate(const ITensorInfo   *input,
                                      const ITensorInfo   *weights,
                                      const ITensorInfo   *bias,
                                      const ITensorInfo   *output,
                                      const PadStrideInfo &info,
                                      bool                 enable_fast_math,
                                      const WeightsInfo   &weights_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(input, weights, output);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(input, 1, DataType::F32, DataType::F16, DataType::QASYMM8,
                                                         DataType::QASYMM8_SIGNED);
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, weights, bias, output);
    const unsigned int width_idx = get_data_layout_dimension_index(weights->data_layout(), DataLayoutDimension::WIDTH);
    const unsigned int height_idx =
        get_data_layout_dimension_index(weights->data_layout(), DataLayoutDimension::HEIGHT);
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(width_idx) < 1);
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(height_idx) < 1);
    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_LAYOUT(weights, input);
    if (is_data_type_quantized_per_channel(weights->data_type()) && is_data_type_quantized(input->data_type()))
    {
        ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(weights, 1, DataType::QSYMM8_PER_CHANNEL);
    }
    else
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, weights);
    }

    const unsigned int pad_left   = info.pad_left();
    const unsigned int pad_top    = info.pad_top();
    const unsigned int pad_right  = info.pad_right();
    const unsigned int pad_bottom = info.pad_bottom();

    ARM_COMPUTE_RETURN_ERROR_ON(((input->dimension(width_idx) - 1) * info.stride().first +
                                 weights->dimension(width_idx)) < (pad_left + pad_right));
    ARM_COMPUTE_RETURN_ERROR_ON(((input->dimension(height_idx) - 1) * info.stride().second +
                                 weights->dimension(height_idx)) < (pad_top + pad_bottom));

    auto out_dims =
        deconvolution_output_dimensions(input->dimension(width_idx), input->dimension(height_idx),
                                        weights->dimension(width_idx), weights->dimension(height_idx), info);

    if (bias != nullptr)
    {
        if (is_data_type_quantized_asymmetric(input->data_type()))
        {
            ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(bias, 1, DataType::S32);
        }
        else
        {
            ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, bias);
        }
    }

    if (output->tensor_shape().total_size() > 0)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, output);

        const TensorShape output_shape = compute_deconvolution_output_shape(out_dims, *input, *weights);

        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimX) != output_shape.x(),
                                        "Output's width is invalid.");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimY) != output_shape.y(),
                                        "Output's height is invalid.");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimZ) != output_shape.z(),
                                        "Output's depth is invalid.");
    }

    uint32_t       deconv_pad_x   = 0;
    uint32_t       deconv_pad_y   = 0;
    const uint32_t stride_x       = info.stride().first;
    const uint32_t stride_y       = info.stride().second;
    const auto     deconv_padding = compute_deconvolution_padding(*input, *weights, static_cast<int32_t>(stride_x),
                                                                  static_cast<int32_t>(stride_y), out_dims);
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(deconv_padding.first < 0 || deconv_padding.second < 0,
                                    "Negative padding not supported");

    const TensorShape scale_out_shape = compute_deconvolution_upsampled_shape(*input, *weights, stride_x, stride_y,
                                                                              out_dims, deconv_pad_x, deconv_pad_y);
    TensorInfo scale_out_info(input->clone()->set_is_resizable(true).reset_padding().set_tensor_shape(scale_out_shape));

    PadStrideInfo upsample_info;
    bool          negative_padding;
    std::tie(upsample_info, negative_padding) = compute_upsample_info(info, deconv_pad_x, deconv_pad_y);

    ARM_COMPUTE_RETURN_ERROR_ON(negative_padding);

    // Do not perform upsampling when the operation uses unit stride in all dimensions
    const bool do_upsampling = stride_x != 1 || stride_y != 1;

    const unsigned int batches_idx =
        get_data_layout_dimension_index(weights->data_layout(), DataLayoutDimension::BATCHES);
    const unsigned int channel_idx =
        get_data_layout_dimension_index(weights->data_layout(), DataLayoutDimension::CHANNEL);
    ARM_COMPUTE_RETURN_ERROR_ON(input->dimension(batches_idx) != scale_out_info.dimension(batches_idx));
    ARM_COMPUTE_RETURN_ERROR_ON(input->dimension(channel_idx) != scale_out_info.dimension(channel_idx));

    if (do_upsampling)
    {
        const PadStrideInfo conv_info(1, 1, 0, 0, 0, 0, DimensionRoundingType::CEIL);
        ARM_COMPUTE_RETURN_ON_ERROR(NEConvolutionLayer::validate(&scale_out_info, weights, bias, output, conv_info,
                                                                 weights_info, Size2D(1U, 1U), ActivationLayerInfo(),
                                                                 enable_fast_math));
    }
    else
    {
        const PadStrideInfo conv_info(1, 1, upsample_info.pad_left(), upsample_info.pad_right(),
                                      upsample_info.pad_top(), upsample_info.pad_bottom(), DimensionRoundingType::CEIL);
        ARM_COMPUTE_RETURN_ON_ERROR(NEConvolutionLayer::validate(input, weights, bias, output, conv_info, weights_info,
                                                                 Size2D(1U, 1U), ActivationLayerInfo(),
                                                                 enable_fast_math));
    }

    return Status{};
}

void NEDeconvolutionLayer::configure(ITensor             *input,
                                     const ITensor       *weights,
                                     const ITensor       *bias,
                                     ITensor             *output,
                                     const PadStrideInfo &info,
                                     bool                 enable_fast_math,
                                     const WeightsInfo   &weights_info)
{
    // Perform validation step
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, weights, output);
    ARM_COMPUTE_ERROR_THROW_ON(NEDeconvolutionLayer::validate(input->info(), weights->info(),
                                                              (bias == nullptr) ? nullptr : bias->info(),
                                                              output->info(), info, enable_fast_math, weights_info));
    ARM_COMPUTE_LOG_PARAMS(input, weights, bias, output, info, enable_fast_math, weights_info);

    const DataLayout   data_layout = input->info()->data_layout();
    const unsigned int width_idx   = get_data_layout_dimension_index(data_layout, DataLayoutDimension::WIDTH);
    const unsigned int height_idx  = get_data_layout_dimension_index(data_layout, DataLayoutDimension::HEIGHT);
    auto               out_dims    = deconvolution_output_dimensions(
                         input->info()->dimension(width_idx), input->info()->dimension(height_idx),
                         weights->info()->dimension(width_idx), weights->info()->dimension(height_idx), info);

    const TensorShape output_shape = compute_deconvolution_output_shape(out_dims, *input->info(), *weights->info());

    _input            = input;
    _original_weights = weights;
    _info             = info;
    _is_prepared      = false;

    const unsigned int stride_x = info.stride().first;
    const unsigned int stride_y = info.stride().second;

    // Output auto initialization if not yet initialized
    auto_init_if_empty(*output->info(), output_shape, 1, input->info()->data_type(),
                       input->info()->quantization_info());

    _flip_axis.allocator()->init(TensorInfo(TensorShape(2U), 1, DataType::U32));

    _weights_flipped.allocator()->init(weights->info()->clone()->set_data_layout(data_layout));
    _flip_weights.configure(weights, &_weights_flipped, &_flip_axis);

    // setup the function to convolve the upscaled output
    uint32_t          deconv_pad_x    = 0;
    uint32_t          deconv_pad_y    = 0;
    const TensorShape scale_out_shape = compute_deconvolution_upsampled_shape(
        *input->info(), *weights->info(), stride_x, stride_y, out_dims, deconv_pad_x, deconv_pad_y);

    PadStrideInfo upsample_info;
    bool          negative_padding;
    std::tie(upsample_info, negative_padding) = compute_upsample_info(info, deconv_pad_x, deconv_pad_y);

    ARM_COMPUTE_ERROR_ON(negative_padding);
    // Do not perform upsampling when the operation uses unit stride in all dimensions
    _do_upsampling = stride_x != 1 || stride_y != 1;

    // Setup flip axis data
    _flip_axis.allocator()->allocate();
    auto axis_data = reinterpret_cast<uint32_t *>(_flip_axis.buffer());
    axis_data[0]   = static_cast<uint32_t>(width_idx);
    axis_data[1]   = static_cast<uint32_t>(height_idx);

    // Setup convolution and upsampling, if needed
    if (_do_upsampling)
    {
        _memory_group.manage(&_scaled_output);

        const PadStrideInfo conv_info(1, 1, 0, 0, 0, 0, DimensionRoundingType::CEIL);
        TensorInfo scale_out_info(scale_out_shape, 1, input->info()->data_type(), input->info()->quantization_info());
        scale_out_info.set_data_layout(data_layout);
        _scaled_output.allocator()->init(scale_out_info);

        // Minor optimization: In the upsampling step, we do not need to allocate space for the padding in the upsampled image.
        // The padding amount can be given as input to the convolution layer.
        _upsample_f.configure(input, &_scaled_output, upsample_info);

        _conv_f.configure(&_scaled_output, &_weights_flipped, bias, output, conv_info, weights_info, Size2D(1U, 1U),
                          ActivationLayerInfo(), enable_fast_math);

        _scaled_output.allocator()->allocate();
    }
    else
    {
        const PadStrideInfo conv_info(1, 1, upsample_info.pad_left(), upsample_info.pad_right(),
                                      upsample_info.pad_top(), upsample_info.pad_bottom(), DimensionRoundingType::CEIL);
        _conv_f.configure(input, &_weights_flipped, bias, output, conv_info, weights_info, Size2D(1U, 1U),
                          ActivationLayerInfo(), enable_fast_math);
    }
}

void NEDeconvolutionLayer::run()
{
    prepare();

    MemoryGroupResourceScope scope_mg(_memory_group);

    if (_do_upsampling)
    {
        _upsample_f.run();
    }
    _conv_f.run();
}

void NEDeconvolutionLayer::prepare()
{
    if (!_is_prepared)
    {
        ARM_COMPUTE_ERROR_ON(!_original_weights->is_used());

        // Run weights flipping and mark original weights tensor as unused
        _weights_flipped.allocator()->allocate();
        _flip_weights.run();
        _original_weights->mark_as_unused();

        // Prepare convolution
        _conv_f.prepare();

        _is_prepared = true;
    }
}
} // namespace arm_compute
