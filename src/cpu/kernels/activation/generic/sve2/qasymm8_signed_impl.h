/*
 * Copyright (c) 2020-2023, 2025 Arm Limited.
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

#ifndef ACL_SRC_CPU_KERNELS_ACTIVATION_GENERIC_SVE2_QASYMM8_SIGNED_IMPL_H
#define ACL_SRC_CPU_KERNELS_ACTIVATION_GENERIC_SVE2_QASYMM8_SIGNED_IMPL_H

#include "arm_compute/core/CoreTypes.h"
#include "arm_compute/function_info/ActivationLayerInfo.h"

#include "src/core/NEON/SVEAsymm.h"
#include "src/core/NEON/SVEMath.h"
#include "src/core/NEON/wrapper/wrapper.h"

#include <arm_sve.h>
#include <cmath>
#include <cstddef>

namespace arm_compute
{
namespace cpu
{
template <typename F>
void dispatch_sve2_qasymm8_signed_activation_function(ActivationLayerInfo::ActivationFunction act,
                                                      const ActivationLayerInfo              &act_info,
                                                      const UniformQuantizationInfo          &qi_in,
                                                      const UniformQuantizationInfo          &qi_out,
                                                      F                                     &&fn)
{
    const auto va              = svdup_n_s8(quantize_qasymm8_signed(act_info.a(), qi_in));
    const auto vb              = svdup_n_s8(quantize_qasymm8_signed(act_info.b(), qi_in));
    const auto const_0         = quantize_qasymm8_signed(0.f, qi_in);
    const auto vconst_0        = svdup_n_s8(const_0);
    const auto vconst_1        = svdup_n_f32(1.f);
    const auto va_f32          = svdup_n_f32(act_info.a());
    const auto vb_f32          = svdup_n_f32(act_info.b());
    const auto const_6_f32     = svdup_n_f32(6.f);
    const auto const_0_f32     = svdup_n_f32(0.f);
    const auto const_3_f32     = svdup_n_f32(3.f);
    const auto const_inv_6_f32 = svdup_n_f32(0.166666667f);

    // Initialise scale/offset for re-quantization
    bool requant = true;
    if (qi_in.scale == qi_out.scale && qi_in.offset == qi_out.offset)
    {
        requant = false;
    }
    float s  = qi_in.scale / qi_out.scale;
    float o  = -qi_in.offset * s + qi_out.offset;
    auto  vs = svdup_n_f32(s);
    auto  vo = svdup_n_f32(o);

    // Initialise scale/offset for re-quantization with int32_t
    const auto voffset_in = svdup_n_s32(qi_in.offset);
    int32_t    s_s32      = round(s * (1 << 8), arm_compute::RoundingPolicy::TO_NEAREST_EVEN);
    int32_t    o_s32      = round(o * (1 << 8), arm_compute::RoundingPolicy::TO_NEAREST_EVEN);
    const auto vs_s32     = svdup_n_s32(s_s32);
    const auto vo_s32     = svdup_n_s32(o_s32);

    // Initialise scale/offset for re-quantization for leaky relu
    int32_t    s_leaky_s32  = round(s * act_info.a() * (1 << 8), arm_compute::RoundingPolicy::TO_NEAREST_EVEN);
    int32_t    o_leaky_s32  = round((-qi_in.offset * s * act_info.a() + qi_out.offset) * (1 << 8),
                                    arm_compute::RoundingPolicy::TO_NEAREST_EVEN);
    const auto vs_leaky_s32 = svdup_n_s32(s_leaky_s32);
    const auto vo_leaky_s32 = svdup_n_s32(o_leaky_s32);

    switch (act)
    {
        case ActivationLayerInfo::ActivationFunction::RELU:
            fn(
                [&](auto vin, auto pg)
                {
                    // Perform activation
                    auto tmp = svmax_s8_z(pg, vconst_0, vin);
                    // Re-quantize to new output space
                    return requant ? svmla_qasymm8_signed_z(pg, tmp, vs, vo) : tmp;
                });
            break;
        case ActivationLayerInfo::ActivationFunction::BOUNDED_RELU:
            fn(
                [&](auto vin, auto pg)
                {
                    // Perform activation
                    auto tmp = svmin_s8_z(pg, va, svmax_s8_z(pg, vconst_0, vin));
                    // Re-quantize to new output space
                    return requant ? svmla_qasymm8_signed_z(pg, tmp, vs, vo) : tmp;
                });
            break;
        case ActivationLayerInfo::ActivationFunction::LU_BOUNDED_RELU:
            fn(
                [&](auto vin, auto pg)
                {
                    // Perform activation
                    auto tmp = svmin_s8_z(pg, va, svmax_s8_z(pg, vb, vin));
                    // Re-quantize to new output space
                    return requant ? svmla_qasymm8_signed_z(pg, tmp, vs, vo) : tmp;
                });
            break;
        case ActivationLayerInfo::ActivationFunction::LOGISTIC:
            fn(
                [&](auto vin, auto pg)
                {
                    // De-quantize
                    const auto vin_deq = svdequantize_z(pg, vin, qi_in);
                    // Perform activation
                    const svfloat32x4_t tmp_dep = svcreate4_f32(
                        svdiv_f32_z(
                            pg, vconst_1,
                            svadd_f32_z(pg, vconst_1, svexp_f32_z(pg, svneg_f32_z(pg, svget4_f32(vin_deq, 0))))),
                        svdiv_f32_z(
                            pg, vconst_1,
                            svadd_f32_z(pg, vconst_1, svexp_f32_z(pg, svneg_f32_z(pg, svget4_f32(vin_deq, 1))))),
                        svdiv_f32_z(
                            pg, vconst_1,
                            svadd_f32_z(pg, vconst_1, svexp_f32_z(pg, svneg_f32_z(pg, svget4_f32(vin_deq, 2))))),
                        svdiv_f32_z(
                            pg, vconst_1,
                            svadd_f32_z(pg, vconst_1, svexp_f32_z(pg, svneg_f32_z(pg, svget4_f32(vin_deq, 3))))));
                    // Re-quantize to new output space
                    return svquantize_signed_z(pg, tmp_dep, qi_out);
                });
            break;
        case ActivationLayerInfo::ActivationFunction::TANH:
            fn(
                [&](auto vin, auto pg)
                {
                    // De-quantize
                    const auto vin_deq = svdequantize_z(pg, vin, qi_in);
                    // Perform activation
                    const svfloat32x4_t tmp_dep = svcreate4_f32(
                        svmul_f32_z(pg, va_f32, svtanh_f32_z(pg, svmul_f32_z(pg, svget4_f32(vin_deq, 0), vb_f32))),
                        svmul_f32_z(pg, va_f32, svtanh_f32_z(pg, svmul_f32_z(pg, svget4_f32(vin_deq, 1), vb_f32))),
                        svmul_f32_z(pg, va_f32, svtanh_f32_z(pg, svmul_f32_z(pg, svget4_f32(vin_deq, 2), vb_f32))),
                        svmul_f32_z(pg, va_f32, svtanh_f32_z(pg, svmul_f32_z(pg, svget4_f32(vin_deq, 3), vb_f32))));
                    // Re-quantize to new output space
                    return svquantize_signed_z(pg, tmp_dep, qi_out);
                });
            break;
        case ActivationLayerInfo::ActivationFunction::HARD_SWISH:
            fn(
                [&](auto vin, auto pg)
                {
                    // De-quantize
                    const auto vin_deq = svdequantize_z(pg, vin, qi_in);
                    // Perform activation
                    const svfloat32x4_t tmp_dep = svcreate4_f32(
                        svmul_f32_z(pg, svget4_f32(vin_deq, 0),
                                    svmul_f32_z(pg, const_inv_6_f32,
                                                svmin_f32_z(pg, const_6_f32,
                                                            svmax_f32_z(pg, const_0_f32,
                                                                        svadd_f32_z(pg, svget4_f32(vin_deq, 0),
                                                                                    const_3_f32))))),
                        svmul_f32_z(pg, svget4_f32(vin_deq, 1),
                                    svmul_f32_z(pg, const_inv_6_f32,
                                                svmin_f32_z(pg, const_6_f32,
                                                            svmax_f32_z(pg, const_0_f32,
                                                                        svadd_f32_z(pg, svget4_f32(vin_deq, 1),
                                                                                    const_3_f32))))),
                        svmul_f32_z(pg, svget4_f32(vin_deq, 2),
                                    svmul_f32_z(pg, const_inv_6_f32,
                                                svmin_f32_z(pg, const_6_f32,
                                                            svmax_f32_z(pg, const_0_f32,
                                                                        svadd_f32_z(pg, svget4_f32(vin_deq, 2),
                                                                                    const_3_f32))))),
                        svmul_f32_z(pg, svget4_f32(vin_deq, 3),
                                    svmul_f32_z(pg, const_inv_6_f32,
                                                svmin_f32_z(pg, const_6_f32,
                                                            svmax_f32_z(pg, const_0_f32,
                                                                        svadd_f32_z(pg, svget4_f32(vin_deq, 3),
                                                                                    const_3_f32))))));
                    // Re-quantize to new output space
                    return svquantize_signed_z(pg, tmp_dep, qi_out);
                });
            break;
        case ActivationLayerInfo::ActivationFunction::LEAKY_RELU:
            fn(
                [&](auto vin, auto pg)
                {
                    svbool_t    p0, p1, p2, p3;
                    svint32x4_t tmp_dep;

                    // Expand to int32
                    const svint32x4_t vin_s32 =
                        svcreate4_s32(svmovlb_s32(svmovlb_s16(vin)), svmovlt_s32(svmovlb_s16(vin)),
                                      svmovlb_s32(svmovlt_s16(vin)), svmovlt_s32(svmovlt_s16(vin)));

                    // Compare elements to input offset
                    if (qi_in.scale >= 0)
                    {
                        p0 = svcmplt_s32(pg, svget4_s32(vin_s32, 0), voffset_in);
                        p1 = svcmplt_s32(pg, svget4_s32(vin_s32, 1), voffset_in);
                        p2 = svcmplt_s32(pg, svget4_s32(vin_s32, 2), voffset_in);
                        p3 = svcmplt_s32(pg, svget4_s32(vin_s32, 3), voffset_in);
                    }
                    else
                    {
                        p0 = svcmpgt_s32(pg, svget4_s32(vin_s32, 0), voffset_in);
                        p1 = svcmpgt_s32(pg, svget4_s32(vin_s32, 1), voffset_in);
                        p2 = svcmpgt_s32(pg, svget4_s32(vin_s32, 2), voffset_in);
                        p3 = svcmpgt_s32(pg, svget4_s32(vin_s32, 3), voffset_in);
                    }

                    // Multiply negative elements and requantize if necessary
                    if (requant)
                    {
                        tmp_dep = svcreate4_s32(
                            svasr_n_s32_m(pg,
                                          svmla_s32_m(pg, svsel(p0, vo_leaky_s32, vo_s32), svget4_s32(vin_s32, 0),
                                                      svsel(p0, vs_leaky_s32, vs_s32)),
                                          8),
                            svasr_n_s32_m(pg,
                                          svmla_s32_m(pg, svsel(p1, vo_leaky_s32, vo_s32), svget4_s32(vin_s32, 1),
                                                      svsel(p1, vs_leaky_s32, vs_s32)),
                                          8),
                            svasr_n_s32_m(pg,
                                          svmla_s32_m(pg, svsel(p2, vo_leaky_s32, vo_s32), svget4_s32(vin_s32, 2),
                                                      svsel(p2, vs_leaky_s32, vs_s32)),
                                          8),
                            svasr_n_s32_m(pg,
                                          svmla_s32_m(pg, svsel(p3, vo_leaky_s32, vo_s32), svget4_s32(vin_s32, 3),
                                                      svsel(p3, vs_leaky_s32, vs_s32)),
                                          8));
                    }
                    else
                    {
                        tmp_dep = svcreate4_s32(
                            svasr_n_s32_m(p0, svmad_s32_m(p0, svget4_s32(vin_s32, 0), vs_leaky_s32, vo_leaky_s32), 8),
                            svasr_n_s32_m(p1, svmad_s32_m(p1, svget4_s32(vin_s32, 1), vs_leaky_s32, vo_leaky_s32), 8),
                            svasr_n_s32_m(p2, svmad_s32_m(p2, svget4_s32(vin_s32, 2), vs_leaky_s32, vo_leaky_s32), 8),
                            svasr_n_s32_m(p3, svmad_s32_m(p3, svget4_s32(vin_s32, 3), vs_leaky_s32, vo_leaky_s32), 8));
                    }

                    // Convert uint32 vectors to uint16 vectors (with saturation)
                    const auto v_low_s16  = svqxtnt_s32(svqxtnb_s32(svget4_s32(tmp_dep, 0)), svget4_s32(tmp_dep, 1));
                    const auto v_high_s16 = svqxtnt_s32(svqxtnb_s32(svget4_s32(tmp_dep, 2)), svget4_s32(tmp_dep, 3));

                    // convert uint16 vectors to uint8 vectors (with saturation)
                    return svqxtnt_s16(svqxtnb_s16(v_low_s16), v_high_s16);
                });
            break;
        default:
            ARM_COMPUTE_ERROR("Unsupported activation function");
    }
}
} // namespace cpu
} // namespace arm_compute

#endif // ACL_SRC_CPU_KERNELS_ACTIVATION_GENERIC_SVE2_QASYMM8_SIGNED_IMPL_H
