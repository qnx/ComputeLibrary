/*
 * Copyright (c) 2023-2024 Arm Limited.
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

#if defined(ARM_COMPUTE_ENABLE_SME2)

#include "arm_gemm.hpp"
#include "../../utils.hpp"

#include <cassert>
#include <limits>

namespace arm_gemm {

void sme2_gemv_fp16fp32fp16_dot_16VL (
    const __fp16 *A_ptr, const __fp16 *B_ptr, __fp16 *output_ptr,
    size_t N, size_t K,
    const __fp16 *bias, Activation act, bool
)
{
    struct KernelArgs {
        __fp16 maxval = static_cast<__fp16>(std::numeric_limits<float>::infinity());
        __fp16 minval = - static_cast<__fp16>(std::numeric_limits<float>::infinity());
        const __fp16 *B_ptr = {};
        size_t output_offset = {};
        unsigned int input_initial_col = {};
    } ka;

    unsigned long flags=0;
    ka.B_ptr = B_ptr;
    switch(act.type) {
        default:
        case Activation::Type::None:
            break;
        case Activation::Type::BoundedReLU:
            ka.maxval = static_cast<__fp16>(act.param1);
            /* fall through */
        case Activation::Type::ReLU:
            ka.minval = 0;
            flags |= 0x2;
            break;
    }
    __asm__ __volatile__(
      "ptrue p8.b\n"
      ".inst 0xd503477f  // SMSTART ZA\n"
      "mov x9, #0x0\n"
      "cntw x28, ALL, MUL #4\n"
      "mov x27, %x[B_ptr]\n"
      "add x26, %x[N], x28\n"
      "mov x25, %x[output_ptr]\n"
      "sub x26, x26, #0x1\n"
      "ptrue p1.b\n"
      "udiv x26, x26, x28\n"
      ".inst 0x25207811  // ptrue pn9.b\n"
      "add x22, x26, #0x3\n"
      "mov x21, #0x1\n"
      "and x22, x22, #0xfffffffffffffffc\n"
      "mul x22, x22, x28\n"
      "mul x22, x22, %x[K]\n"
      "lsl x22, x22, #0x1\n"
      "1:"  // RHS size check loop
      "cmp x22, #0x200000\n"
      "blt 2f\n"
      "tbnz x22, #0, 3f\n"
      "lsr x22, x22, #0x1\n"
      "lsl x21, x21, #0x1\n"
      "b 1b\n"
      "2:"  // RHS do prefetch
      "lsl x20, x22, #0x26\n"
      "sub x21, x21, #0x1\n"
      "lsl x21, x21, #0x16\n"
      "orr x22, x22, x20\n"
      "orr x22, x22, x21\n"
      ".inst 0xf8b64b7a  // rprfm pldonce, x22, [x27]\n"
      "3:"  // RHS prefetch exit
      "mov x24, %x[bias]\n"
      "4:"  // Column loop
      "cmp x26, #0x4\n"
      "bge 28f\n"
      "cmp x26, #0x2\n"
      "bgt 20f\n"
      "beq 12f\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x1\n"
      "mov x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x257447f0  // whilelt p8.h, XZR, x20, VLx2\n"
      "cbz x24, 5f\n"
      "ld1h { z20.s }, p1/Z, [x24]\n"
      "addvl x20, x24, #4\n"
      "ld1h { z21.s }, p1/Z, [x24, #1, MUL VL]\n"
      "ld1h { z22.s }, p1/Z, [x24, #2, MUL VL]\n"
      "ld1h { z23.s }, p1/Z, [x24, #3, MUL VL]\n"
      "fcvt z20.s, p1/m, z20.h\n"
      "fcvt z21.s, p1/m, z21.h\n"
      "fcvt z22.s, p1/m, z22.h\n"
      "fcvt z23.s, p1/m, z23.h\n"
      ".inst 0xc0042e80  // mova za.d[x9, #0], { z20.d-z23.d }\n"
      "b 6f\n"
      "5:"  // Width 1: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "6:"  // Width 1: setup done
      "cmp x22, #0x8\n"
      "ble 8f\n"
      "7:"  // Width 1: Multiply loop: Main loop head
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27]\n"
      "addvl x27, x27, #16\n"
      "ld1rqh { z0.h }, p0/Z, [x23]\n"
      "sub x22, x22, #0x8\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa040a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27]\n"
      "addvl x27, x27, #16\n"
      "cmp x22, #0x8\n"
      ".inst 0xa040a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc150b208  // fdot za.s[x9, 0], { z16.h-z19.h }, z0.h[0]\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc150b788  // fdot za.s[x9, 0], { z28.h-z31.h }, z0.h[1]\n"
      ".inst 0xc150bb08  // fdot za.s[x9, 0], { z24.h-z27.h }, z0.h[2]\n"
      ".inst 0xc150bc88  // fdot za.s[x9, 0], { z4.h-z7.h }, z0.h[3]\n"
      "bgt 7b\n"
      "8:"  // Width 1: Multiply loop: Single iteration only
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "ld1rqh { z11.h }, p0/Z, [x23]\n"
      "add x23, x23, #0x10\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb308  // fdot za.s[x9, 0], { z24.h-z27.h }, z11.h[0]\n"
      "ble 9f\n"
      ".inst 0xa040a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb708  // fdot za.s[x9, 0], { z24.h-z27.h }, z11.h[1]\n"
      "ble 9f\n"
      ".inst 0xa040a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbb08  // fdot za.s[x9, 0], { z24.h-z27.h }, z11.h[2]\n"
      "ble 9f\n"
      ".inst 0xa040a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbf88  // fdot za.s[x9, 0], { z28.h-z31.h }, z11.h[3]\n"
      "9:"  // Width 1: Multiply loop: multiply skip
      "tbz %x[flags], #1, 10f\n"
      ".inst 0xc0062c10  // mova { z16.d-z19.d }, za.d[x9, #0]\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      "ld1rh { z29.h }, p1/Z, [x21]\n"
      "ld1rh { z20.h }, p1/Z, [x20]\n"
      ".inst 0xc120e204  // fcvt z4.h, { z16.s-z17.s }\n"
      ".inst 0xc120e245  // fcvt z5.h, { z18.s-z19.s }\n"
      ".inst 0xc174c3a4  // fclamp { z4.h-z5.h }, z29.h, z20.h\n"
      ".inst 0xa0602324  // st1h { z4.h-z5.h }, p8, [x25]\n"
      "addvl x25, x25, #2\n"
      "b 11f\n"
      "10:"  // Width 1: No activation
      ".inst 0xc0062c00  // mova { z0.d-z3.d }, za.d[x9, #0]\n"
      ".inst 0xc120e012  // fcvt z18.h, { z0.s-z1.s }\n"
      ".inst 0xc120e05a  // fcvt z26.h, { z2.s-z3.s }\n"
      ".inst 0xa1602332  // st1h { z18.h, z26.h }, p8, [x25]\n"
      "addvl x25, x25, #2\n"
      "11:"  // Width 1: Output done
      "b 36f\n"
      "12:"  // Width 2
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x1\n"
      "sub x20, %x[N], x28\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x257447f0  // whilelt p8.h, XZR, x20, VLx2\n"
      "cbz x24, 13f\n"
      "ld1h { z12.s }, p1/Z, [x24]\n"
      "addvl x20, x24, #4\n"
      "ld1h { z13.s }, p1/Z, [x24, #1, MUL VL]\n"
      "ld1h { z14.s }, p1/Z, [x24, #2, MUL VL]\n"
      "ld1h { z15.s }, p1/Z, [x24, #3, MUL VL]\n"
      "fcvt z12.s, p1/m, z12.h\n"
      "ld1h { z28.s }, p1/Z, [x24, #4, MUL VL]\n"
      "fcvt z13.s, p1/m, z13.h\n"
      "ld1h { z29.s }, p1/Z, [x24, #5, MUL VL]\n"
      "fcvt z14.s, p1/m, z14.h\n"
      "ld1h { z30.s }, p1/Z, [x24, #6, MUL VL]\n"
      "fcvt z15.s, p1/m, z15.h\n"
      "ld1h { z31.s }, p1/Z, [x24, #7, MUL VL]\n"
      "fcvt z28.s, p1/m, z28.h\n"
      "fcvt z29.s, p1/m, z29.h\n"
      "fcvt z30.s, p1/m, z30.h\n"
      "fcvt z31.s, p1/m, z31.h\n"
      ".inst 0xc0042d80  // mova za.d[x9, #0], { z12.d-z15.d }\n"
      ".inst 0xc0042f81  // mova za.d[x9, #1], { z28.d-z31.d }\n"
      "b 14f\n"
      "13:"  // Width 2: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "14:"  // Width 2: setup done
      "cmp x22, #0x8\n"
      "ble 16f\n"
      "15:"  // Width 2: Multiply loop: Main loop head
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      "sub x22, x22, #0x8\n"
      "ld1rqh { z8.h }, p0/Z, [x23]\n"
      "cmp x22, #0x8\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xa040a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc158b088  // fdot za.s[x9, 0], { z4.h-z7.h }, z8.h[0]\n"
      ".inst 0xa041a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc158b009  // fdot za.s[x9, 1], { z0.h-z3.h }, z8.h[0]\n"
      ".inst 0xa040a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      ".inst 0xa041a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc158b608  // fdot za.s[x9, 0], { z16.h-z19.h }, z8.h[1]\n"
      ".inst 0xa041a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc158b589  // fdot za.s[x9, 1], { z12.h-z15.h }, z8.h[1]\n"
      ".inst 0xc158bb08  // fdot za.s[x9, 0], { z24.h-z27.h }, z8.h[2]\n"
      ".inst 0xc158b809  // fdot za.s[x9, 1], { z0.h-z3.h }, z8.h[2]\n"
      ".inst 0xc158bc88  // fdot za.s[x9, 0], { z4.h-z7.h }, z8.h[3]\n"
      ".inst 0xc158bf89  // fdot za.s[x9, 1], { z28.h-z31.h }, z8.h[3]\n"
      "bgt 15b\n"
      "16:"  // Width 2: Multiply loop: Single iteration only
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "ld1rqh { z11.h }, p0/Z, [x23]\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb088  // fdot za.s[x9, 0], { z4.h-z7.h }, z11.h[0]\n"
      ".inst 0xc15bb189  // fdot za.s[x9, 1], { z12.h-z15.h }, z11.h[0]\n"
      "ble 17f\n"
      ".inst 0xa040a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb608  // fdot za.s[x9, 0], { z16.h-z19.h }, z11.h[1]\n"
      ".inst 0xc15bb689  // fdot za.s[x9, 1], { z20.h-z23.h }, z11.h[1]\n"
      "ble 17f\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb988  // fdot za.s[x9, 0], { z12.h-z15.h }, z11.h[2]\n"
      ".inst 0xc15bba89  // fdot za.s[x9, 1], { z20.h-z23.h }, z11.h[2]\n"
      "ble 17f\n"
      ".inst 0xa040a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27]\n"
      ".inst 0xa041a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbc08  // fdot za.s[x9, 0], { z0.h-z3.h }, z11.h[3]\n"
      ".inst 0xc15bbf09  // fdot za.s[x9, 1], { z24.h-z27.h }, z11.h[3]\n"
      "17:"  // Width 2: Multiply loop: multiply skip
      "tbz %x[flags], #1, 18f\n"
      ".inst 0xc0062c0c  // mova { z12.d-z15.d }, za.d[x9, #0]\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c3c  // mova { z28.d-z31.d }, za.d[x9, #1]\n"
      "ld1rh { z5.h }, p1/Z, [x21]\n"
      "ld1rh { z21.h }, p1/Z, [x20]\n"
      ".inst 0xc120e188  // fcvt z8.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1c9  // fcvt z9.h, { z14.s-z15.s }\n"
      ".inst 0xc120e39c  // fcvt z28.h, { z28.s-z29.s }\n"
      ".inst 0xc120e3dd  // fcvt z29.h, { z30.s-z31.s }\n"
      ".inst 0xc175c0a8  // fclamp { z8.h-z9.h }, z5.h, z21.h\n"
      ".inst 0xc175c0bc  // fclamp { z28.h-z29.h }, z5.h, z21.h\n"
      ".inst 0xa0602728  // st1h { z8.h-z9.h }, pn9.b, [x25]\n"
      ".inst 0xa061233c  // st1h { z28.h-z29.h }, p8, [x25, #0x2, MUL VL]\n"
      "addvl x25, x25, #4\n"
      "b 19f\n"
      "18:"  // Width 2: No activation
      ".inst 0xc0062c0c  // mova { z12.d-z15.d }, za.d[x9, #0]\n"
      ".inst 0xc0062c24  // mova { z4.d-z7.d }, za.d[x9, #1]\n"
      ".inst 0xc120e194  // fcvt z20.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1dc  // fcvt z28.h, { z14.s-z15.s }\n"
      ".inst 0xa1602734  // st1h { z20.h, z28.h }, pn9.b, [x25]\n"
      ".inst 0xc120e09a  // fcvt z26.h, { z4.s-z5.s }\n"
      ".inst 0xc120e0db  // fcvt z27.h, { z6.s-z7.s }\n"
      ".inst 0xa061233a  // st1h { z26.h-z27.h }, p8, [x25, #0x2, MUL VL]\n"
      "addvl x25, x25, #4\n"
      "19:"  // Width 2: Output done
      "b 36f\n"
      "20:"  // Width 3
      "mov x20, #0x2\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x1\n"
      "msub x20, x28, x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x257447f0  // whilelt p8.h, XZR, x20, VLx2\n"
      "cbz x24, 21f\n"
      "addvl x20, x24, #4\n"
      "ld1h { z16.s }, p1/Z, [x24]\n"
      "ld1h { z17.s }, p1/Z, [x24, #1, MUL VL]\n"
      "ld1h { z18.s }, p1/Z, [x24, #2, MUL VL]\n"
      "ld1h { z19.s }, p1/Z, [x24, #3, MUL VL]\n"
      "fcvt z16.s, p1/m, z16.h\n"
      "ld1h { z8.s }, p1/Z, [x24, #4, MUL VL]\n"
      "fcvt z17.s, p1/m, z17.h\n"
      "ld1h { z9.s }, p1/Z, [x24, #5, MUL VL]\n"
      "fcvt z18.s, p1/m, z18.h\n"
      "ld1h { z10.s }, p1/Z, [x24, #6, MUL VL]\n"
      "fcvt z19.s, p1/m, z19.h\n"
      "ld1h { z11.s }, p1/Z, [x24, #7, MUL VL]\n"
      "fcvt z8.s, p1/m, z8.h\n"
      "ld1h { z24.s }, p1/Z, [x20]\n"
      "fcvt z9.s, p1/m, z9.h\n"
      "ld1h { z25.s }, p1/Z, [x20, #1, MUL VL]\n"
      "fcvt z10.s, p1/m, z10.h\n"
      "ld1h { z26.s }, p1/Z, [x20, #2, MUL VL]\n"
      "fcvt z11.s, p1/m, z11.h\n"
      ".inst 0xc0042e00  // mova za.d[x9, #0], { z16.d-z19.d }\n"
      "ld1h { z27.s }, p1/Z, [x20, #3, MUL VL]\n"
      "fcvt z24.s, p1/m, z24.h\n"
      "fcvt z25.s, p1/m, z25.h\n"
      "fcvt z26.s, p1/m, z26.h\n"
      "fcvt z27.s, p1/m, z27.h\n"
      ".inst 0xc0042d01  // mova za.d[x9, #1], { z8.d-z11.d }\n"
      ".inst 0xc0042f02  // mova za.d[x9, #2], { z24.d-z27.d }\n"
      "b 22f\n"
      "21:"  // Width 3: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "22:"  // Width 3: setup done
      "cmp x22, #0x8\n"
      "ble 24f\n"
      "23:"  // Width 3: Multiply loop: Main loop head
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27]\n"
      "sub x22, x22, #0x8\n"
      "ld1rqh { z6.h }, p0/Z, [x23]\n"
      "cmp x22, #0x8\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc156b288  // fdot za.s[x9, 0], { z20.h-z23.h }, z6.h[0]\n"
      ".inst 0xa040a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc156b189  // fdot za.s[x9, 1], { z12.h-z15.h }, z6.h[0]\n"
      ".inst 0xa041a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc156b00a  // fdot za.s[x9, 2], { z0.h-z3.h }, z6.h[0]\n"
      ".inst 0xa042a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xa040a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc156b788  // fdot za.s[x9, 0], { z28.h-z31.h }, z6.h[1]\n"
      ".inst 0xa041a769  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc156b589  // fdot za.s[x9, 1], { z12.h-z15.h }, z6.h[1]\n"
      ".inst 0xa042a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc156b40a  // fdot za.s[x9, 2], { z0.h-z3.h }, z6.h[1]\n"
      ".inst 0xa040a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27]\n"
      ".inst 0xa041a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc156ba88  // fdot za.s[x9, 0], { z20.h-z23.h }, z6.h[2]\n"
      ".inst 0xa042a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc156b909  // fdot za.s[x9, 1], { z8.h-z11.h }, z6.h[2]\n"
      ".inst 0xc156b98a  // fdot za.s[x9, 2], { z12.h-z15.h }, z6.h[2]\n"
      ".inst 0xc156bc08  // fdot za.s[x9, 0], { z0.h-z3.h }, z6.h[3]\n"
      ".inst 0xc156be09  // fdot za.s[x9, 1], { z16.h-z19.h }, z6.h[3]\n"
      ".inst 0xc156be8a  // fdot za.s[x9, 2], { z20.h-z23.h }, z6.h[3]\n"
      "bgt 23b\n"
      "24:"  // Width 3: Multiply loop: Single iteration only
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "ld1rqh { z11.h }, p0/Z, [x23]\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb188  // fdot za.s[x9, 0], { z12.h-z15.h }, z11.h[0]\n"
      ".inst 0xc15bb009  // fdot za.s[x9, 1], { z0.h-z3.h }, z11.h[0]\n"
      ".inst 0xc15bb20a  // fdot za.s[x9, 2], { z16.h-z19.h }, z11.h[0]\n"
      "ble 25f\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb588  // fdot za.s[x9, 0], { z12.h-z15.h }, z11.h[1]\n"
      ".inst 0xc15bb609  // fdot za.s[x9, 1], { z16.h-z19.h }, z11.h[1]\n"
      ".inst 0xc15bb68a  // fdot za.s[x9, 2], { z20.h-z23.h }, z11.h[1]\n"
      "ble 25f\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb888  // fdot za.s[x9, 0], { z4.h-z7.h }, z11.h[2]\n"
      ".inst 0xc15bbb89  // fdot za.s[x9, 1], { z28.h-z31.h }, z11.h[2]\n"
      ".inst 0xc15bba8a  // fdot za.s[x9, 2], { z20.h-z23.h }, z11.h[2]\n"
      "ble 25f\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      ".inst 0xa041a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbc88  // fdot za.s[x9, 0], { z4.h-z7.h }, z11.h[3]\n"
      ".inst 0xc15bbf89  // fdot za.s[x9, 1], { z28.h-z31.h }, z11.h[3]\n"
      ".inst 0xc15bbd8a  // fdot za.s[x9, 2], { z12.h-z15.h }, z11.h[3]\n"
      "25:"  // Width 3: Multiply loop: multiply skip
      "tbz %x[flags], #1, 26f\n"
      ".inst 0xc0062c0c  // mova { z12.d-z15.d }, za.d[x9, #0]\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c20  // mova { z0.d-z3.d }, za.d[x9, #1]\n"
      "ld1rh { z17.h }, p1/Z, [x21]\n"
      ".inst 0xc0062c44  // mova { z4.d-z7.d }, za.d[x9, #2]\n"
      "ld1rh { z16.h }, p1/Z, [x20]\n"
      ".inst 0xc120e18c  // fcvt z12.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1cd  // fcvt z13.h, { z14.s-z15.s }\n"
      ".inst 0xc120e00e  // fcvt z14.h, { z0.s-z1.s }\n"
      ".inst 0xc120e04f  // fcvt z15.h, { z2.s-z3.s }\n"
      ".inst 0xc170c22c  // fclamp { z12.h-z13.h }, z17.h, z16.h\n"
      ".inst 0xc120e092  // fcvt z18.h, { z4.s-z5.s }\n"
      ".inst 0xc120e0d3  // fcvt z19.h, { z6.s-z7.s }\n"
      ".inst 0xc170c22e  // fclamp { z14.h-z15.h }, z17.h, z16.h\n"
      ".inst 0xc170c232  // fclamp { z18.h-z19.h }, z17.h, z16.h\n"
      ".inst 0xa060272c  // st1h { z12.h-z13.h }, pn9.b, [x25]\n"
      ".inst 0xa061272e  // st1h { z14.h-z15.h }, pn9.b, [x25, #0x2, MUL VL]\n"
      ".inst 0xa0622332  // st1h { z18.h-z19.h }, p8, [x25, #0x4, MUL VL]\n"
      "addvl x25, x25, #6\n"
      "b 27f\n"
      "26:"  // Width 3: No activation
      ".inst 0xc0062c18  // mova { z24.d-z27.d }, za.d[x9, #0]\n"
      ".inst 0xc0062c28  // mova { z8.d-z11.d }, za.d[x9, #1]\n"
      ".inst 0xc0062c4c  // mova { z12.d-z15.d }, za.d[x9, #2]\n"
      ".inst 0xc120e311  // fcvt z17.h, { z24.s-z25.s }\n"
      ".inst 0xc120e359  // fcvt z25.h, { z26.s-z27.s }\n"
      ".inst 0xa1602731  // st1h { z17.h, z25.h }, pn9.b, [x25]\n"
      ".inst 0xc120e112  // fcvt z18.h, { z8.s-z9.s }\n"
      ".inst 0xc120e153  // fcvt z19.h, { z10.s-z11.s }\n"
      ".inst 0xa0612732  // st1h { z18.h-z19.h }, pn9.b, [x25, #0x2, MUL VL]\n"
      ".inst 0xc120e191  // fcvt z17.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1d9  // fcvt z25.h, { z14.s-z15.s }\n"
      ".inst 0xa1622331  // st1h { z17.h, z25.h }, p8, [x25, #0x4, MUL VL]\n"
      "addvl x25, x25, #6\n"
      "27:"  // Width 3: Output done
      "b 36f\n"
      "28:"  // Width 4
      "mov x20, #0x3\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x1\n"
      "msub x20, x28, x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x257447f0  // whilelt p8.h, XZR, x20, VLx2\n"
      "cbz x24, 29f\n"
      "addvl x20, x24, #4\n"
      "ld1h { z28.s }, p1/Z, [x24]\n"
      "ld1h { z29.s }, p1/Z, [x24, #1, MUL VL]\n"
      "ld1h { z30.s }, p1/Z, [x24, #2, MUL VL]\n"
      "ld1h { z31.s }, p1/Z, [x24, #3, MUL VL]\n"
      "fcvt z28.s, p1/m, z28.h\n"
      "ld1h { z8.s }, p1/Z, [x24, #4, MUL VL]\n"
      "fcvt z29.s, p1/m, z29.h\n"
      "ld1h { z9.s }, p1/Z, [x24, #5, MUL VL]\n"
      "fcvt z30.s, p1/m, z30.h\n"
      "ld1h { z10.s }, p1/Z, [x24, #6, MUL VL]\n"
      "fcvt z31.s, p1/m, z31.h\n"
      "ld1h { z11.s }, p1/Z, [x24, #7, MUL VL]\n"
      "fcvt z8.s, p1/m, z8.h\n"
      "addvl x24, x24, #8\n"
      "ld1h { z0.s }, p1/Z, [x20]\n"
      "fcvt z9.s, p1/m, z9.h\n"
      "ld1h { z1.s }, p1/Z, [x20, #1, MUL VL]\n"
      "fcvt z10.s, p1/m, z10.h\n"
      "ld1h { z2.s }, p1/Z, [x20, #2, MUL VL]\n"
      "fcvt z11.s, p1/m, z11.h\n"
      ".inst 0xc0042f80  // mova za.d[x9, #0], { z28.d-z31.d }\n"
      "ld1h { z3.s }, p1/Z, [x20, #3, MUL VL]\n"
      "fcvt z0.s, p1/m, z0.h\n"
      "ld1h { z28.s }, p1/Z, [x20, #4, MUL VL]\n"
      "fcvt z1.s, p1/m, z1.h\n"
      "ld1h { z29.s }, p1/Z, [x20, #5, MUL VL]\n"
      "fcvt z2.s, p1/m, z2.h\n"
      "ld1h { z30.s }, p1/Z, [x20, #6, MUL VL]\n"
      "fcvt z3.s, p1/m, z3.h\n"
      ".inst 0xc0042d01  // mova za.d[x9, #1], { z8.d-z11.d }\n"
      "ld1h { z31.s }, p1/Z, [x20, #7, MUL VL]\n"
      "fcvt z28.s, p1/m, z28.h\n"
      "fcvt z29.s, p1/m, z29.h\n"
      "fcvt z30.s, p1/m, z30.h\n"
      "fcvt z31.s, p1/m, z31.h\n"
      ".inst 0xc0042c02  // mova za.d[x9, #2], { z0.d-z3.d }\n"
      ".inst 0xc0042f83  // mova za.d[x9, #3], { z28.d-z31.d }\n"
      "b 30f\n"
      "29:"  // Width 4: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "30:"  // Width 4: setup done
      "cmp x22, #0x8\n"
      "ble 32f\n"
      "31:"  // Width 4: Multiply loop: Main loop head
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a769  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x27]\n"
      "sub x22, x22, #0x8\n"
      "ld1rqh { z3.h }, p0/Z, [x23]\n"
      "cmp x22, #0x8\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc153b108  // fdot za.s[x9, 0], { z8.h-z11.h }, z3.h[0]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc153b389  // fdot za.s[x9, 1], { z28.h-z31.h }, z3.h[0]\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc153b30a  // fdot za.s[x9, 2], { z24.h-z27.h }, z3.h[0]\n"
      ".inst 0xa041a769  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc153b08b  // fdot za.s[x9, 3], { z4.h-z7.h }, z3.h[0]\n"
      ".inst 0xa042a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc153b588  // fdot za.s[x9, 0], { z12.h-z15.h }, z3.h[1]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc153b509  // fdot za.s[x9, 1], { z8.h-z11.h }, z3.h[1]\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc153b60a  // fdot za.s[x9, 2], { z16.h-z19.h }, z3.h[1]\n"
      ".inst 0xa041a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc153b70b  // fdot za.s[x9, 3], { z24.h-z27.h }, z3.h[1]\n"
      ".inst 0xa042a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a769  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc153b988  // fdot za.s[x9, 0], { z12.h-z15.h }, z3.h[2]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc153b889  // fdot za.s[x9, 1], { z4.h-z7.h }, z3.h[2]\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      ".inst 0xc153ba8a  // fdot za.s[x9, 2], { z20.h-z23.h }, z3.h[2]\n"
      ".inst 0xa041a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xc153b90b  // fdot za.s[x9, 3], { z8.h-z11.h }, z3.h[2]\n"
      ".inst 0xa042a769  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc153bd88  // fdot za.s[x9, 0], { z12.h-z15.h }, z3.h[3]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc153bc89  // fdot za.s[x9, 1], { z4.h-z7.h }, z3.h[3]\n"
      ".inst 0xc153bd0a  // fdot za.s[x9, 2], { z8.h-z11.h }, z3.h[3]\n"
      ".inst 0xc153be0b  // fdot za.s[x9, 3], { z16.h-z19.h }, z3.h[3]\n"
      "bgt 31b\n"
      "32:"  // Width 4: Multiply loop: Single iteration only
      "whilelt p0.h, XZR, x22\n"
      ".inst 0xa040a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      "ld1rqh { z11.h }, p0/Z, [x23]\n"
      "add x23, x23, #0x10\n"
      ".inst 0xa041a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc15bb208  // fdot za.s[x9, 0], { z16.h-z19.h }, z11.h[0]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb089  // fdot za.s[x9, 1], { z4.h-z7.h }, z11.h[0]\n"
      ".inst 0xc15bb18a  // fdot za.s[x9, 2], { z12.h-z15.h }, z11.h[0]\n"
      ".inst 0xc15bb38b  // fdot za.s[x9, 3], { z28.h-z31.h }, z11.h[0]\n"
      "ble 33f\n"
      ".inst 0xa040a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a779  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc15bb488  // fdot za.s[x9, 0], { z4.h-z7.h }, z11.h[1]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bb609  // fdot za.s[x9, 1], { z16.h-z19.h }, z11.h[1]\n"
      ".inst 0xc15bb58a  // fdot za.s[x9, 2], { z12.h-z15.h }, z11.h[1]\n"
      ".inst 0xc15bb70b  // fdot za.s[x9, 3], { z24.h-z27.h }, z11.h[1]\n"
      "ble 33f\n"
      ".inst 0xa040a76d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x27]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xa041a77d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc15bb988  // fdot za.s[x9, 0], { z12.h-z15.h }, z11.h[2]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbb89  // fdot za.s[x9, 1], { z28.h-z31.h }, z11.h[2]\n"
      ".inst 0xc15bb80a  // fdot za.s[x9, 2], { z0.h-z3.h }, z11.h[2]\n"
      ".inst 0xc15bb88b  // fdot za.s[x9, 3], { z4.h-z7.h }, z11.h[2]\n"
      "ble 33f\n"
      ".inst 0xa040a761  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x27]\n"
      ".inst 0xa041a771  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0xa042a765  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      ".inst 0xa043a775  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      ".inst 0xc15bbc08  // fdot za.s[x9, 0], { z0.h-z3.h }, z11.h[3]\n"
      "addvl x27, x27, #16\n"
      ".inst 0xc15bbe09  // fdot za.s[x9, 1], { z16.h-z19.h }, z11.h[3]\n"
      ".inst 0xc15bbc8a  // fdot za.s[x9, 2], { z4.h-z7.h }, z11.h[3]\n"
      ".inst 0xc15bbe8b  // fdot za.s[x9, 3], { z20.h-z23.h }, z11.h[3]\n"
      "33:"  // Width 4: Multiply loop: multiply skip
      "tbz %x[flags], #1, 34f\n"
      ".inst 0xc0062c1c  // mova { z28.d-z31.d }, za.d[x9, #0]\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c2c  // mova { z12.d-z15.d }, za.d[x9, #1]\n"
      "ld1rh { z19.h }, p1/Z, [x21]\n"
      ".inst 0xc0062c40  // mova { z0.d-z3.d }, za.d[x9, #2]\n"
      "ld1rh { z18.h }, p1/Z, [x20]\n"
      ".inst 0xc0062c64  // mova { z4.d-z7.d }, za.d[x9, #3]\n"
      ".inst 0xc120e38a  // fcvt z10.h, { z28.s-z29.s }\n"
      ".inst 0xc120e3cb  // fcvt z11.h, { z30.s-z31.s }\n"
      ".inst 0xc120e18c  // fcvt z12.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1cd  // fcvt z13.h, { z14.s-z15.s }\n"
      ".inst 0xc172c26a  // fclamp { z10.h-z11.h }, z19.h, z18.h\n"
      ".inst 0xc120e00e  // fcvt z14.h, { z0.s-z1.s }\n"
      ".inst 0xc120e04f  // fcvt z15.h, { z2.s-z3.s }\n"
      ".inst 0xc172c26c  // fclamp { z12.h-z13.h }, z19.h, z18.h\n"
      ".inst 0xc120e090  // fcvt z16.h, { z4.s-z5.s }\n"
      ".inst 0xc120e0d1  // fcvt z17.h, { z6.s-z7.s }\n"
      ".inst 0xc172c26e  // fclamp { z14.h-z15.h }, z19.h, z18.h\n"
      ".inst 0xc172c270  // fclamp { z16.h-z17.h }, z19.h, z18.h\n"
      ".inst 0xa060272a  // st1h { z10.h-z11.h }, pn9.b, [x25]\n"
      ".inst 0xa061272c  // st1h { z12.h-z13.h }, pn9.b, [x25, #0x2, MUL VL]\n"
      ".inst 0xa062272e  // st1h { z14.h-z15.h }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xa0632330  // st1h { z16.h-z17.h }, p8, [x25, #0x6, MUL VL]\n"
      "addvl x25, x25, #8\n"
      "b 35f\n"
      "34:"  // Width 4: No activation
      ".inst 0xc0062c0c  // mova { z12.d-z15.d }, za.d[x9, #0]\n"
      ".inst 0xc0062c30  // mova { z16.d-z19.d }, za.d[x9, #1]\n"
      ".inst 0xc0062c5c  // mova { z28.d-z31.d }, za.d[x9, #2]\n"
      ".inst 0xc0062c68  // mova { z8.d-z11.d }, za.d[x9, #3]\n"
      ".inst 0xc120e187  // fcvt z7.h, { z12.s-z13.s }\n"
      ".inst 0xc120e1cf  // fcvt z15.h, { z14.s-z15.s }\n"
      ".inst 0xa1602727  // st1h { z7.h, z15.h }, pn9.b, [x25]\n"
      ".inst 0xc120e207  // fcvt z7.h, { z16.s-z17.s }\n"
      ".inst 0xc120e24f  // fcvt z15.h, { z18.s-z19.s }\n"
      ".inst 0xa1612727  // st1h { z7.h, z15.h }, pn9.b, [x25, #0x2, MUL VL]\n"
      ".inst 0xc120e38e  // fcvt z14.h, { z28.s-z29.s }\n"
      ".inst 0xc120e3cf  // fcvt z15.h, { z30.s-z31.s }\n"
      ".inst 0xa062272e  // st1h { z14.h-z15.h }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xc120e112  // fcvt z18.h, { z8.s-z9.s }\n"
      ".inst 0xc120e15a  // fcvt z26.h, { z10.s-z11.s }\n"
      ".inst 0xa1632332  // st1h { z18.h, z26.h }, p8, [x25, #0x6, MUL VL]\n"
      "addvl x25, x25, #8\n"
      "35:"  // Width 4: Output done
      "subs x26, x26, #0x4\n"
      "sub %x[N], %x[N], x28, LSL #2\n"
      "bgt 4b\n"
      "36:"  // Exit
      ".inst 0xd503467f  // SMSTOP\n"
      "ptrue p8.b\n"
      : [N] "+&r" (N)
      : [A_ptr] "r" (A_ptr), [B_ptr] "r" (B_ptr), [K] "r" (K), [args_ptr] "r" (&ka), [bias] "r" (bias), [flags] "r" (flags), [offset_max] "I" (offsetof(KernelArgs, maxval)), [offset_min] "I" (offsetof(KernelArgs, minval)), [output_ptr] "r" (output_ptr)
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x9", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

} // namespace arm_gemm

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
