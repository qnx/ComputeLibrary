/*
 * Copyright (c) 2021-2023 Arm Limited.
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

#include <cstddef>
#include <cstdint>

#if defined(__aarch64__) && defined(__ARM_FP16_ARGS) && defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)

namespace arm_conv {
namespace depthwise {

void a64_fp16_nhwc_generic_output9_mla_depthfirst_impl(
  const __fp16 *const *const inptrs,
  __fp16 *const *const outptrs,
  const void *params,
  const void *bias,
  const unsigned int n_points,
  const unsigned int n_channels,
  const __fp16 activation_min,
  const __fp16 activation_max
)
{
  const __fp16 minmax_vals[2] = { activation_min, activation_max };

  __asm__ __volatile__(
    "ld1r { v2.8h }, [%x[minmax_vals]]\n"
    "lsr x12, %x[n_channels], #0x3\n"
    "add x20, %x[minmax_vals], #0x2\n"
    "ld1r { v1.8h }, [x20]\n"
    "mov x11, #0x0\n"
    "cbz x12, 5f\n"
    "1:"  // Channel loop
    "movi v23.16b, #0x0\n"
    "cbz %x[bias], 2f\n"
    "ldr q23, [%x[bias], x11]\n"
    "2:"  // Channel loop: Load bias: Done
    "ldr q0, [%x[params], #0x0]\n"
    "mov x21, %x[inptrs]\n"
    "ldp x10, x9, [x21], #0x10\n"
    "subs x20, %x[n_points], #0x1\n"
    "ldr q14, [x10, x11]\n"
    "ldr q15, [x9, x11]\n"
    "mov v24.16b, v23.16b\n"
    "mov v25.16b, v23.16b\n"
    "ldp x28, x27, [x21], #0x10\n"
    "ldr q16, [x28, x11]\n"
    "mov v26.16b, v23.16b\n"
    "mov v27.16b, v23.16b\n"
    "ldr q17, [x27, x11]\n"
    "ldp x26, x25, [x21], #0x10\n"
    "mov v28.16b, v23.16b\n"
    "mov v29.16b, v23.16b\n"
    "ldr q18, [x26, x11]\n"
    "ldr q19, [x25, x11]\n"
    "mov v30.16b, v23.16b\n"
    "mov v31.16b, v23.16b\n"
    "ldp x24, x23, [x21], #0x10\n"
    "ldr q20, [x24, x11]\n"
    "add %x[params], %x[params], #0x10\n"
    "ldr q21, [x23, x11]\n"
    "ldr x22, [x21], #0x8\n"
    "ldr q22, [x22, x11]\n"
    "ble 4f\n"
    "3:"  // Channel loop: Planar loop
    "ldp x10, x9, [x21], #0x10\n"
    "ldp x28, x27, [x21], #0x10\n"
    "subs x20, x20, #0x1\n"
    "fmla v23.8h, v14.8h, v0.8h\n"
    "ldr q14, [x10, x11]\n"
    "ldp x26, x25, [x21], #0x10\n"
    "fmla v24.8h, v15.8h, v0.8h\n"
    "fmla v25.8h, v16.8h, v0.8h\n"
    "ldr q15, [x9, x11]\n"
    "ldr q16, [x28, x11]\n"
    "fmla v26.8h, v17.8h, v0.8h\n"
    "fmla v27.8h, v18.8h, v0.8h\n"
    "ldr q17, [x27, x11]\n"
    "ldr q18, [x26, x11]\n"
    "fmla v28.8h, v19.8h, v0.8h\n"
    "fmla v29.8h, v20.8h, v0.8h\n"
    "ldr q19, [x25, x11]\n"
    "ldp x24, x23, [x21], #0x10\n"
    "fmla v30.8h, v21.8h, v0.8h\n"
    "fmla v31.8h, v22.8h, v0.8h\n"
    "ldr q0, [%x[params], #0x0]\n"
    "ldr q20, [x24, x11]\n"
    "add %x[params], %x[params], #0x10\n"
    "ldr q21, [x23, x11]\n"
    "ldr x22, [x21], #0x8\n"
    "ldr q22, [x22, x11]\n"
    "bgt 3b\n"
    "4:"  // Channel loop: Planar tail
    "fmla v23.8h, v14.8h, v0.8h\n"
    "fmla v24.8h, v15.8h, v0.8h\n"
    "fmax v23.8h, v23.8h, v2.8h\n"
    "ldp x28, x27, [%x[outptrs], #0x0]\n"
    "fmla v25.8h, v16.8h, v0.8h\n"
    "fmla v26.8h, v17.8h, v0.8h\n"
    "fmax v24.8h, v24.8h, v2.8h\n"
    "ldp x26, x25, [%x[outptrs], #0x10]\n"
    "fmla v27.8h, v18.8h, v0.8h\n"
    "fmla v28.8h, v19.8h, v0.8h\n"
    "fmax v25.8h, v25.8h, v2.8h\n"
    "ldp x24, x23, [%x[outptrs], #0x20]\n"
    "fmla v29.8h, v20.8h, v0.8h\n"
    "fmla v30.8h, v21.8h, v0.8h\n"
    "fmax v26.8h, v26.8h, v2.8h\n"
    "ldp x22, x21, [%x[outptrs], #0x30]\n"
    "fmla v31.8h, v22.8h, v0.8h\n"
    "fmax v27.8h, v27.8h, v2.8h\n"
    "ldr x20, [%x[outptrs], #0x40]\n"
    "fmax v28.8h, v28.8h, v2.8h\n"
    "fmax v29.8h, v29.8h, v2.8h\n"
    "fmax v30.8h, v30.8h, v2.8h\n"
    "fmax v31.8h, v31.8h, v2.8h\n"
    "fmin v23.8h, v23.8h, v1.8h\n"
    "fmin v24.8h, v24.8h, v1.8h\n"
    "str q23, [x28, x11]\n"
    "fmin v25.8h, v25.8h, v1.8h\n"
    "fmin v26.8h, v26.8h, v1.8h\n"
    "str q24, [x27, x11]\n"
    "fmin v27.8h, v27.8h, v1.8h\n"
    "fmin v28.8h, v28.8h, v1.8h\n"
    "str q25, [x26, x11]\n"
    "fmin v29.8h, v29.8h, v1.8h\n"
    "fmin v30.8h, v30.8h, v1.8h\n"
    "str q26, [x25, x11]\n"
    "fmin v31.8h, v31.8h, v1.8h\n"
    "str q27, [x24, x11]\n"
    "str q28, [x23, x11]\n"
    "str q29, [x22, x11]\n"
    "str q30, [x21, x11]\n"
    "str q31, [x20, x11]\n"
    "add x11, x11, #0x10\n"
    "cmp x11, x12, LSL #4\n"
    "blt 1b\n"
    "5:"  // Oddments
    "tst %x[n_channels], #0x7\n"
    "beq 25f\n"
    "movi v23.16b, #0x0\n"
    "cbz %x[bias], 10f\n"
    "add x20, %x[bias], x11\n"
    "tbz %x[n_channels], #2, 7f\n"
    "ld1 { v23.d }[0], [x20], #0x8\n"
    "tbz %x[n_channels], #1, 6f\n"
    "ld1 { v23.s }[2], [x20], #0x4\n"
    "tbz %x[n_channels], #0, 9f\n"
    "ld1 { v23.h }[6], [x20], #0x2\n"
    "b 9f\n"
    "6:"  // Oddments: Load bias: Bit 2: Bit 1: Unset
    "tbz %x[n_channels], #0, 9f\n"
    "ld1 { v23.h }[4], [x20], #0x2\n"
    "b 9f\n"
    "7:"  // Oddments: Load bias: Bit 2: Unset
    "tbz %x[n_channels], #1, 8f\n"
    "ld1 { v23.s }[0], [x20], #0x4\n"
    "tbz %x[n_channels], #0, 9f\n"
    "ld1 { v23.h }[2], [x20], #0x2\n"
    "b 9f\n"
    "8:"  // Oddments: Load bias: Bit 2: Unset: Bit 1: Unset
    "ld1 { v23.h }[0], [x20], #0x2\n"
    "9:"  // Oddments: Load bias: Bit 2: End
    "10:"  // Oddments: Load bias: Done
    "ldr q0, [%x[params], #0x0]\n"
    "mov x21, %x[inptrs]\n"
    "ldp x10, x9, [x21], #0x10\n"
    "mov v24.16b, v23.16b\n"
    "ldp x28, x27, [x21], #0x10\n"
    "ldp x26, x25, [x21], #0x10\n"
    "mov v25.16b, v23.16b\n"
    "mov v26.16b, v23.16b\n"
    "ldp x24, x23, [x21], #0x10\n"
    "ldr x22, [x21], #0x8\n"
    "mov v27.16b, v23.16b\n"
    "mov v28.16b, v23.16b\n"
    "mov v29.16b, v23.16b\n"
    "mov v30.16b, v23.16b\n"
    "add x10, x10, x11\n"
    "add x9, x9, x11\n"
    "mov v31.16b, v23.16b\n"
    "add x28, x28, x11\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "add %x[params], %x[params], #0x10\n"
    "tbz %x[n_channels], #2, 12f\n"
    "ldr d14, [x10], #0x8\n"
    "ldr d15, [x9], #0x8\n"
    "ldr d16, [x28], #0x8\n"
    "ldr d17, [x27], #0x8\n"
    "ldr d18, [x26], #0x8\n"
    "ldr d19, [x25], #0x8\n"
    "ldr d20, [x24], #0x8\n"
    "ldr d21, [x23], #0x8\n"
    "ldr d22, [x22], #0x8\n"
    "tbz %x[n_channels], #1, 11f\n"
    "ld1 { v14.s }[2], [x10], #0x4\n"
    "ld1 { v15.s }[2], [x9], #0x4\n"
    "ld1 { v16.s }[2], [x28], #0x4\n"
    "ld1 { v17.s }[2], [x27], #0x4\n"
    "ld1 { v18.s }[2], [x26], #0x4\n"
    "ld1 { v19.s }[2], [x25], #0x4\n"
    "ld1 { v20.s }[2], [x24], #0x4\n"
    "ld1 { v21.s }[2], [x23], #0x4\n"
    "ld1 { v22.s }[2], [x22], #0x4\n"
    "tbz %x[n_channels], #0, 14f\n"
    "ld1 { v14.h }[6], [x10], #0x2\n"
    "ld1 { v15.h }[6], [x9], #0x2\n"
    "ld1 { v16.h }[6], [x28], #0x2\n"
    "ld1 { v17.h }[6], [x27], #0x2\n"
    "ld1 { v18.h }[6], [x26], #0x2\n"
    "ld1 { v19.h }[6], [x25], #0x2\n"
    "ld1 { v20.h }[6], [x24], #0x2\n"
    "ld1 { v21.h }[6], [x23], #0x2\n"
    "ld1 { v22.h }[6], [x22], #0x2\n"
    "b 14f\n"
    "11:"  // Oddments: Load: Bit 2: Bit 1: Unset
    "tbz %x[n_channels], #0, 14f\n"
    "ld1 { v14.h }[4], [x10], #0x2\n"
    "ld1 { v15.h }[4], [x9], #0x2\n"
    "ld1 { v16.h }[4], [x28], #0x2\n"
    "ld1 { v17.h }[4], [x27], #0x2\n"
    "ld1 { v18.h }[4], [x26], #0x2\n"
    "ld1 { v19.h }[4], [x25], #0x2\n"
    "ld1 { v20.h }[4], [x24], #0x2\n"
    "ld1 { v21.h }[4], [x23], #0x2\n"
    "ld1 { v22.h }[4], [x22], #0x2\n"
    "b 14f\n"
    "12:"  // Oddments: Load: Bit 2: Unset
    "tbz %x[n_channels], #1, 13f\n"
    "ldr s14, [x10], #0x4\n"
    "ldr s15, [x9], #0x4\n"
    "ldr s16, [x28], #0x4\n"
    "ldr s17, [x27], #0x4\n"
    "ldr s18, [x26], #0x4\n"
    "ldr s19, [x25], #0x4\n"
    "ldr s20, [x24], #0x4\n"
    "ldr s21, [x23], #0x4\n"
    "ldr s22, [x22], #0x4\n"
    "tbz %x[n_channels], #0, 14f\n"
    "ld1 { v14.h }[2], [x10], #0x2\n"
    "ld1 { v15.h }[2], [x9], #0x2\n"
    "ld1 { v16.h }[2], [x28], #0x2\n"
    "ld1 { v17.h }[2], [x27], #0x2\n"
    "ld1 { v18.h }[2], [x26], #0x2\n"
    "ld1 { v19.h }[2], [x25], #0x2\n"
    "ld1 { v20.h }[2], [x24], #0x2\n"
    "ld1 { v21.h }[2], [x23], #0x2\n"
    "ld1 { v22.h }[2], [x22], #0x2\n"
    "b 14f\n"
    "13:"  // Oddments: Load: Bit 2: Unset: Bit 1: Unset
    "ldr h14, [x10], #0x2\n"
    "ldr h15, [x9], #0x2\n"
    "ldr h16, [x28], #0x2\n"
    "ldr h17, [x27], #0x2\n"
    "ldr h18, [x26], #0x2\n"
    "ldr h19, [x25], #0x2\n"
    "ldr h20, [x24], #0x2\n"
    "ldr h21, [x23], #0x2\n"
    "ldr h22, [x22], #0x2\n"
    "14:"  // Oddments: Load: Bit 2: End
    "subs x20, %x[n_points], #0x1\n"
    "ble 20f\n"
    "15:"  // Oddments: Planar loop
    "ldp x10, x9, [x21], #0x10\n"
    "ldp x28, x27, [x21], #0x10\n"
    "fmla v23.8h, v14.8h, v0.8h\n"
    "fmla v24.8h, v15.8h, v0.8h\n"
    "ldp x26, x25, [x21], #0x10\n"
    "ldp x24, x23, [x21], #0x10\n"
    "fmla v25.8h, v16.8h, v0.8h\n"
    "fmla v26.8h, v17.8h, v0.8h\n"
    "ldr x22, [x21], #0x8\n"
    "fmla v27.8h, v18.8h, v0.8h\n"
    "fmla v28.8h, v19.8h, v0.8h\n"
    "add x10, x10, x11\n"
    "fmla v29.8h, v20.8h, v0.8h\n"
    "fmla v30.8h, v21.8h, v0.8h\n"
    "add x9, x9, x11\n"
    "add x28, x28, x11\n"
    "fmla v31.8h, v22.8h, v0.8h\n"
    "ldr q0, [%x[params], #0x0]\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "add %x[params], %x[params], #0x10\n"
    "tbz %x[n_channels], #2, 17f\n"
    "ldr d14, [x10], #0x8\n"
    "ldr d15, [x9], #0x8\n"
    "ldr d16, [x28], #0x8\n"
    "ldr d17, [x27], #0x8\n"
    "ldr d18, [x26], #0x8\n"
    "ldr d19, [x25], #0x8\n"
    "ldr d20, [x24], #0x8\n"
    "ldr d21, [x23], #0x8\n"
    "ldr d22, [x22], #0x8\n"
    "tbz %x[n_channels], #1, 16f\n"
    "ld1 { v14.s }[2], [x10], #0x4\n"
    "ld1 { v15.s }[2], [x9], #0x4\n"
    "ld1 { v16.s }[2], [x28], #0x4\n"
    "ld1 { v17.s }[2], [x27], #0x4\n"
    "ld1 { v18.s }[2], [x26], #0x4\n"
    "ld1 { v19.s }[2], [x25], #0x4\n"
    "ld1 { v20.s }[2], [x24], #0x4\n"
    "ld1 { v21.s }[2], [x23], #0x4\n"
    "ld1 { v22.s }[2], [x22], #0x4\n"
    "tbz %x[n_channels], #0, 19f\n"
    "ld1 { v14.h }[6], [x10], #0x2\n"
    "ld1 { v15.h }[6], [x9], #0x2\n"
    "ld1 { v16.h }[6], [x28], #0x2\n"
    "ld1 { v17.h }[6], [x27], #0x2\n"
    "ld1 { v18.h }[6], [x26], #0x2\n"
    "ld1 { v19.h }[6], [x25], #0x2\n"
    "ld1 { v20.h }[6], [x24], #0x2\n"
    "ld1 { v21.h }[6], [x23], #0x2\n"
    "ld1 { v22.h }[6], [x22], #0x2\n"
    "b 19f\n"
    "16:"  // Oddments: Planar loop: Load: Bit 2: Bit 1: Unset
    "tbz %x[n_channels], #0, 19f\n"
    "ld1 { v14.h }[4], [x10], #0x2\n"
    "ld1 { v15.h }[4], [x9], #0x2\n"
    "ld1 { v16.h }[4], [x28], #0x2\n"
    "ld1 { v17.h }[4], [x27], #0x2\n"
    "ld1 { v18.h }[4], [x26], #0x2\n"
    "ld1 { v19.h }[4], [x25], #0x2\n"
    "ld1 { v20.h }[4], [x24], #0x2\n"
    "ld1 { v21.h }[4], [x23], #0x2\n"
    "ld1 { v22.h }[4], [x22], #0x2\n"
    "b 19f\n"
    "17:"  // Oddments: Planar loop: Load: Bit 2: Unset
    "tbz %x[n_channels], #1, 18f\n"
    "ldr s14, [x10], #0x4\n"
    "ldr s15, [x9], #0x4\n"
    "ldr s16, [x28], #0x4\n"
    "ldr s17, [x27], #0x4\n"
    "ldr s18, [x26], #0x4\n"
    "ldr s19, [x25], #0x4\n"
    "ldr s20, [x24], #0x4\n"
    "ldr s21, [x23], #0x4\n"
    "ldr s22, [x22], #0x4\n"
    "tbz %x[n_channels], #0, 19f\n"
    "ld1 { v14.h }[2], [x10], #0x2\n"
    "ld1 { v15.h }[2], [x9], #0x2\n"
    "ld1 { v16.h }[2], [x28], #0x2\n"
    "ld1 { v17.h }[2], [x27], #0x2\n"
    "ld1 { v18.h }[2], [x26], #0x2\n"
    "ld1 { v19.h }[2], [x25], #0x2\n"
    "ld1 { v20.h }[2], [x24], #0x2\n"
    "ld1 { v21.h }[2], [x23], #0x2\n"
    "ld1 { v22.h }[2], [x22], #0x2\n"
    "b 19f\n"
    "18:"  // Oddments: Planar loop: Load: Bit 2: Unset: Bit 1: Unset
    "ldr h14, [x10], #0x2\n"
    "ldr h15, [x9], #0x2\n"
    "ldr h16, [x28], #0x2\n"
    "ldr h17, [x27], #0x2\n"
    "ldr h18, [x26], #0x2\n"
    "ldr h19, [x25], #0x2\n"
    "ldr h20, [x24], #0x2\n"
    "ldr h21, [x23], #0x2\n"
    "ldr h22, [x22], #0x2\n"
    "19:"  // Oddments: Planar loop: Load: Bit 2: End
    "subs x20, x20, #0x1\n"
    "bgt 15b\n"
    "20:"  // Oddments: Planar tail
    "fmla v23.8h, v14.8h, v0.8h\n"
    "fmla v24.8h, v15.8h, v0.8h\n"
    "fmax v23.8h, v23.8h, v2.8h\n"
    "ldp x28, x27, [%x[outptrs], #0x0]\n"
    "fmla v25.8h, v16.8h, v0.8h\n"
    "fmla v26.8h, v17.8h, v0.8h\n"
    "fmax v24.8h, v24.8h, v2.8h\n"
    "ldp x26, x25, [%x[outptrs], #0x10]\n"
    "fmla v27.8h, v18.8h, v0.8h\n"
    "fmla v28.8h, v19.8h, v0.8h\n"
    "fmax v25.8h, v25.8h, v2.8h\n"
    "ldp x24, x23, [%x[outptrs], #0x20]\n"
    "fmla v29.8h, v20.8h, v0.8h\n"
    "fmla v30.8h, v21.8h, v0.8h\n"
    "fmax v26.8h, v26.8h, v2.8h\n"
    "ldp x22, x21, [%x[outptrs], #0x30]\n"
    "fmla v31.8h, v22.8h, v0.8h\n"
    "fmax v27.8h, v27.8h, v2.8h\n"
    "ldr x20, [%x[outptrs], #0x40]\n"
    "add x28, x28, x11\n"
    "fmax v28.8h, v28.8h, v2.8h\n"
    "fmax v29.8h, v29.8h, v2.8h\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "fmax v30.8h, v30.8h, v2.8h\n"
    "fmax v31.8h, v31.8h, v2.8h\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "fmin v23.8h, v23.8h, v1.8h\n"
    "fmin v24.8h, v24.8h, v1.8h\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "fmin v25.8h, v25.8h, v1.8h\n"
    "fmin v26.8h, v26.8h, v1.8h\n"
    "add x21, x21, x11\n"
    "add x20, x20, x11\n"
    "fmin v27.8h, v27.8h, v1.8h\n"
    "fmin v28.8h, v28.8h, v1.8h\n"
    "fmin v29.8h, v29.8h, v1.8h\n"
    "fmin v30.8h, v30.8h, v1.8h\n"
    "fmin v31.8h, v31.8h, v1.8h\n"
    "tbz %x[n_channels], #2, 22f\n"
    "st1 { v23.d }[0], [x28], #0x8\n"
    "st1 { v24.d }[0], [x27], #0x8\n"
    "st1 { v25.d }[0], [x26], #0x8\n"
    "st1 { v26.d }[0], [x25], #0x8\n"
    "st1 { v27.d }[0], [x24], #0x8\n"
    "st1 { v28.d }[0], [x23], #0x8\n"
    "st1 { v29.d }[0], [x22], #0x8\n"
    "st1 { v30.d }[0], [x21], #0x8\n"
    "st1 { v31.d }[0], [x20], #0x8\n"
    "tbz %x[n_channels], #1, 21f\n"
    "st1 { v23.s }[2], [x28], #0x4\n"
    "st1 { v24.s }[2], [x27], #0x4\n"
    "st1 { v25.s }[2], [x26], #0x4\n"
    "st1 { v26.s }[2], [x25], #0x4\n"
    "st1 { v27.s }[2], [x24], #0x4\n"
    "st1 { v28.s }[2], [x23], #0x4\n"
    "st1 { v29.s }[2], [x22], #0x4\n"
    "st1 { v30.s }[2], [x21], #0x4\n"
    "st1 { v31.s }[2], [x20], #0x4\n"
    "tbz %x[n_channels], #0, 24f\n"
    "st1 { v23.h }[6], [x28], #0x2\n"
    "st1 { v24.h }[6], [x27], #0x2\n"
    "st1 { v25.h }[6], [x26], #0x2\n"
    "st1 { v26.h }[6], [x25], #0x2\n"
    "st1 { v27.h }[6], [x24], #0x2\n"
    "st1 { v28.h }[6], [x23], #0x2\n"
    "st1 { v29.h }[6], [x22], #0x2\n"
    "st1 { v30.h }[6], [x21], #0x2\n"
    "st1 { v31.h }[6], [x20], #0x2\n"
    "b 24f\n"
    "21:"  // Oddments: Store: Bit 2: Bit 1: Unset
    "tbz %x[n_channels], #0, 24f\n"
    "st1 { v23.h }[4], [x28], #0x2\n"
    "st1 { v24.h }[4], [x27], #0x2\n"
    "st1 { v25.h }[4], [x26], #0x2\n"
    "st1 { v26.h }[4], [x25], #0x2\n"
    "st1 { v27.h }[4], [x24], #0x2\n"
    "st1 { v28.h }[4], [x23], #0x2\n"
    "st1 { v29.h }[4], [x22], #0x2\n"
    "st1 { v30.h }[4], [x21], #0x2\n"
    "st1 { v31.h }[4], [x20], #0x2\n"
    "b 24f\n"
    "22:"  // Oddments: Store: Bit 2: Unset
    "tbz %x[n_channels], #1, 23f\n"
    "st1 { v23.s }[0], [x28], #0x4\n"
    "st1 { v24.s }[0], [x27], #0x4\n"
    "st1 { v25.s }[0], [x26], #0x4\n"
    "st1 { v26.s }[0], [x25], #0x4\n"
    "st1 { v27.s }[0], [x24], #0x4\n"
    "st1 { v28.s }[0], [x23], #0x4\n"
    "st1 { v29.s }[0], [x22], #0x4\n"
    "st1 { v30.s }[0], [x21], #0x4\n"
    "st1 { v31.s }[0], [x20], #0x4\n"
    "tbz %x[n_channels], #0, 24f\n"
    "st1 { v23.h }[2], [x28], #0x2\n"
    "st1 { v24.h }[2], [x27], #0x2\n"
    "st1 { v25.h }[2], [x26], #0x2\n"
    "st1 { v26.h }[2], [x25], #0x2\n"
    "st1 { v27.h }[2], [x24], #0x2\n"
    "st1 { v28.h }[2], [x23], #0x2\n"
    "st1 { v29.h }[2], [x22], #0x2\n"
    "st1 { v30.h }[2], [x21], #0x2\n"
    "st1 { v31.h }[2], [x20], #0x2\n"
    "b 24f\n"
    "23:"  // Oddments: Store: Bit 2: Unset: Bit 1: Unset
    "st1 { v23.h }[0], [x28], #0x2\n"
    "st1 { v24.h }[0], [x27], #0x2\n"
    "st1 { v25.h }[0], [x26], #0x2\n"
    "st1 { v26.h }[0], [x25], #0x2\n"
    "st1 { v27.h }[0], [x24], #0x2\n"
    "st1 { v28.h }[0], [x23], #0x2\n"
    "st1 { v29.h }[0], [x22], #0x2\n"
    "st1 { v30.h }[0], [x21], #0x2\n"
    "st1 { v31.h }[0], [x20], #0x2\n"
    "24:"  // Oddments: Store: Bit 2: End

    "25:"  // End

    : [params] "+&r" (params)
    : [bias] "r" (bias), [inptrs] "r" (inptrs), [minmax_vals] "r" (minmax_vals), [n_channels] "r" ((uint64_t) n_channels), [n_points] "r" ((uint64_t) n_points), [outptrs] "r" (outptrs)
    : "cc", "memory", "v0", "v1", "v2", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "x9", "x10", "x11", "x12", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(__aarch64__) && defined(__ARM_FP16_ARGS) && defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)