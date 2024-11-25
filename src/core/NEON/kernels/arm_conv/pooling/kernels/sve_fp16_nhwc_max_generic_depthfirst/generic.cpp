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

#include <cstdint>
#include <cstddef>

#if defined(ARM_COMPUTE_ENABLE_SVE) && defined(__ARM_FP16_ARGS)

namespace arm_conv {
namespace pooling {


void sve_fp16_nhwc_max_generic_depthfirst_impl(
  const uint64_t,
  const uint64_t n_valid_cells,
  uint64_t n_channels,
  const __fp16 *const *const inptrs,
  __fp16 *outptr
)
{
  __asm__ __volatile__(
    "mov x9, #0x0\n"
    "cnth x28\n"
    "cnth x27, ALL, MUL #2\n"
    "cnth x26, ALL, MUL #3\n"
    "ptrue p4.b\n"
    "whilelt p3.h, x9, %x[n_channels]\n"
    "whilelt p2.h, x28, %x[n_channels]\n"
    "whilelt p1.h, x27, %x[n_channels]\n"
    "whilelt p0.h, x26, %x[n_channels]\n"
    "b.none 7f\n"
    "1:"  // 4-vectors of channels
    "lsr x25, %x[n_valid_cells], #0x2\n"
    "mov z6.h, #0xfc00\n"
    "mov z5.h, #0xfc00\n"
    "mov x24, %x[inptrs]\n"
    "mov z4.h, #0xfc00\n"
    "mov z3.h, #0xfc00\n"
    "cbz x25, 4f\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "subs x25, x25, #0x1\n"
    "add x24, x24, #0x20\n"
    "ld1h { z2.h }, p3/Z, [x23, x9, LSL #1]\n"
    "ld1h { z1.h }, p3/Z, [x22, x9, LSL #1]\n"
    "ld1h { z23.h }, p3/Z, [x21, x9, LSL #1]\n"
    "ld1h { z0.h }, p3/Z, [x20, x9, LSL #1]\n"
    "ld1h { z31.h }, p2/Z, [x23, x28, LSL #1]\n"
    "ld1h { z30.h }, p2/Z, [x22, x28, LSL #1]\n"
    "ld1h { z22.h }, p2/Z, [x21, x28, LSL #1]\n"
    "ld1h { z29.h }, p2/Z, [x20, x28, LSL #1]\n"
    "ld1h { z28.h }, p1/Z, [x23, x27, LSL #1]\n"
    "ld1h { z27.h }, p1/Z, [x22, x27, LSL #1]\n"
    "ld1h { z21.h }, p1/Z, [x21, x27, LSL #1]\n"
    "ld1h { z26.h }, p1/Z, [x20, x27, LSL #1]\n"
    "ld1h { z16.h }, p0/Z, [x23, x26, LSL #1]\n"
    "ld1h { z25.h }, p0/Z, [x22, x26, LSL #1]\n"
    "ld1h { z20.h }, p0/Z, [x21, x26, LSL #1]\n"
    "ld1h { z24.h }, p0/Z, [x20, x26, LSL #1]\n"
    "beq 3f\n"
    "2:"  // 4-vectors of channels: 4 inputs loop
    "movprfx z19, z2\n fmax z19.h, p4/M, z19.h, z1.h\n"
    "fmax z23.h, p4/M, z23.h, z0.h\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "movprfx z18, z31\n fmax z18.h, p4/M, z18.h, z30.h\n"
    "fmax z22.h, p4/M, z22.h, z29.h\n"
    "movprfx z17, z28\n fmax z17.h, p4/M, z17.h, z27.h\n"
    "fmax z21.h, p4/M, z21.h, z26.h\n"
    "fmax z16.h, p4/M, z16.h, z25.h\n"
    "fmax z20.h, p4/M, z20.h, z24.h\n"
    "ld1h { z2.h }, p3/Z, [x23, x9, LSL #1]\n"
    "ld1h { z1.h }, p3/Z, [x22, x9, LSL #1]\n"
    "fmax z19.h, p4/M, z19.h, z23.h\n"
    "fmax z18.h, p4/M, z18.h, z22.h\n"
    "ld1h { z23.h }, p3/Z, [x21, x9, LSL #1]\n"
    "ld1h { z0.h }, p3/Z, [x20, x9, LSL #1]\n"
    "fmax z17.h, p4/M, z17.h, z21.h\n"
    "subs x25, x25, #0x1\n"
    "ld1h { z31.h }, p2/Z, [x23, x28, LSL #1]\n"
    "ld1h { z30.h }, p2/Z, [x22, x28, LSL #1]\n"
    "fmax z16.h, p4/M, z16.h, z20.h\n"
    "add x24, x24, #0x20\n"
    "ld1h { z22.h }, p2/Z, [x21, x28, LSL #1]\n"
    "ld1h { z29.h }, p2/Z, [x20, x28, LSL #1]\n"
    "fmax z6.h, p4/M, z6.h, z19.h\n"
    "fmax z5.h, p4/M, z5.h, z18.h\n"
    "ld1h { z28.h }, p1/Z, [x23, x27, LSL #1]\n"
    "ld1h { z27.h }, p1/Z, [x22, x27, LSL #1]\n"
    "fmax z4.h, p4/M, z4.h, z17.h\n"
    "ld1h { z21.h }, p1/Z, [x21, x27, LSL #1]\n"
    "ld1h { z26.h }, p1/Z, [x20, x27, LSL #1]\n"
    "fmax z3.h, p4/M, z3.h, z16.h\n"
    "ld1h { z16.h }, p0/Z, [x23, x26, LSL #1]\n"
    "ld1h { z25.h }, p0/Z, [x22, x26, LSL #1]\n"
    "ld1h { z20.h }, p0/Z, [x21, x26, LSL #1]\n"
    "ld1h { z24.h }, p0/Z, [x20, x26, LSL #1]\n"
    "bgt 2b\n"
    "3:"  // 4-vectors of channels: 4 inputs tail
    "movprfx z19, z2\n fmax z19.h, p4/M, z19.h, z1.h\n"
    "fmax z23.h, p4/M, z23.h, z0.h\n"
    "movprfx z18, z31\n fmax z18.h, p4/M, z18.h, z30.h\n"
    "fmax z22.h, p4/M, z22.h, z29.h\n"
    "movprfx z17, z28\n fmax z17.h, p4/M, z17.h, z27.h\n"
    "fmax z21.h, p4/M, z21.h, z26.h\n"
    "fmax z16.h, p4/M, z16.h, z25.h\n"
    "fmax z20.h, p4/M, z20.h, z24.h\n"
    "fmax z19.h, p4/M, z19.h, z23.h\n"
    "fmax z18.h, p4/M, z18.h, z22.h\n"
    "fmax z17.h, p4/M, z17.h, z21.h\n"
    "fmax z16.h, p4/M, z16.h, z20.h\n"
    "fmax z6.h, p4/M, z6.h, z19.h\n"
    "fmax z5.h, p4/M, z5.h, z18.h\n"
    "fmax z4.h, p4/M, z4.h, z17.h\n"
    "fmax z3.h, p4/M, z3.h, z16.h\n"
    "4:"  // 4-vectors of channels: After loop
    "ands x21, %x[n_valid_cells], #0x3\n"
    "beq 6f\n"
    "5:"  // 4-vectors of channels: Single input loop
    "ldr x20, [x24], #0x8\n"
    "subs x21, x21, #0x1\n"
    "ld1h { z19.h }, p3/Z, [x20, x9, LSL #1]\n"
    "ld1h { z18.h }, p2/Z, [x20, x28, LSL #1]\n"
    "ld1h { z17.h }, p1/Z, [x20, x27, LSL #1]\n"
    "ld1h { z16.h }, p0/Z, [x20, x26, LSL #1]\n"
    "fmax z6.h, p4/M, z6.h, z19.h\n"
    "fmax z5.h, p4/M, z5.h, z18.h\n"
    "fmax z4.h, p4/M, z4.h, z17.h\n"
    "fmax z3.h, p4/M, z3.h, z16.h\n"
    "bgt 5b\n"
    "6:"  // 4-vectors of channels: Single input loop: End
    "st1h { z6.h }, p3, [%x[outptr], x9, LSL #1]\n"
    "inch x9, ALL, MUL #4\n"
    "st1h { z5.h }, p2, [%x[outptr], x28, LSL #1]\n"
    "inch x28, ALL, MUL #4\n"
    "st1h { z4.h }, p1, [%x[outptr], x27, LSL #1]\n"
    "inch x27, ALL, MUL #4\n"
    "st1h { z3.h }, p0, [%x[outptr], x26, LSL #1]\n"
    "inch x26, ALL, MUL #4\n"
    "whilelt p0.h, x26, %x[n_channels]\n"
    "b.any 1b\n"
    "7:"  // Single vector of channels
    "whilelt p3.h, x9, %x[n_channels]\n"
    "b.none 14f\n"
    "8:"  // Single vector of channels: Loop
    "lsr x25, %x[n_valid_cells], #0x2\n"
    "mov z6.h, #0xfc00\n"
    "mov x24, %x[inptrs]\n"
    "cbz x25, 11f\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "subs x25, x25, #0x1\n"
    "add x24, x24, #0x20\n"
    "ld1h { z2.h }, p3/Z, [x23, x9, LSL #1]\n"
    "ld1h { z1.h }, p3/Z, [x22, x9, LSL #1]\n"
    "ld1h { z23.h }, p3/Z, [x21, x9, LSL #1]\n"
    "ld1h { z0.h }, p3/Z, [x20, x9, LSL #1]\n"
    "beq 10f\n"
    "9:"  // Single vector of channels: Loop: 4 inputs loop
    "movprfx z16, z2\n fmax z16.h, p4/M, z16.h, z1.h\n"
    "movprfx z17, z23\n fmax z17.h, p4/M, z17.h, z0.h\n"
    "ldp x23, x22, [x24, #0x0]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "subs x25, x25, #0x1\n"
    "add x24, x24, #0x20\n"
    "fmax z16.h, p4/M, z16.h, z17.h\n"
    "ld1h { z2.h }, p3/Z, [x23, x9, LSL #1]\n"
    "ld1h { z1.h }, p3/Z, [x22, x9, LSL #1]\n"
    "ld1h { z23.h }, p3/Z, [x21, x9, LSL #1]\n"
    "ld1h { z0.h }, p3/Z, [x20, x9, LSL #1]\n"
    "fmax z6.h, p4/M, z6.h, z16.h\n"
    "bgt 9b\n"
    "10:"  // Single vector of channels: Loop: 4 inputs tail
    "movprfx z16, z2\n fmax z16.h, p4/M, z16.h, z1.h\n"
    "movprfx z17, z23\n fmax z17.h, p4/M, z17.h, z0.h\n"
    "fmax z16.h, p4/M, z16.h, z17.h\n"
    "fmax z6.h, p4/M, z6.h, z16.h\n"
    "11:"  // Single vector of channels: Loop: After loop
    "ands x21, %x[n_valid_cells], #0x3\n"
    "beq 13f\n"
    "12:"  // Single vector of channels: Loop: Single input loop
    "ldr x20, [x24], #0x8\n"
    "subs x21, x21, #0x1\n"
    "ld1h { z16.h }, p3/Z, [x20, x9, LSL #1]\n"
    "fmax z6.h, p4/M, z6.h, z16.h\n"
    "bgt 12b\n"
    "13:"  // Single vector of channels: Loop: Single input loop: End
    "st1h { z6.h }, p3, [%x[outptr], x9, LSL #1]\n"
    "inch x9\n"
    "whilelt p3.h, x9, %x[n_channels]\n"
    "b.any 8b\n"
    "14:"  // End
    :
    : [inptrs] "r" (inptrs), [n_channels] "r" (n_channels), [n_valid_cells] "r" (n_valid_cells), [outptr] "r" (outptr)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "x9", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace pooling
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE) && defined(__ARM_FP16_ARGS)
