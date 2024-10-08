/*
 * Copyright (c) 2021-2024 Arm Limited.
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

#include "arm_gemm.hpp"

#include <cstddef>
#include <cstdint>

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace depthwise {

void sve_u8q_nhwc_5x5_s1_output2x2_mla_depthfirst_impl(
  const unsigned int n_channels,
  const uint8_t *const *const inptrs,
  const uint8_t *const weights,
  const int32_t *const bias,
  const arm_gemm::Requantize32 &qp,
  const int32_t *const requant_muls,
  const int32_t *const requant_shifts,
  uint8_t *const *const outptrs
)
{
  struct Params
  {
    uint64_t n_channels;
    const void *weights;
    const int32_t *bias;
    const arm_gemm::Requantize32 *requant;
    const int32_t *const requant_muls;
    const int32_t *const requant_shifts;
    uint8_t *const *const outptrs;
    const uint8_t *inptrs[36];

    Params(
      uint64_t n_channels,
      const uint8_t *const *inptrs_raw,
      const void *const weights,
      const int32_t *const bias,
      const arm_gemm::Requantize32 &qp,
      const int32_t *const requant_muls,
      const int32_t *const requant_shifts,
      uint8_t *const *outptrs
    ) : n_channels(n_channels), weights(weights), bias(bias),
        requant(&qp), requant_muls(requant_muls),
        requant_shifts(requant_shifts), outptrs(outptrs)
    {
      inptrs[0] = inptrs_raw[0];
      inptrs[1] = inptrs_raw[1];
      inptrs[2] = inptrs_raw[6];
      inptrs[3] = inptrs_raw[7];
      inptrs[4] = inptrs_raw[2];
      inptrs[5] = inptrs_raw[8];
      inptrs[6] = inptrs_raw[3];
      inptrs[7] = inptrs_raw[4];
      inptrs[8] = inptrs_raw[11];
      inptrs[9] = inptrs_raw[12];
      inptrs[10] = inptrs_raw[9];
      inptrs[11] = inptrs_raw[10];
      inptrs[12] = inptrs_raw[5];
      inptrs[13] = inptrs_raw[13];
      inptrs[14] = inptrs_raw[14];
      inptrs[15] = inptrs_raw[15];
      inptrs[16] = inptrs_raw[16];
      inptrs[17] = inptrs_raw[17];
      inptrs[18] = inptrs_raw[18];
      inptrs[19] = inptrs_raw[19];
      inptrs[20] = inptrs_raw[20];
      inptrs[21] = inptrs_raw[21];
      inptrs[22] = inptrs_raw[22];
      inptrs[23] = inptrs_raw[23];
      inptrs[24] = inptrs_raw[24];
      inptrs[25] = inptrs_raw[25];
      inptrs[26] = inptrs_raw[26];
      inptrs[27] = inptrs_raw[27];
      inptrs[28] = inptrs_raw[28];
      inptrs[29] = inptrs_raw[29];
      inptrs[30] = inptrs_raw[30];
      inptrs[31] = inptrs_raw[31];
      inptrs[32] = inptrs_raw[32];
      inptrs[33] = inptrs_raw[33];
      inptrs[34] = inptrs_raw[34];
      inptrs[35] = inptrs_raw[35];

    }
  };

  const Params params(n_channels, inptrs, weights, bias, qp,
                      requant_muls, requant_shifts, outptrs);

  __asm__ __volatile__(
    "mov x2, #0x0\n"
    "ldr x27, [%x[params], %[offsetof_Params_requant]]\n"
    "ptrue p4.b\n"
    "ldr x3, [%x[params], %[offsetof_Params_n_channels]]\n"
    "ldr x26, [%x[params], %[offsetof_Params_outptrs]]\n"
    "ldr x4, [%x[params], %[offsetof_Params_weights]]\n"
    "add x5, %x[params], %[offsetof_Params_inptrs]\n"
    "mov x6, #0x0\n"
    "ldr x25, [%x[params], %[offsetof_Params_bias]]\n"
    "ldr x7, [%x[params], %[offsetof_Params_requant_muls]]\n"
    "mov x24, x2\n"
    "add x20, x27, %[offsetof_Requantize32_a_offset]\n"
    "add x23, x27, %[offsetof_Requantize32_b_offset]\n"
    "add x22, x27, %[offsetof_Requantize32_c_offset]\n"
    "ld1rb { z14.b }, p4/Z, [x20]\n"
    "ldr x8, [%x[params], %[offsetof_Params_requant_shifts]]\n"
    "add x21, x27, %[offsetof_Requantize32_minval]\n"
    "add x20, x27, %[offsetof_Requantize32_maxval]\n"
    "ld1rb { z12.b }, p4/Z, [x23]\n"
    "ld1rh { z10.h }, p4/Z, [x22]\n"
    "incw x24\n"
    "ld1rh { z15.h }, p4/Z, [x21]\n"
    "ld1rh { z13.h }, p4/Z, [x20]\n"
    "whilelt p3.h, x2, x3\n"
    "ldp x17, x16, [x26, #0x0]\n"
    "ldp x15, x14, [x26, #0x10]\n"
    "whilelt p2.s, x2, x3\n"
    "whilelt p1.s, x24, x3\n"
    "ld1w { z5.s }, p2/Z, [x25]\n"
    "ld1w { z16.s }, p1/Z, [x25, #1, MUL VL]\n"
    "addvl x25, x25, #2\n"
    "ld1b { z25.h }, p4/Z, [x4]\n"
    "ld1b { z28.h }, p4/Z, [x4, #1, MUL VL]\n"
    "ld1b { z4.h }, p4/Z, [x4, #2, MUL VL]\n"
    "ld1b { z23.h }, p4/Z, [x4, #3, MUL VL]\n"
    "ld1b { z31.h }, p4/Z, [x4, #4, MUL VL]\n"
    "ldp x9, x28, [x5, #0x0]\n"
    "uzp1 z6.s, z5.s, z16.s\n"
    "uzp2 z30.s, z5.s, z16.s\n"
    "str x25, [%x[params], %[offsetof_Params_bias]]\n"
    ".inst 0x454c1b39  // usublb z25.h, z25.b, z12.b\n"
    ".inst 0x454c1b9c  // usublb z28.h, z28.b, z12.b\n"
    ".inst 0x454c1884  // usublb z4.h, z4.b, z12.b\n"
    ".inst 0x454c1af7  // usublb z23.h, z23.b, z12.b\n"
    "ldp x27, x26, [x5, #0x10]\n"
    "mov z17.d, z6.d\n"
    "mov z8.d, z30.d\n"
    "mov z21.d, z6.d\n"
    "mov z27.d, z30.d\n"
    "ldp x25, x24, [x5, #0x20]\n"
    "mov z7.d, z6.d\n"
    "mov z9.d, z30.d\n"
    ".inst 0x454c1bff  // usublb z31.h, z31.b, z12.b\n"
    "ldp x23, x22, [x5, #0x30]\n"
    "ldp x21, x20, [x5, #0x40]\n"
    "ld1b { z26.h }, p3/Z, [x9, x2]\n"
    "ld1b { z16.h }, p3/Z, [x28, x2]\n"
    "ld1b { z24.h }, p3/Z, [x27, x2]\n"
    "ld1b { z5.h }, p3/Z, [x26, x2]\n"
    "ld1b { z18.h }, p3/Z, [x25, x2]\n"
    "ld1b { z3.h }, p3/Z, [x24, x2]\n"
    "ld1b { z19.h }, p3/Z, [x23, x2]\n"
    "ld1b { z11.h }, p3/Z, [x22, x2]\n"
    ".inst 0x454e1b5a  // usublb z26.h, z26.b, z14.b\n"
    ".inst 0x454e1a10  // usublb z16.h, z16.b, z14.b\n"
    "ld1b { z20.h }, p3/Z, [x21, x2]\n"
    "ld1b { z29.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454e1b18  // usublb z24.h, z24.b, z14.b\n"
    ".inst 0x454e18a5  // usublb z5.h, z5.b, z14.b\n"
    ".inst 0x454e1a52  // usublb z18.h, z18.b, z14.b\n"
    ".inst 0x454e1863  // usublb z3.h, z3.b, z14.b\n"
    ".inst 0x454e1a73  // usublb z19.h, z19.b, z14.b\n"
    ".inst 0x454e196b  // usublb z11.h, z11.b, z14.b\n"
    ".inst 0x454e1a94  // usublb z20.h, z20.b, z14.b\n"
    ".inst 0x454e1bbd  // usublb z29.h, z29.b, z14.b\n"
    "1:"  // Loop
    ".inst 0x44994346  // smlalb z6.s, p4/M, z26.h, z25.h\n"
    ".inst 0x4499475e  // smlalt z30.s, p4/M, z26.h, z25.h\n"
    "ldr x23, [x5, #0x50]\n"
    "ldr x22, [x5, #0x58]\n"
    ".inst 0x44994211  // smlalb z17.s, p4/M, z16.h, z25.h\n"
    ".inst 0x44994315  // smlalb z21.s, p4/M, z24.h, z25.h\n"
    "ldr x21, [x5, #0x60]\n"
    "ld1b { z0.h }, p4/Z, [x4, #5, MUL VL]\n"
    ".inst 0x449940a7  // smlalb z7.s, p4/M, z5.h, z25.h\n"
    ".inst 0x44994608  // smlalt z8.s, p4/M, z16.h, z25.h\n"
    "ldr x20, [x5, #0x68]\n"
    "ld1b { z26.h }, p4/Z, [x4, #6, MUL VL]\n"
    "ld1b { z2.h }, p3/Z, [x23, x2]\n"
    ".inst 0x4499471b  // smlalt z27.s, p4/M, z24.h, z25.h\n"
    ".inst 0x449944a9  // smlalt z9.s, p4/M, z5.h, z25.h\n"
    "ld1b { z22.h }, p3/Z, [x22, x2]\n"
    ".inst 0x449c4206  // smlalb z6.s, p4/M, z16.h, z28.h\n"
    ".inst 0x449c461e  // smlalt z30.s, p4/M, z16.h, z28.h\n"
    "ld1b { z1.h }, p3/Z, [x21, x2]\n"
    ".inst 0x454c1800  // usublb z0.h, z0.b, z12.b\n"
    ".inst 0x449c4251  // smlalb z17.s, p4/M, z18.h, z28.h\n"
    ".inst 0x449c40b5  // smlalb z21.s, p4/M, z5.h, z28.h\n"
    "ld1b { z16.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454c1b5a  // usublb z26.h, z26.b, z12.b\n"
    ".inst 0x449c4067  // smlalb z7.s, p4/M, z3.h, z28.h\n"
    ".inst 0x454e1842  // usublb z2.h, z2.b, z14.b\n"
    ".inst 0x449c4648  // smlalt z8.s, p4/M, z18.h, z28.h\n"
    "ldr x20, [x5, #0x70]\n"
    ".inst 0x449c44bb  // smlalt z27.s, p4/M, z5.h, z28.h\n"
    ".inst 0x449c4469  // smlalt z9.s, p4/M, z3.h, z28.h\n"
    ".inst 0x454e1ad6  // usublb z22.h, z22.b, z14.b\n"
    "ld1b { z28.h }, p4/Z, [x4, #7, MUL VL]\n"
    ".inst 0x44844246  // smlalb z6.s, p4/M, z18.h, z4.h\n"
    ".inst 0x4484465e  // smlalt z30.s, p4/M, z18.h, z4.h\n"
    ".inst 0x454e1821  // usublb z1.h, z1.b, z14.b\n"
    "inch x4, ALL, MUL #8\n"
    ".inst 0x44844271  // smlalb z17.s, p4/M, z19.h, z4.h\n"
    ".inst 0x44844075  // smlalb z21.s, p4/M, z3.h, z4.h\n"
    ".inst 0x454e1a10  // usublb z16.h, z16.b, z14.b\n"
    "ld1b { z25.h }, p3/Z, [x20, x2]\n"
    ".inst 0x44844047  // smlalb z7.s, p4/M, z2.h, z4.h\n"
    ".inst 0x44844668  // smlalt z8.s, p4/M, z19.h, z4.h\n"
    ".inst 0x454c1b9c  // usublb z28.h, z28.b, z12.b\n"
    "ldr x20, [x5, #0x78]\n"
    ".inst 0x4484447b  // smlalt z27.s, p4/M, z3.h, z4.h\n"
    ".inst 0x44844449  // smlalt z9.s, p4/M, z2.h, z4.h\n"
    "ld1b { z18.h }, p4/Z, [x4]\n"
    "ldr x22, [x5, #0x80]\n"
    ".inst 0x44974266  // smlalb z6.s, p4/M, z19.h, z23.h\n"
    ".inst 0x4497467e  // smlalt z30.s, p4/M, z19.h, z23.h\n"
    ".inst 0x454e1b39  // usublb z25.h, z25.b, z14.b\n"
    "ld1b { z4.h }, p4/Z, [x4, #1, MUL VL]\n"
    ".inst 0x44974171  // smlalb z17.s, p4/M, z11.h, z23.h\n"
    ".inst 0x44974055  // smlalb z21.s, p4/M, z2.h, z23.h\n"
    "ld1b { z19.h }, p3/Z, [x20, x2]\n"
    "ldr x21, [x5, #0x88]\n"
    ".inst 0x449742c7  // smlalb z7.s, p4/M, z22.h, z23.h\n"
    ".inst 0x44974568  // smlalt z8.s, p4/M, z11.h, z23.h\n"
    ".inst 0x454c1a52  // usublb z18.h, z18.b, z12.b\n"
    "ldr x20, [x5, #0x90]\n"
    ".inst 0x4497445b  // smlalt z27.s, p4/M, z2.h, z23.h\n"
    ".inst 0x449746c9  // smlalt z9.s, p4/M, z22.h, z23.h\n"
    "ld1b { z23.h }, p3/Z, [x22, x2]\n"
    ".inst 0x454c1884  // usublb z4.h, z4.b, z12.b\n"
    ".inst 0x449f4166  // smlalb z6.s, p4/M, z11.h, z31.h\n"
    ".inst 0x449f457e  // smlalt z30.s, p4/M, z11.h, z31.h\n"
    ".inst 0x454e1a73  // usublb z19.h, z19.b, z14.b\n"
    "ld1b { z11.h }, p4/Z, [x4, #2, MUL VL]\n"
    ".inst 0x449f4031  // smlalb z17.s, p4/M, z1.h, z31.h\n"
    ".inst 0x449f42d5  // smlalb z21.s, p4/M, z22.h, z31.h\n"
    "ldr x23, [x5, #0x98]\n"
    "ldr x22, [x5, #0xa0]\n"
    ".inst 0x449f4287  // smlalb z7.s, p4/M, z20.h, z31.h\n"
    ".inst 0x449f4428  // smlalt z8.s, p4/M, z1.h, z31.h\n"
    ".inst 0x454e1af7  // usublb z23.h, z23.b, z14.b\n"
    "ld1b { z1.h }, p3/Z, [x21, x2]\n"
    ".inst 0x449f46db  // smlalt z27.s, p4/M, z22.h, z31.h\n"
    ".inst 0x449f4689  // smlalt z9.s, p4/M, z20.h, z31.h\n"
    ".inst 0x454c196b  // usublb z11.h, z11.b, z12.b\n"
    "ld1b { z31.h }, p4/Z, [x4, #3, MUL VL]\n"
    ".inst 0x44804306  // smlalb z6.s, p4/M, z24.h, z0.h\n"
    ".inst 0x4480471e  // smlalt z30.s, p4/M, z24.h, z0.h\n"
    "ld1b { z24.h }, p3/Z, [x20, x2]\n"
    "ldr x20, [x5, #0xa8]\n"
    ".inst 0x448040b1  // smlalb z17.s, p4/M, z5.h, z0.h\n"
    ".inst 0x448043b5  // smlalb z21.s, p4/M, z29.h, z0.h\n"
    ".inst 0x454e1821  // usublb z1.h, z1.b, z14.b\n"
    "ldr x21, [x5, #0xb0]\n"
    ".inst 0x44804207  // smlalb z7.s, p4/M, z16.h, z0.h\n"
    ".inst 0x448044a8  // smlalt z8.s, p4/M, z5.h, z0.h\n"
    ".inst 0x454c1bff  // usublb z31.h, z31.b, z12.b\n"
    "ldr x13, [x5, #0xb8]\n"
    ".inst 0x448047bb  // smlalt z27.s, p4/M, z29.h, z0.h\n"
    ".inst 0x44804609  // smlalt z9.s, p4/M, z16.h, z0.h\n"
    "ld1b { z0.h }, p3/Z, [x23, x2]\n"
    ".inst 0x454e1b18  // usublb z24.h, z24.b, z14.b\n"
    ".inst 0x449a40a6  // smlalb z6.s, p4/M, z5.h, z26.h\n"
    ".inst 0x449a44be  // smlalt z30.s, p4/M, z5.h, z26.h\n"
    "ld1b { z5.h }, p4/Z, [x4, #4, MUL VL]\n"
    "ldr x12, [x5, #0xc0]\n"
    ".inst 0x449a4071  // smlalb z17.s, p4/M, z3.h, z26.h\n"
    ".inst 0x449a4215  // smlalb z21.s, p4/M, z16.h, z26.h\n"
    "ldr x11, [x5, #0xc8]\n"
    "ldr x10, [x5, #0xd0]\n"
    ".inst 0x449a4327  // smlalb z7.s, p4/M, z25.h, z26.h\n"
    ".inst 0x449a4468  // smlalt z8.s, p4/M, z3.h, z26.h\n"
    ".inst 0x454e1800  // usublb z0.h, z0.b, z14.b\n"
    "ldr x9, [x5, #0xd8]\n"
    ".inst 0x449a461b  // smlalt z27.s, p4/M, z16.h, z26.h\n"
    ".inst 0x449a4729  // smlalt z9.s, p4/M, z25.h, z26.h\n"
    "ld1b { z26.h }, p3/Z, [x22, x2]\n"
    ".inst 0x454c18a5  // usublb z5.h, z5.b, z12.b\n"
    ".inst 0x449c4066  // smlalb z6.s, p4/M, z3.h, z28.h\n"
    ".inst 0x449c447e  // smlalt z30.s, p4/M, z3.h, z28.h\n"
    "ld1b { z3.h }, p4/Z, [x4, #5, MUL VL]\n"
    "ldr x28, [x5, #0xe0]\n"
    ".inst 0x449c4051  // smlalb z17.s, p4/M, z2.h, z28.h\n"
    ".inst 0x449c4335  // smlalb z21.s, p4/M, z25.h, z28.h\n"
    "ldr x27, [x5, #0xe8]\n"
    "ldr x26, [x5, #0xf0]\n"
    ".inst 0x449c4267  // smlalb z7.s, p4/M, z19.h, z28.h\n"
    ".inst 0x449c4448  // smlalt z8.s, p4/M, z2.h, z28.h\n"
    ".inst 0x454e1b5a  // usublb z26.h, z26.b, z14.b\n"
    "ldr x25, [x5, #0xf8]\n"
    ".inst 0x449c473b  // smlalt z27.s, p4/M, z25.h, z28.h\n"
    ".inst 0x449c4669  // smlalt z9.s, p4/M, z19.h, z28.h\n"
    "ld1b { z28.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454c1863  // usublb z3.h, z3.b, z12.b\n"
    ".inst 0x44924046  // smlalb z6.s, p4/M, z2.h, z18.h\n"
    ".inst 0x4492445e  // smlalt z30.s, p4/M, z2.h, z18.h\n"
    "ld1b { z2.h }, p4/Z, [x4, #6, MUL VL]\n"
    "ldr x24, [x5, #0x100]\n"
    ".inst 0x449242d1  // smlalb z17.s, p4/M, z22.h, z18.h\n"
    ".inst 0x44924275  // smlalb z21.s, p4/M, z19.h, z18.h\n"
    "ldr x23, [x5, #0x108]\n"
    "ldr x22, [x5, #0x110]\n"
    ".inst 0x449242e7  // smlalb z7.s, p4/M, z23.h, z18.h\n"
    ".inst 0x449246c8  // smlalt z8.s, p4/M, z22.h, z18.h\n"
    ".inst 0x454e1b9c  // usublb z28.h, z28.b, z14.b\n"
    "ldr x20, [x5, #0x118]\n"
    ".inst 0x4492467b  // smlalt z27.s, p4/M, z19.h, z18.h\n"
    ".inst 0x449246e9  // smlalt z9.s, p4/M, z23.h, z18.h\n"
    "ld1b { z18.h }, p3/Z, [x21, x2]\n"
    ".inst 0x454c1842  // usublb z2.h, z2.b, z12.b\n"
    ".inst 0x448442c6  // smlalb z6.s, p4/M, z22.h, z4.h\n"
    ".inst 0x448446de  // smlalt z30.s, p4/M, z22.h, z4.h\n"
    "ld1b { z22.h }, p4/Z, [x4, #7, MUL VL]\n"
    "inch x4, ALL, MUL #8\n"
    ".inst 0x44844291  // smlalb z17.s, p4/M, z20.h, z4.h\n"
    ".inst 0x448442f5  // smlalb z21.s, p4/M, z23.h, z4.h\n"
    "whilelt p0.h, x6, x3\n"
    "ldr x21, [%x[params], %[offsetof_Params_bias]]\n"
    ".inst 0x44844027  // smlalb z7.s, p4/M, z1.h, z4.h\n"
    ".inst 0x44844688  // smlalt z8.s, p4/M, z20.h, z4.h\n"
    ".inst 0x454e1a52  // usublb z18.h, z18.b, z14.b\n"
    "ld1b { z20.h }, p3/Z, [x13, x2]\n"
    ".inst 0x448446fb  // smlalt z27.s, p4/M, z23.h, z4.h\n"
    ".inst 0x44844429  // smlalt z9.s, p4/M, z1.h, z4.h\n"
    ".inst 0x454c1ad6  // usublb z22.h, z22.b, z12.b\n"
    "ld1b { z4.h }, p4/Z, [x4]\n"
    ".inst 0x448b43a6  // smlalb z6.s, p4/M, z29.h, z11.h\n"
    ".inst 0x448b47be  // smlalt z30.s, p4/M, z29.h, z11.h\n"
    "ld1b { z29.h }, p3/Z, [x12, x2]\n"
    ".inst 0x448b4211  // smlalb z17.s, p4/M, z16.h, z11.h\n"
    ".inst 0x448b4315  // smlalb z21.s, p4/M, z24.h, z11.h\n"
    ".inst 0x454e1a94  // usublb z20.h, z20.b, z14.b\n"
    ".inst 0x448b4007  // smlalb z7.s, p4/M, z0.h, z11.h\n"
    ".inst 0x448b4608  // smlalt z8.s, p4/M, z16.h, z11.h\n"
    ".inst 0x454c1884  // usublb z4.h, z4.b, z12.b\n"
    ".inst 0x448b471b  // smlalt z27.s, p4/M, z24.h, z11.h\n"
    ".inst 0x448b4409  // smlalt z9.s, p4/M, z0.h, z11.h\n"
    "ld1b { z11.h }, p3/Z, [x11, x2]\n"
    ".inst 0x454e1bbd  // usublb z29.h, z29.b, z14.b\n"
    ".inst 0x449f4206  // smlalb z6.s, p4/M, z16.h, z31.h\n"
    ".inst 0x449f461e  // smlalt z30.s, p4/M, z16.h, z31.h\n"
    "ld1b { z16.h }, p4/Z, [x4, #1, MUL VL]\n"
    ".inst 0x449f4331  // smlalb z17.s, p4/M, z25.h, z31.h\n"
    ".inst 0x449f4015  // smlalb z21.s, p4/M, z0.h, z31.h\n"
    ".inst 0x449f4347  // smlalb z7.s, p4/M, z26.h, z31.h\n"
    ".inst 0x449f4728  // smlalt z8.s, p4/M, z25.h, z31.h\n"
    ".inst 0x454e196b  // usublb z11.h, z11.b, z14.b\n"
    ".inst 0x449f441b  // smlalt z27.s, p4/M, z0.h, z31.h\n"
    ".inst 0x449f4749  // smlalt z9.s, p4/M, z26.h, z31.h\n"
    "ld1b { z31.h }, p3/Z, [x10, x2]\n"
    ".inst 0x454c1a10  // usublb z16.h, z16.b, z12.b\n"
    ".inst 0x44854326  // smlalb z6.s, p4/M, z25.h, z5.h\n"
    ".inst 0x4485473e  // smlalt z30.s, p4/M, z25.h, z5.h\n"
    "ld1b { z25.h }, p4/Z, [x4, #2, MUL VL]\n"
    ".inst 0x44854271  // smlalb z17.s, p4/M, z19.h, z5.h\n"
    ".inst 0x44854355  // smlalb z21.s, p4/M, z26.h, z5.h\n"
    ".inst 0x44854387  // smlalb z7.s, p4/M, z28.h, z5.h\n"
    ".inst 0x44854668  // smlalt z8.s, p4/M, z19.h, z5.h\n"
    ".inst 0x454e1bff  // usublb z31.h, z31.b, z14.b\n"
    ".inst 0x4485475b  // smlalt z27.s, p4/M, z26.h, z5.h\n"
    ".inst 0x44854789  // smlalt z9.s, p4/M, z28.h, z5.h\n"
    "ld1b { z5.h }, p3/Z, [x9, x2]\n"
    ".inst 0x454c1b39  // usublb z25.h, z25.b, z12.b\n"
    ".inst 0x44834266  // smlalb z6.s, p4/M, z19.h, z3.h\n"
    ".inst 0x4483467e  // smlalt z30.s, p4/M, z19.h, z3.h\n"
    "ld1b { z19.h }, p4/Z, [x4, #3, MUL VL]\n"
    ".inst 0x448342f1  // smlalb z17.s, p4/M, z23.h, z3.h\n"
    ".inst 0x44834395  // smlalb z21.s, p4/M, z28.h, z3.h\n"
    ".inst 0x44834247  // smlalb z7.s, p4/M, z18.h, z3.h\n"
    ".inst 0x448346e8  // smlalt z8.s, p4/M, z23.h, z3.h\n"
    ".inst 0x454e18a5  // usublb z5.h, z5.b, z14.b\n"
    ".inst 0x4483479b  // smlalt z27.s, p4/M, z28.h, z3.h\n"
    ".inst 0x44834649  // smlalt z9.s, p4/M, z18.h, z3.h\n"
    "ld1b { z3.h }, p3/Z, [x28, x2]\n"
    ".inst 0x454c1a73  // usublb z19.h, z19.b, z12.b\n"
    ".inst 0x448242e6  // smlalb z6.s, p4/M, z23.h, z2.h\n"
    ".inst 0x448246fe  // smlalt z30.s, p4/M, z23.h, z2.h\n"
    "ld1b { z23.h }, p4/Z, [x4, #4, MUL VL]\n"
    ".inst 0x44824031  // smlalb z17.s, p4/M, z1.h, z2.h\n"
    ".inst 0x44824255  // smlalb z21.s, p4/M, z18.h, z2.h\n"
    ".inst 0x44824287  // smlalb z7.s, p4/M, z20.h, z2.h\n"
    ".inst 0x44824428  // smlalt z8.s, p4/M, z1.h, z2.h\n"
    ".inst 0x454e1863  // usublb z3.h, z3.b, z14.b\n"
    "ld1b { z1.h }, p3/Z, [x27, x2]\n"
    ".inst 0x4482465b  // smlalt z27.s, p4/M, z18.h, z2.h\n"
    ".inst 0x44824689  // smlalt z9.s, p4/M, z20.h, z2.h\n"
    ".inst 0x454c1af7  // usublb z23.h, z23.b, z12.b\n"
    "ld1b { z2.h }, p4/Z, [x4, #5, MUL VL]\n"
    ".inst 0x44964306  // smlalb z6.s, p4/M, z24.h, z22.h\n"
    ".inst 0x4496471e  // smlalt z30.s, p4/M, z24.h, z22.h\n"
    "ld1b { z24.h }, p3/Z, [x26, x2]\n"
    ".inst 0x44964011  // smlalb z17.s, p4/M, z0.h, z22.h\n"
    ".inst 0x449643b5  // smlalb z21.s, p4/M, z29.h, z22.h\n"
    ".inst 0x454e1821  // usublb z1.h, z1.b, z14.b\n"
    ".inst 0x44964167  // smlalb z7.s, p4/M, z11.h, z22.h\n"
    ".inst 0x44964408  // smlalt z8.s, p4/M, z0.h, z22.h\n"
    ".inst 0x454c1842  // usublb z2.h, z2.b, z12.b\n"
    ".inst 0x449647bb  // smlalt z27.s, p4/M, z29.h, z22.h\n"
    ".inst 0x44964569  // smlalt z9.s, p4/M, z11.h, z22.h\n"
    "ld1b { z22.h }, p3/Z, [x25, x2]\n"
    ".inst 0x454e1b18  // usublb z24.h, z24.b, z14.b\n"
    ".inst 0x44844006  // smlalb z6.s, p4/M, z0.h, z4.h\n"
    ".inst 0x4484441e  // smlalt z30.s, p4/M, z0.h, z4.h\n"
    "ld1b { z0.h }, p4/Z, [x4, #6, MUL VL]\n"
    ".inst 0x44844351  // smlalb z17.s, p4/M, z26.h, z4.h\n"
    ".inst 0x44844175  // smlalb z21.s, p4/M, z11.h, z4.h\n"
    ".inst 0x448443e7  // smlalb z7.s, p4/M, z31.h, z4.h\n"
    ".inst 0x44844748  // smlalt z8.s, p4/M, z26.h, z4.h\n"
    ".inst 0x454e1ad6  // usublb z22.h, z22.b, z14.b\n"
    ".inst 0x4484457b  // smlalt z27.s, p4/M, z11.h, z4.h\n"
    ".inst 0x448447e9  // smlalt z9.s, p4/M, z31.h, z4.h\n"
    "ld1b { z4.h }, p3/Z, [x24, x2]\n"
    ".inst 0x454c1800  // usublb z0.h, z0.b, z12.b\n"
    ".inst 0x44904346  // smlalb z6.s, p4/M, z26.h, z16.h\n"
    ".inst 0x4490475e  // smlalt z30.s, p4/M, z26.h, z16.h\n"
    "ld1b { z26.h }, p4/Z, [x4, #7, MUL VL]\n"
    "inch x4, ALL, MUL #8\n"
    ".inst 0x44904391  // smlalb z17.s, p4/M, z28.h, z16.h\n"
    ".inst 0x449043f5  // smlalb z21.s, p4/M, z31.h, z16.h\n"
    ".inst 0x449040a7  // smlalb z7.s, p4/M, z5.h, z16.h\n"
    ".inst 0x44904788  // smlalt z8.s, p4/M, z28.h, z16.h\n"
    ".inst 0x454e1884  // usublb z4.h, z4.b, z14.b\n"
    ".inst 0x449047fb  // smlalt z27.s, p4/M, z31.h, z16.h\n"
    ".inst 0x449044a9  // smlalt z9.s, p4/M, z5.h, z16.h\n"
    "ld1b { z16.h }, p3/Z, [x23, x2]\n"
    ".inst 0x454c1b5a  // usublb z26.h, z26.b, z12.b\n"
    ".inst 0x44994386  // smlalb z6.s, p4/M, z28.h, z25.h\n"
    ".inst 0x4499479e  // smlalt z30.s, p4/M, z28.h, z25.h\n"
    "ld1b { z28.h }, p4/Z, [x4]\n"
    "inch x4\n"
    ".inst 0x44994251  // smlalb z17.s, p4/M, z18.h, z25.h\n"
    ".inst 0x449940b5  // smlalb z21.s, p4/M, z5.h, z25.h\n"
    ".inst 0x44994067  // smlalb z7.s, p4/M, z3.h, z25.h\n"
    ".inst 0x44994648  // smlalt z8.s, p4/M, z18.h, z25.h\n"
    ".inst 0x454e1a10  // usublb z16.h, z16.b, z14.b\n"
    ".inst 0x449944bb  // smlalt z27.s, p4/M, z5.h, z25.h\n"
    ".inst 0x44994469  // smlalt z9.s, p4/M, z3.h, z25.h\n"
    "ld1b { z25.h }, p3/Z, [x22, x2]\n"
    ".inst 0x454c1b9c  // usublb z28.h, z28.b, z12.b\n"
    ".inst 0x44934246  // smlalb z6.s, p4/M, z18.h, z19.h\n"
    ".inst 0x4493465e  // smlalt z30.s, p4/M, z18.h, z19.h\n"
    "ld1w { z18.s }, p2/Z, [x7]\n"
    ".inst 0x44934291  // smlalb z17.s, p4/M, z20.h, z19.h\n"
    ".inst 0x44934075  // smlalb z21.s, p4/M, z3.h, z19.h\n"
    ".inst 0x44934027  // smlalb z7.s, p4/M, z1.h, z19.h\n"
    ".inst 0x44934688  // smlalt z8.s, p4/M, z20.h, z19.h\n"
    "ld1w { z20.s }, p1/Z, [x7, #1, MUL VL]\n"
    ".inst 0x454e1b39  // usublb z25.h, z25.b, z14.b\n"
    ".inst 0x4493447b  // smlalt z27.s, p4/M, z3.h, z19.h\n"
    ".inst 0x44934429  // smlalt z9.s, p4/M, z1.h, z19.h\n"
    "ld1b { z19.h }, p3/Z, [x20, x2]\n"
    "inch x2\n"
    ".inst 0x449743a6  // smlalb z6.s, p4/M, z29.h, z23.h\n"
    ".inst 0x449747be  // smlalt z30.s, p4/M, z29.h, z23.h\n"
    "addvl x7, x7, #2\n"
    ".inst 0x44974171  // smlalb z17.s, p4/M, z11.h, z23.h\n"
    ".inst 0x44974315  // smlalb z21.s, p4/M, z24.h, z23.h\n"
    "uzp1 z29.s, z18.s, z20.s\n"
    ".inst 0x449742c7  // smlalb z7.s, p4/M, z22.h, z23.h\n"
    ".inst 0x44974568  // smlalt z8.s, p4/M, z11.h, z23.h\n"
    "uzp2 z18.s, z18.s, z20.s\n"
    "ld1w { z20.s }, p2/Z, [x8]\n"
    ".inst 0x4497471b  // smlalt z27.s, p4/M, z24.h, z23.h\n"
    ".inst 0x449746c9  // smlalt z9.s, p4/M, z22.h, z23.h\n"
    "ld1w { z24.s }, p1/Z, [x8, #1, MUL VL]\n"
    ".inst 0x454e1a73  // usublb z19.h, z19.b, z14.b\n"
    ".inst 0x44824166  // smlalb z6.s, p4/M, z11.h, z2.h\n"
    ".inst 0x4482457e  // smlalt z30.s, p4/M, z11.h, z2.h\n"
    "mov x20, x2\n"
    "whilelt p2.s, x2, x3\n"
    ".inst 0x448243f1  // smlalb z17.s, p4/M, z31.h, z2.h\n"
    ".inst 0x448242d5  // smlalb z21.s, p4/M, z22.h, z2.h\n"
    "addvl x8, x8, #2\n"
    ".inst 0x44824087  // smlalb z7.s, p4/M, z4.h, z2.h\n"
    ".inst 0x448247e8  // smlalt z8.s, p4/M, z31.h, z2.h\n"
    "uzp1 z23.s, z20.s, z24.s\n"
    ".inst 0x448246db  // smlalt z27.s, p4/M, z22.h, z2.h\n"
    ".inst 0x44824489  // smlalt z9.s, p4/M, z4.h, z2.h\n"
    "uzp2 z22.s, z20.s, z24.s\n"
    "incw x20\n"
    ".inst 0x448043e6  // smlalb z6.s, p4/M, z31.h, z0.h\n"
    ".inst 0x448047fe  // smlalt z30.s, p4/M, z31.h, z0.h\n"
    ".inst 0x448040b1  // smlalb z17.s, p4/M, z5.h, z0.h\n"
    ".inst 0x44804095  // smlalb z21.s, p4/M, z4.h, z0.h\n"
    ".inst 0x44804207  // smlalb z7.s, p4/M, z16.h, z0.h\n"
    ".inst 0x448044a8  // smlalt z8.s, p4/M, z5.h, z0.h\n"
    "whilelt p1.s, x20, x3\n"
    "whilelt p3.h, x2, x3\n"
    ".inst 0x4480449b  // smlalt z27.s, p4/M, z4.h, z0.h\n"
    ".inst 0x44804609  // smlalt z9.s, p4/M, z16.h, z0.h\n"
    ".inst 0x449a40a6  // smlalb z6.s, p4/M, z5.h, z26.h\n"
    ".inst 0x449a44be  // smlalt z30.s, p4/M, z5.h, z26.h\n"
    ".inst 0x449a4071  // smlalb z17.s, p4/M, z3.h, z26.h\n"
    ".inst 0x449a4215  // smlalb z21.s, p4/M, z16.h, z26.h\n"
    ".inst 0x449a4327  // smlalb z7.s, p4/M, z25.h, z26.h\n"
    ".inst 0x449a4468  // smlalt z8.s, p4/M, z3.h, z26.h\n"
    ".inst 0x449a461b  // smlalt z27.s, p4/M, z16.h, z26.h\n"
    ".inst 0x449a4729  // smlalt z9.s, p4/M, z25.h, z26.h\n"
    ".inst 0x449c4066  // smlalb z6.s, p4/M, z3.h, z28.h\n"
    ".inst 0x449c447e  // smlalt z30.s, p4/M, z3.h, z28.h\n"
    ".inst 0x449c4031  // smlalb z17.s, p4/M, z1.h, z28.h\n"
    ".inst 0x449c4335  // smlalb z21.s, p4/M, z25.h, z28.h\n"
    ".inst 0x449c4267  // smlalb z7.s, p4/M, z19.h, z28.h\n"
    ".inst 0x449c4428  // smlalt z8.s, p4/M, z1.h, z28.h\n"
    ".inst 0x449c473b  // smlalt z27.s, p4/M, z25.h, z28.h\n"
    ".inst 0x449c4669  // smlalt z9.s, p4/M, z19.h, z28.h\n"
    ".inst 0x04bd74c6  // sqrdmulh z6.s, z6.s, z29.s\n"
    ".inst 0x04b277de  // sqrdmulh z30.s, z30.s, z18.s\n"
    ".inst 0x04bd7631  // sqrdmulh z17.s, z17.s, z29.s\n"
    ".inst 0x04bd76b5  // sqrdmulh z21.s, z21.s, z29.s\n"
    "and z19.d, z6.d, z23.d\n"
    ".inst 0x04bd74e7  // sqrdmulh z7.s, z7.s, z29.s\n"
    ".inst 0x04b27508  // sqrdmulh z8.s, z8.s, z18.s\n"
    "and z16.d, z30.d, z22.d\n"
    "and z2.d, z17.d, z23.d\n"
    "asr z19.s, z19.s, #0x1f\n"
    "and z20.d, z21.d, z23.d\n"
    ".inst 0x04b2777b  // sqrdmulh z27.s, z27.s, z18.s\n"
    ".inst 0x04b27529  // sqrdmulh z9.s, z9.s, z18.s\n"
    "asr z16.s, z16.s, #0x1f\n"
    "asr z2.s, z2.s, #0x1f\n"
    "sqadd z6.s, z6.s, z19.s\n"
    "and z19.d, z7.d, z23.d\n"
    "and z0.d, z8.d, z22.d\n"
    "asr z20.s, z20.s, #0x1f\n"
    "sqadd z30.s, z30.s, z16.s\n"
    "and z26.d, z27.d, z22.d\n"
    "asr z19.s, z19.s, #0x1f\n"
    "and z16.d, z9.d, z22.d\n"
    ".inst 0x448292e6  // srshl z6.s, p4/M, z6.s, z23.s\n"
    "sqadd z17.s, z17.s, z2.s\n"
    "asr z0.s, z0.s, #0x1f\n"
    "sqadd z21.s, z21.s, z20.s\n"
    "asr z26.s, z26.s, #0x1f\n"
    ".inst 0x448292de  // srshl z30.s, p4/M, z30.s, z22.s\n"
    "sqadd z7.s, z7.s, z19.s\n"
    "asr z16.s, z16.s, #0x1f\n"
    ".inst 0x448292f1  // srshl z17.s, p4/M, z17.s, z23.s\n"
    "sqadd z8.s, z8.s, z0.s\n"
    ".inst 0x453040c6  // sqxtnb z6.h, z6.s\n"
    ".inst 0x448292f5  // srshl z21.s, p4/M, z21.s, z23.s\n"
    "sqadd z27.s, z27.s, z26.s\n"
    ".inst 0x448292e7  // srshl z7.s, p4/M, z7.s, z23.s\n"
    "sqadd z9.s, z9.s, z16.s\n"
    ".inst 0x45304231  // sqxtnb z17.h, z17.s\n"
    ".inst 0x448292c8  // srshl z8.s, p4/M, z8.s, z22.s\n"
    ".inst 0x453042b5  // sqxtnb z21.h, z21.s\n"
    ".inst 0x453047c6  // sqxtnt z6.h, z30.s\n"
    ".inst 0x448292db  // srshl z27.s, p4/M, z27.s, z22.s\n"
    ".inst 0x448292c9  // srshl z9.s, p4/M, z9.s, z22.s\n"
    ".inst 0x453040e7  // sqxtnb z7.h, z7.s\n"
    ".inst 0x45304511  // sqxtnt z17.h, z8.s\n"
    ".inst 0x45304775  // sqxtnt z21.h, z27.s\n"
    ".inst 0x45304527  // sqxtnt z7.h, z9.s\n"
    "sqadd z6.h, z6.h, z10.h\n"
    "sqadd z17.h, z17.h, z10.h\n"
    "sqadd z21.h, z21.h, z10.h\n"
    "sqadd z7.h, z7.h, z10.h\n"
    "smax z6.h, p4/M, z6.h, z15.h\n"
    "smax z17.h, p4/M, z17.h, z15.h\n"
    "smax z21.h, p4/M, z21.h, z15.h\n"
    "smax z7.h, p4/M, z7.h, z15.h\n"
    "smin z6.h, p4/M, z6.h, z13.h\n"
    "smin z17.h, p4/M, z17.h, z13.h\n"
    "smin z21.h, p4/M, z21.h, z13.h\n"
    "smin z7.h, p4/M, z7.h, z13.h\n"
    "st1b { z6.h }, p0, [x17, x6]\n"
    "st1b { z17.h }, p0, [x16, x6]\n"
    "st1b { z21.h }, p0, [x15, x6]\n"
    "st1b { z7.h }, p0, [x14, x6]\n"
    "inch x6\n"
    "ld1w { z21.s }, p2/Z, [x21]\n"
    "ld1w { z16.s }, p1/Z, [x21, #1, MUL VL]\n"
    "addvl x21, x21, #2\n"
    "ld1b { z25.h }, p4/Z, [x4]\n"
    "ld1b { z28.h }, p4/Z, [x4, #1, MUL VL]\n"
    "ld1b { z4.h }, p4/Z, [x4, #2, MUL VL]\n"
    "ld1b { z23.h }, p4/Z, [x4, #3, MUL VL]\n"
    "ld1b { z31.h }, p4/Z, [x4, #4, MUL VL]\n"
    "ldp x9, x28, [x5, #0x0]\n"
    "uzp1 z6.s, z21.s, z16.s\n"
    "uzp2 z30.s, z21.s, z16.s\n"
    "str x21, [%x[params], %[offsetof_Params_bias]]\n"
    ".inst 0x454c1b39  // usublb z25.h, z25.b, z12.b\n"
    ".inst 0x454c1b9c  // usublb z28.h, z28.b, z12.b\n"
    ".inst 0x454c1884  // usublb z4.h, z4.b, z12.b\n"
    ".inst 0x454c1af7  // usublb z23.h, z23.b, z12.b\n"
    "ldp x27, x26, [x5, #0x10]\n"
    "mov z17.d, z6.d\n"
    "mov z8.d, z30.d\n"
    "mov z21.d, z6.d\n"
    "mov z27.d, z30.d\n"
    "ldp x25, x24, [x5, #0x20]\n"
    "mov z7.d, z6.d\n"
    "mov z9.d, z30.d\n"
    ".inst 0x454c1bff  // usublb z31.h, z31.b, z12.b\n"
    "ldp x23, x22, [x5, #0x30]\n"
    "ldp x21, x20, [x5, #0x40]\n"
    "ld1b { z26.h }, p3/Z, [x9, x2]\n"
    "ld1b { z16.h }, p3/Z, [x28, x2]\n"
    "ld1b { z24.h }, p3/Z, [x27, x2]\n"
    "ld1b { z5.h }, p3/Z, [x26, x2]\n"
    "ld1b { z18.h }, p3/Z, [x25, x2]\n"
    "ld1b { z3.h }, p3/Z, [x24, x2]\n"
    "ld1b { z19.h }, p3/Z, [x23, x2]\n"
    "ld1b { z11.h }, p3/Z, [x22, x2]\n"
    ".inst 0x454e1b5a  // usublb z26.h, z26.b, z14.b\n"
    ".inst 0x454e1a10  // usublb z16.h, z16.b, z14.b\n"
    "ld1b { z20.h }, p3/Z, [x21, x2]\n"
    "ld1b { z29.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454e1b18  // usublb z24.h, z24.b, z14.b\n"
    ".inst 0x454e18a5  // usublb z5.h, z5.b, z14.b\n"
    ".inst 0x454e1a52  // usublb z18.h, z18.b, z14.b\n"
    ".inst 0x454e1863  // usublb z3.h, z3.b, z14.b\n"
    ".inst 0x454e1a73  // usublb z19.h, z19.b, z14.b\n"
    ".inst 0x454e196b  // usublb z11.h, z11.b, z14.b\n"
    ".inst 0x454e1a94  // usublb z20.h, z20.b, z14.b\n"
    ".inst 0x454e1bbd  // usublb z29.h, z29.b, z14.b\n"
    "b.any 1b\n"
    :
    : [offsetof_Params_bias] "I" (offsetof(Params, bias)), [offsetof_Params_inptrs] "I" (offsetof(Params, inptrs)), [offsetof_Params_n_channels] "I" (offsetof(Params, n_channels)), [offsetof_Params_outptrs] "I" (offsetof(Params, outptrs)), [offsetof_Params_requant] "I" (offsetof(Params, requant)), [offsetof_Params_requant_muls] "I" (offsetof(Params, requant_muls)), [offsetof_Params_requant_shifts] "I" (offsetof(Params, requant_shifts)), [offsetof_Params_weights] "I" (offsetof(Params, weights)), [offsetof_Requantize32_a_offset] "I" (offsetof(arm_gemm::Requantize32, a_offset)), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [params] "r" (&params)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)
