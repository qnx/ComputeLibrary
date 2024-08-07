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

#include "pooling.hpp"
#include <cstdint>
#include <cstddef>

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace pooling {


void sve_u8q_nhwc_max_generic_depthfirst_impl(
  const uint64_t,
  const uint64_t n_valid_cells,
  uint64_t n_channels,
  const uint8_t *const *const inptrs,
  uint8_t *outptr,
  const Requantize32 &qp
)
{
  __asm__ __volatile__(
    "mov x9, #0x0\n"
    "cntb x28\n"
    "cntb x27, ALL, MUL #2\n"
    "cntb x26, ALL, MUL #3\n"
    "whilelt p4.b, x9, %x[n_channels]\n"
    "whilelt p3.b, x28, %x[n_channels]\n"
    "whilelt p2.b, x27, %x[n_channels]\n"
    "whilelt p1.b, x26, %x[n_channels]\n"
    "ptrue p0.b\n"
    "b.none 7f\n"
    "1:"  // 4-vectors of channels
    "lsr x25, %x[n_valid_cells], #0x2\n"
    "mov z8.b, #0x0\n"
    "mov z7.b, #0x0\n"
    "mov x24, %x[inptrs]\n"
    "mov z6.b, #0x0\n"
    "mov z5.b, #0x0\n"
    "cbz x25, 4f\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "subs x25, x25, #0x1\n"
    "add x24, x24, #0x20\n"
    "ld1b { z4.b }, p4/Z, [x23, x9]\n"
    "ld1b { z3.b }, p4/Z, [x22, x9]\n"
    "ld1b { z2.b }, p4/Z, [x21, x9]\n"
    "ld1b { z1.b }, p4/Z, [x20, x9]\n"
    "ld1b { z0.b }, p3/Z, [x23, x28]\n"
    "ld1b { z31.b }, p3/Z, [x22, x28]\n"
    "ld1b { z22.b }, p3/Z, [x21, x28]\n"
    "ld1b { z30.b }, p3/Z, [x20, x28]\n"
    "ld1b { z29.b }, p2/Z, [x23, x27]\n"
    "ld1b { z28.b }, p2/Z, [x22, x27]\n"
    "ld1b { z21.b }, p2/Z, [x21, x27]\n"
    "ld1b { z27.b }, p2/Z, [x20, x27]\n"
    "ld1b { z26.b }, p1/Z, [x23, x26]\n"
    "ld1b { z25.b }, p1/Z, [x22, x26]\n"
    "ld1b { z20.b }, p1/Z, [x21, x26]\n"
    "ld1b { z24.b }, p1/Z, [x20, x26]\n"
    "beq 3f\n"
    "2:"  // 4-vectors of channels: 4 inputs loop
    "movprfx z19, z4\n umax z19.b, p0/M, z19.b, z3.b\n"
    "movprfx z23, z2\n umax z23.b, p0/M, z23.b, z1.b\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "movprfx z18, z0\n umax z18.b, p0/M, z18.b, z31.b\n"
    "umax z22.b, p0/M, z22.b, z30.b\n"
    "ld1b { z4.b }, p4/Z, [x23, x9]\n"
    "ld1b { z3.b }, p4/Z, [x22, x9]\n"
    "movprfx z17, z29\n umax z17.b, p0/M, z17.b, z28.b\n"
    "umax z21.b, p0/M, z21.b, z27.b\n"
    "ld1b { z2.b }, p4/Z, [x21, x9]\n"
    "ld1b { z1.b }, p4/Z, [x20, x9]\n"
    "movprfx z16, z26\n umax z16.b, p0/M, z16.b, z25.b\n"
    "umax z20.b, p0/M, z20.b, z24.b\n"
    "ld1b { z0.b }, p3/Z, [x23, x28]\n"
    "ld1b { z31.b }, p3/Z, [x22, x28]\n"
    "umax z19.b, p0/M, z19.b, z23.b\n"
    "umax z18.b, p0/M, z18.b, z22.b\n"
    "ld1b { z22.b }, p3/Z, [x21, x28]\n"
    "ld1b { z30.b }, p3/Z, [x20, x28]\n"
    "umax z17.b, p0/M, z17.b, z21.b\n"
    "umax z16.b, p0/M, z16.b, z20.b\n"
    "ld1b { z29.b }, p2/Z, [x23, x27]\n"
    "ld1b { z28.b }, p2/Z, [x22, x27]\n"
    "subs x25, x25, #0x1\n"
    "umax z8.b, p0/M, z8.b, z19.b\n"
    "ld1b { z21.b }, p2/Z, [x21, x27]\n"
    "ld1b { z27.b }, p2/Z, [x20, x27]\n"
    "umax z7.b, p0/M, z7.b, z18.b\n"
    "umax z6.b, p0/M, z6.b, z17.b\n"
    "ld1b { z26.b }, p1/Z, [x23, x26]\n"
    "ld1b { z25.b }, p1/Z, [x22, x26]\n"
    "umax z5.b, p0/M, z5.b, z16.b\n"
    "add x24, x24, #0x20\n"
    "ld1b { z20.b }, p1/Z, [x21, x26]\n"
    "ld1b { z24.b }, p1/Z, [x20, x26]\n"
    "bgt 2b\n"
    "3:"  // 4-vectors of channels: 4 inputs tail
    "movprfx z19, z4\n umax z19.b, p0/M, z19.b, z3.b\n"
    "movprfx z23, z2\n umax z23.b, p0/M, z23.b, z1.b\n"
    "movprfx z18, z0\n umax z18.b, p0/M, z18.b, z31.b\n"
    "umax z22.b, p0/M, z22.b, z30.b\n"
    "movprfx z17, z29\n umax z17.b, p0/M, z17.b, z28.b\n"
    "umax z21.b, p0/M, z21.b, z27.b\n"
    "movprfx z16, z26\n umax z16.b, p0/M, z16.b, z25.b\n"
    "umax z20.b, p0/M, z20.b, z24.b\n"
    "umax z19.b, p0/M, z19.b, z23.b\n"
    "umax z18.b, p0/M, z18.b, z22.b\n"
    "umax z17.b, p0/M, z17.b, z21.b\n"
    "umax z16.b, p0/M, z16.b, z20.b\n"
    "umax z8.b, p0/M, z8.b, z19.b\n"
    "umax z7.b, p0/M, z7.b, z18.b\n"
    "umax z6.b, p0/M, z6.b, z17.b\n"
    "umax z5.b, p0/M, z5.b, z16.b\n"
    "4:"  // 4-vectors of channels: After loop
    "ands x21, %x[n_valid_cells], #0x3\n"
    "beq 6f\n"
    "5:"  // 4-vectors of channels: Single input loop
    "ldr x20, [x24], #0x8\n"
    "ld1b { z16.b }, p4/Z, [x20, x9]\n"
    "subs x21, x21, #0x1\n"
    "umax z8.b, p0/M, z8.b, z16.b\n"
    "ld1b { z17.b }, p3/Z, [x20, x28]\n"
    "ld1b { z16.b }, p2/Z, [x20, x27]\n"
    "umax z7.b, p0/M, z7.b, z17.b\n"
    "umax z6.b, p0/M, z6.b, z16.b\n"
    "ld1b { z16.b }, p1/Z, [x20, x26]\n"
    "umax z5.b, p0/M, z5.b, z16.b\n"
    "bgt 5b\n"
    "6:"  // 4-vectors of channels: Single input loop: End
    "add x20, %x[quant_params], %[offsetof_qp_input_offset]\n"
    "ld1rw { z3.s }, p0/Z, [x20]\n"
    ".inst 0x4508a911  // ushllb z17.h, z8.b, #0x0\n"
    ".inst 0x4508ad18  // ushllt z24.h, z8.b, #0x0\n"
    ".inst 0x4508a8f7  // ushllb z23.h, z7.b, #0x0\n"
    ".inst 0x4508acf6  // ushllt z22.h, z7.b, #0x0\n"
    "neg z3.s, p0/M, z3.s\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_left_shift]\n"
    ".inst 0x4508a8d5  // ushllb z21.h, z6.b, #0x0\n"
    ".inst 0x4508acd4  // ushllt z20.h, z6.b, #0x0\n"
    "ld1rw { z2.s }, p0/Z, [x20]\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_mul]\n"
    ".inst 0x4508a8b3  // ushllb z19.h, z5.b, #0x0\n"
    ".inst 0x4508acb0  // ushllt z16.h, z5.b, #0x0\n"
    "ld1rw { z18.s }, p0/Z, [x20]\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_right_shift]\n"
    ".inst 0x45914061  // saddwb z1.s, z3.s, z17.h\n"
    ".inst 0x45914471  // saddwt z17.s, z3.s, z17.h\n"
    ".inst 0x44828041  // srshl z1.s, p0/M, z1.s, z2.s\n"
    ".inst 0x44828051  // srshl z17.s, p0/M, z17.s, z2.s\n"
    ".inst 0x45984060  // saddwb z0.s, z3.s, z24.h\n"
    ".inst 0x4598447f  // saddwt z31.s, z3.s, z24.h\n"
    ".inst 0x44828040  // srshl z0.s, p0/M, z0.s, z2.s\n"
    ".inst 0x4482805f  // srshl z31.s, p0/M, z31.s, z2.s\n"
    ".inst 0x4597407e  // saddwb z30.s, z3.s, z23.h\n"
    ".inst 0x4597447d  // saddwt z29.s, z3.s, z23.h\n"
    ".inst 0x4482805e  // srshl z30.s, p0/M, z30.s, z2.s\n"
    ".inst 0x4482805d  // srshl z29.s, p0/M, z29.s, z2.s\n"
    ".inst 0x4596407c  // saddwb z28.s, z3.s, z22.h\n"
    ".inst 0x4596447b  // saddwt z27.s, z3.s, z22.h\n"
    ".inst 0x4482805c  // srshl z28.s, p0/M, z28.s, z2.s\n"
    ".inst 0x4482805b  // srshl z27.s, p0/M, z27.s, z2.s\n"
    ".inst 0x4595407a  // saddwb z26.s, z3.s, z21.h\n"
    ".inst 0x45954479  // saddwt z25.s, z3.s, z21.h\n"
    ".inst 0x4482805a  // srshl z26.s, p0/M, z26.s, z2.s\n"
    ".inst 0x44828059  // srshl z25.s, p0/M, z25.s, z2.s\n"
    ".inst 0x45944078  // saddwb z24.s, z3.s, z20.h\n"
    ".inst 0x45944477  // saddwt z23.s, z3.s, z20.h\n"
    ".inst 0x44828058  // srshl z24.s, p0/M, z24.s, z2.s\n"
    ".inst 0x44828057  // srshl z23.s, p0/M, z23.s, z2.s\n"
    ".inst 0x45934076  // saddwb z22.s, z3.s, z19.h\n"
    ".inst 0x45934475  // saddwt z21.s, z3.s, z19.h\n"
    ".inst 0x44828056  // srshl z22.s, p0/M, z22.s, z2.s\n"
    ".inst 0x44828055  // srshl z21.s, p0/M, z21.s, z2.s\n"
    ".inst 0x45904074  // saddwb z20.s, z3.s, z16.h\n"
    ".inst 0x45904473  // saddwt z19.s, z3.s, z16.h\n"
    ".inst 0x44828054  // srshl z20.s, p0/M, z20.s, z2.s\n"
    ".inst 0x44828053  // srshl z19.s, p0/M, z19.s, z2.s\n"
    "ld1rw { z16.s }, p0/Z, [x20]\n"
    ".inst 0x04b27421  // sqrdmulh z1.s, z1.s, z18.s\n"
    ".inst 0x04b27631  // sqrdmulh z17.s, z17.s, z18.s\n"
    "add x20, %x[quant_params], %[offsetof_qp_output_offset]\n"
    ".inst 0x04b27400  // sqrdmulh z0.s, z0.s, z18.s\n"
    ".inst 0x04b277ff  // sqrdmulh z31.s, z31.s, z18.s\n"
    ".inst 0x44828201  // srshl z1.s, p0/M, z1.s, z16.s\n"
    ".inst 0x44828211  // srshl z17.s, p0/M, z17.s, z16.s\n"
    ".inst 0x04b277de  // sqrdmulh z30.s, z30.s, z18.s\n"
    ".inst 0x04b277bd  // sqrdmulh z29.s, z29.s, z18.s\n"
    ".inst 0x44828200  // srshl z0.s, p0/M, z0.s, z16.s\n"
    ".inst 0x4482821f  // srshl z31.s, p0/M, z31.s, z16.s\n"
    ".inst 0x04b2779c  // sqrdmulh z28.s, z28.s, z18.s\n"
    ".inst 0x04b2777b  // sqrdmulh z27.s, z27.s, z18.s\n"
    ".inst 0x4482821e  // srshl z30.s, p0/M, z30.s, z16.s\n"
    ".inst 0x4482821d  // srshl z29.s, p0/M, z29.s, z16.s\n"
    ".inst 0x04b2775a  // sqrdmulh z26.s, z26.s, z18.s\n"
    ".inst 0x04b27739  // sqrdmulh z25.s, z25.s, z18.s\n"
    ".inst 0x4482821c  // srshl z28.s, p0/M, z28.s, z16.s\n"
    ".inst 0x4482821b  // srshl z27.s, p0/M, z27.s, z16.s\n"
    ".inst 0x04b27718  // sqrdmulh z24.s, z24.s, z18.s\n"
    ".inst 0x04b276f7  // sqrdmulh z23.s, z23.s, z18.s\n"
    ".inst 0x4482821a  // srshl z26.s, p0/M, z26.s, z16.s\n"
    ".inst 0x44828219  // srshl z25.s, p0/M, z25.s, z16.s\n"
    ".inst 0x04b276d6  // sqrdmulh z22.s, z22.s, z18.s\n"
    ".inst 0x04b276b5  // sqrdmulh z21.s, z21.s, z18.s\n"
    ".inst 0x44828218  // srshl z24.s, p0/M, z24.s, z16.s\n"
    ".inst 0x44828217  // srshl z23.s, p0/M, z23.s, z16.s\n"
    ".inst 0x04b27694  // sqrdmulh z20.s, z20.s, z18.s\n"
    ".inst 0x04b27673  // sqrdmulh z19.s, z19.s, z18.s\n"
    ".inst 0x44828216  // srshl z22.s, p0/M, z22.s, z16.s\n"
    ".inst 0x44828215  // srshl z21.s, p0/M, z21.s, z16.s\n"
    ".inst 0x44828214  // srshl z20.s, p0/M, z20.s, z16.s\n"
    ".inst 0x44828213  // srshl z19.s, p0/M, z19.s, z16.s\n"
    "ld1rw { z16.s }, p0/Z, [x20]\n"
    "add z1.s, z1.s, z16.s\n"
    "add z17.s, z17.s, z16.s\n"
    "add z0.s, z0.s, z16.s\n"
    "add z31.s, z31.s, z16.s\n"
    "add z30.s, z30.s, z16.s\n"
    "add z29.s, z29.s, z16.s\n"
    "add z28.s, z28.s, z16.s\n"
    "add z27.s, z27.s, z16.s\n"
    "add z26.s, z26.s, z16.s\n"
    "add z25.s, z25.s, z16.s\n"
    "add z24.s, z24.s, z16.s\n"
    "add z23.s, z23.s, z16.s\n"
    "add z22.s, z22.s, z16.s\n"
    "add z21.s, z21.s, z16.s\n"
    "add z20.s, z20.s, z16.s\n"
    "add z19.s, z19.s, z16.s\n"
    "mov z16.s, #0x0\n"
    "smax z1.s, p0/M, z1.s, z16.s\n"
    "smax z17.s, p0/M, z17.s, z16.s\n"
    "smax z0.s, p0/M, z0.s, z16.s\n"
    "smax z31.s, p0/M, z31.s, z16.s\n"
    "mov z18.s, #0xff\n"
    "smax z30.s, p0/M, z30.s, z16.s\n"
    "smax z29.s, p0/M, z29.s, z16.s\n"
    "smax z28.s, p0/M, z28.s, z16.s\n"
    "smax z27.s, p0/M, z27.s, z16.s\n"
    "smax z26.s, p0/M, z26.s, z16.s\n"
    "smax z25.s, p0/M, z25.s, z16.s\n"
    "smax z24.s, p0/M, z24.s, z16.s\n"
    "smax z23.s, p0/M, z23.s, z16.s\n"
    "smax z22.s, p0/M, z22.s, z16.s\n"
    "smax z21.s, p0/M, z21.s, z16.s\n"
    "smax z20.s, p0/M, z20.s, z16.s\n"
    "smax z19.s, p0/M, z19.s, z16.s\n"
    "smin z1.s, p0/M, z1.s, z18.s\n"
    "smin z17.s, p0/M, z17.s, z18.s\n"
    "trn1 z17.h, z1.h, z17.h\n"
    "smin z0.s, p0/M, z0.s, z18.s\n"
    "smin z31.s, p0/M, z31.s, z18.s\n"
    "trn1 z16.h, z0.h, z31.h\n"
    "trn1 z16.b, z17.b, z16.b\n"
    "smin z30.s, p0/M, z30.s, z18.s\n"
    "smin z29.s, p0/M, z29.s, z18.s\n"
    "trn1 z17.h, z30.h, z29.h\n"
    "st1b { z16.b }, p4, [%x[outptr], x9]\n"
    "smin z28.s, p0/M, z28.s, z18.s\n"
    "smin z27.s, p0/M, z27.s, z18.s\n"
    "trn1 z16.h, z28.h, z27.h\n"
    "trn1 z16.b, z17.b, z16.b\n"
    "smin z26.s, p0/M, z26.s, z18.s\n"
    "smin z25.s, p0/M, z25.s, z18.s\n"
    "trn1 z17.h, z26.h, z25.h\n"
    "st1b { z16.b }, p3, [%x[outptr], x28]\n"
    "smin z24.s, p0/M, z24.s, z18.s\n"
    "smin z23.s, p0/M, z23.s, z18.s\n"
    "trn1 z16.h, z24.h, z23.h\n"
    "trn1 z16.b, z17.b, z16.b\n"
    "smin z22.s, p0/M, z22.s, z18.s\n"
    "smin z21.s, p0/M, z21.s, z18.s\n"
    "trn1 z17.h, z22.h, z21.h\n"
    "st1b { z16.b }, p2, [%x[outptr], x27]\n"
    "smin z20.s, p0/M, z20.s, z18.s\n"
    "smin z19.s, p0/M, z19.s, z18.s\n"
    "trn1 z16.h, z20.h, z19.h\n"
    "trn1 z16.b, z17.b, z16.b\n"
    "st1b { z16.b }, p1, [%x[outptr], x26]\n"
    "incb x26, ALL, MUL #4\n"
    "whilelt p1.b, x26, %x[n_channels]\n"
    "incb x9, ALL, MUL #4\n"
    "incb x28, ALL, MUL #4\n"
    "incb x27, ALL, MUL #4\n"
    "b.any 1b\n"
    "7:"  // Single vector of channels
    "whilelt p4.b, x9, %x[n_channels]\n"
    "b.none 14f\n"
    "8:"  // Single vector of channels: Loop
    "lsr x25, %x[n_valid_cells], #0x2\n"
    "mov z8.b, #0x0\n"
    "mov x24, %x[inptrs]\n"
    "cbz x25, 11f\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "subs x25, x25, #0x1\n"
    "add x24, x24, #0x20\n"
    "ld1b { z4.b }, p4/Z, [x23, x9]\n"
    "ld1b { z3.b }, p4/Z, [x22, x9]\n"
    "ld1b { z2.b }, p4/Z, [x21, x9]\n"
    "ld1b { z1.b }, p4/Z, [x20, x9]\n"
    "beq 10f\n"
    "9:"  // Single vector of channels: Loop: 4 inputs loop
    "movprfx z16, z4\n umax z16.b, p0/M, z16.b, z3.b\n"
    "movprfx z17, z2\n umax z17.b, p0/M, z17.b, z1.b\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "umax z16.b, p0/M, z16.b, z17.b\n"
    "subs x25, x25, #0x1\n"
    "ld1b { z4.b }, p4/Z, [x23, x9]\n"
    "ld1b { z3.b }, p4/Z, [x22, x9]\n"
    "umax z8.b, p0/M, z8.b, z16.b\n"
    "add x24, x24, #0x20\n"
    "ld1b { z2.b }, p4/Z, [x21, x9]\n"
    "ld1b { z1.b }, p4/Z, [x20, x9]\n"
    "bgt 9b\n"
    "10:"  // Single vector of channels: Loop: 4 inputs tail
    "movprfx z16, z4\n umax z16.b, p0/M, z16.b, z3.b\n"
    "movprfx z17, z2\n umax z17.b, p0/M, z17.b, z1.b\n"
    "umax z16.b, p0/M, z16.b, z17.b\n"
    "umax z8.b, p0/M, z8.b, z16.b\n"
    "11:"  // Single vector of channels: Loop: After loop
    "ands x21, %x[n_valid_cells], #0x3\n"
    "beq 13f\n"
    "12:"  // Single vector of channels: Loop: Single input loop
    "ldr x20, [x24], #0x8\n"
    "ld1b { z16.b }, p4/Z, [x20, x9]\n"
    "subs x21, x21, #0x1\n"
    "umax z8.b, p0/M, z8.b, z16.b\n"
    "bgt 12b\n"
    "13:"  // Single vector of channels: Loop: Single input loop: End
    "add x20, %x[quant_params], %[offsetof_qp_input_offset]\n"
    "ld1rw { z18.s }, p0/Z, [x20]\n"
    ".inst 0x4508a911  // ushllb z17.h, z8.b, #0x0\n"
    ".inst 0x4508ad10  // ushllt z16.h, z8.b, #0x0\n"
    "neg z18.s, p0/M, z18.s\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_left_shift]\n"
    ".inst 0x45914255  // saddwb z21.s, z18.s, z17.h\n"
    ".inst 0x45914654  // saddwt z20.s, z18.s, z17.h\n"
    ".inst 0x45904253  // saddwb z19.s, z18.s, z16.h\n"
    ".inst 0x45904652  // saddwt z18.s, z18.s, z16.h\n"
    "ld1rw { z17.s }, p0/Z, [x20]\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_mul]\n"
    "ld1rw { z16.s }, p0/Z, [x20]\n"
    ".inst 0x44828235  // srshl z21.s, p0/M, z21.s, z17.s\n"
    ".inst 0x44828234  // srshl z20.s, p0/M, z20.s, z17.s\n"
    ".inst 0x04b076b5  // sqrdmulh z21.s, z21.s, z16.s\n"
    ".inst 0x44828233  // srshl z19.s, p0/M, z19.s, z17.s\n"
    ".inst 0x44828232  // srshl z18.s, p0/M, z18.s, z17.s\n"
    ".inst 0x04b07694  // sqrdmulh z20.s, z20.s, z16.s\n"
    ".inst 0x04b07673  // sqrdmulh z19.s, z19.s, z16.s\n"
    "add x20, %x[quant_params], %[offsetof_qp_per_layer_right_shift]\n"
    "ld1rw { z17.s }, p0/Z, [x20]\n"
    ".inst 0x04b07652  // sqrdmulh z18.s, z18.s, z16.s\n"
    "add x20, %x[quant_params], %[offsetof_qp_output_offset]\n"
    ".inst 0x44828235  // srshl z21.s, p0/M, z21.s, z17.s\n"
    ".inst 0x44828234  // srshl z20.s, p0/M, z20.s, z17.s\n"
    "ld1rw { z16.s }, p0/Z, [x20]\n"
    "add z21.s, z21.s, z16.s\n"
    ".inst 0x44828233  // srshl z19.s, p0/M, z19.s, z17.s\n"
    ".inst 0x44828232  // srshl z18.s, p0/M, z18.s, z17.s\n"
    "add z20.s, z20.s, z16.s\n"
    "add z19.s, z19.s, z16.s\n"
    "add z18.s, z18.s, z16.s\n"
    "mov z16.s, #0x0\n"
    "smax z21.s, p0/M, z21.s, z16.s\n"
    "smax z20.s, p0/M, z20.s, z16.s\n"
    "smax z19.s, p0/M, z19.s, z16.s\n"
    "smax z18.s, p0/M, z18.s, z16.s\n"
    "mov z16.s, #0xff\n"
    "smin z21.s, p0/M, z21.s, z16.s\n"
    "smin z20.s, p0/M, z20.s, z16.s\n"
    "trn1 z17.h, z21.h, z20.h\n"
    "smin z19.s, p0/M, z19.s, z16.s\n"
    "smin z18.s, p0/M, z18.s, z16.s\n"
    "trn1 z16.h, z19.h, z18.h\n"
    "trn1 z16.b, z17.b, z16.b\n"
    "st1b { z16.b }, p4, [%x[outptr], x9]\n"
    "incb x9\n"
    "whilelt p4.b, x9, %x[n_channels]\n"
    "b.any 8b\n"
    "14:"  // End
    :
    : [inptrs] "r" (inptrs), [n_channels] "r" (n_channels), [n_valid_cells] "r" (n_valid_cells), [offsetof_qp_input_offset] "I" (offsetof(Requantize32, input_offset)), [offsetof_qp_output_offset] "I" (offsetof(Requantize32, output_offset)), [offsetof_qp_per_layer_left_shift] "I" (offsetof(Requantize32, per_layer_left_shift)), [offsetof_qp_per_layer_mul] "I" (offsetof(Requantize32, per_layer_mul)), [offsetof_qp_per_layer_right_shift] "I" (offsetof(Requantize32, per_layer_right_shift)), [outptr] "r" (outptr), [quant_params] "r" (&qp)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "x9", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace pooling
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)
