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
#ifdef ARM_COMPUTE_ENABLE_SME2

#include "arm_gemm.hpp"


#include "../../asmlib.hpp"
#include "../../utils.hpp"

namespace arm_gemm {

void sme2_interleaved_nomerge_fp16fp32fp16_mopa_4VLx1VL(const __fp16 *const A, const __fp16 *const B, __fp16 *const C, int ldc, const int M, const int N, const int K, const __fp16 *const bias, const Activation act, bool accumulate, float *const accumulator_buffer)
{
  struct KernelArgs
  {
    KernelArgs(
      const __fp16 *const A,
      const __fp16 *const B,
      __fp16 *const C, const int ldc,
      const int M, const int N, const int K,
      const __fp16 *const bias,
      const Activation act,
      bool accumulate,
      float *const accumulator_buffer
    ) : A(A),
        B(B), kstride_bytes(roundup(K, 2) * sizeof(__fp16)),
        C(C), ldcb(ldc * sizeof(__fp16)),
        M(M), N(N), K(K),
        min(-static_cast<__fp16>(std::numeric_limits<float>::infinity())),
        max(static_cast<__fp16>(std::numeric_limits<float>::infinity())),
        bias(bias),
        accumulator_buffer(accumulator_buffer),
        flags(0x0)
    {
      if (accumulate)
      {
        flags |= 1 << 0;  // FILL_ACCUMULATORS_FROM_BUFFER
      }
      if (C == nullptr)
      {
        flags |= 1 << 1;  // STORE_ACCUMULATORS_TO_BUFFER
      }

      // Initialise the activation values
      switch (act.type)
      {
        default:
        case Activation::Type::None:
            break;
        case Activation::Type::BoundedReLU:
            this->max = static_cast<__fp16>(act.param1);
            /* fall through */
        case Activation::Type::ReLU:
            this->min = static_cast<__fp16>(0);
            break;
      }
    }

    const __fp16 *const A;
    const __fp16 *const B;
    const long kstride_bytes;
    __fp16 *const C;
    const long ldcb;
    const long M, N, K;
    __fp16 min = -static_cast<__fp16>(std::numeric_limits<float>::infinity());
    __fp16 max = static_cast<__fp16>(std::numeric_limits<float>::infinity());

    const __fp16 *const bias;


    float *const accumulator_buffer;
    uint64_t flags;
  };

  // Construct arguments for this kernel
  KernelArgs args(A, B, C, ldc, M, N, K, bias, act, accumulate, accumulator_buffer);

  __asm__ __volatile__(
      "ldr x16, [%x[args], %[offsetof_flags]]\n"
      ".inst 0xd503477f  // SMSTART ZA\n"
      "ptrue p1.b\n"
      ".inst 0x25207811  // ptrue pn9.b\n"
      "ldr x15, [%x[args], %[offsetof_accumulator_buffer]]\n"
      "ldr x14, [%x[args], %[offsetof_accumulator_buffer]]\n"
      "tbz x16, #0, 2f\n"
      "mov x12, #0x0\n"
      "cntw x20\n"
      "1:"  // Initial accumulator load from buffer: Loop
      ".inst 0xa040c5e8  // ld1w { z8.s-z11.s }, pn9.b/Z, [x15]\n"
      ".inst 0xa041c5f4  // ld1w { z20.s-z23.s }, pn9.b/Z, [x15, #0x4, MUL VL]\n"
      ".inst 0xa042c5e4  // ld1w { z4.s-z7.s }, pn9.b/Z, [x15, #0x8, MUL VL]\n"
      ".inst 0xa043c5ec  // ld1w { z12.s-z15.s }, pn9.b/Z, [x15, #0xc, MUL VL]\n"
      ".inst 0xc0840500  // mova za0h.s[x12], { z8.s-z11.s }\n"
      "addvl x15, x15, #16\n"
      ".inst 0xc0840681  // mova za1h.s[x12], { z20.s-z23.s }\n"
      ".inst 0xc0840482  // mova za2h.s[x12], { z4.s-z7.s }\n"
      ".inst 0xc0840583  // mova za3h.s[x12], { z12.s-z15.s }\n"
      "add x12, x12, #0x4\n"
      "cmp x12, x20\n"
      "blt 1b\n"
      "2:"  // Initial accumulator load from buffer: End
      "ldr w13, [%x[args], %[offsetof_M]]\n"
      "mov x11, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w9, [%x[args], %[offsetof_N]]\n"
      "ldr x28, [%x[args], %[offsetof_A]]\n"
      "3:"  // M and N loop
      "mov x27, x28\n"
      "whilelt p8.s, x10, x9\n"
      "tbnz x16, #0, 4f\n"
      "ldr x20, [%x[args], %[offsetof_bias]]\n"
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "cbz x20, 5f\n"
      "whilelt p0.h, x10, x9\n"
      "fmov z13.h, #0.0\n"
      "fmov z27.h, #1.0\n"
      "ld1h { z14.h }, p0/Z, [x20, x10, LSL #1]\n"
      "zip1 z30.h, z14.h, z13.h\n"
      ".inst 0x81be2760  // fmopa za0.s, p1/M, p1/M, z27.h, z30.h\n"
      ".inst 0x81be2761  // fmopa za1.s, p1/M, p1/M, z27.h, z30.h\n"
      ".inst 0x81be2762  // fmopa za2.s, p1/M, p1/M, z27.h, z30.h\n"
      ".inst 0x81be2763  // fmopa za3.s, p1/M, p1/M, z27.h, z30.h\n"
      "4:"  // Prepare accumulators: Test for last block
      "mov x20, x10\n"
      "mov x21, x11\n"
      "incw x20\n"
      "incw x21, ALL, MUL #4\n"
      "cmp x20, x9\n"
      "mov x20, x16\n"
      "csel x21, x11, x21, LT\n"
      "bfm x16, XZR, #0x0, #0x0  // bfc x16, #0x0, #0x1\n"
      "cmp x21, x13\n"
      "csel x16, x20, x16, LT\n"
      "5:"  // Prepare accumulators: End
      "ldr x20, [%x[args], %[offsetof_K]]\n"
      "ldr x23, [%x[args], %[offsetof_B]]\n"
      "ldr x22, [%x[args], %[offsetof_kstride_bytes]]\n"
      "add x20, x20, #0x1\n"
      "lsr x20, x20, #0x1\n"
      "lsr x21, x20, #0x2\n"
      "madd x23, x10, x22, x23\n"  // bptr = B + n * kstride_bytes
      "and x20, x20, #0x3\n"
      "cbz x21, 8f\n"
      "subs x21, x21, #0x1\n"
      ".inst 0xa040a778  // ld1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      "ld1h { z1.h }, p1/Z, [x23]\n"
      ".inst 0xa041a764  // ld1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      "ld1h { z23.h }, p1/Z, [x23, #1, MUL VL]\n"
      ".inst 0xa042a77c  // ld1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "ld1h { z19.h }, p1/Z, [x23, #2, MUL VL]\n"
      ".inst 0xa043a76c  // ld1h { z12.h-z15.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      "addvl x27, x27, #16\n"
      "ld1h { z0.h }, p1/Z, [x23, #3, MUL VL]\n"
      "addvl x23, x23, #4\n"
      "ble 7f\n"
      "6:"  // K loop
      ".inst 0x81a12700  // fmopa za0.s, p1/M, p1/M, z24.h, z1.h\n"
      "subs x21, x21, #0x1\n"
      ".inst 0x81a12721  // fmopa za1.s, p1/M, p1/M, z25.h, z1.h\n"
      ".inst 0x81a12742  // fmopa za2.s, p1/M, p1/M, z26.h, z1.h\n"
      ".inst 0x81a12763  // fmopa za3.s, p1/M, p1/M, z27.h, z1.h\n"
      ".inst 0xa040a778  // ld1h { z24.h-z27.h }, pn9.b/Z, [x27]\n"
      ".inst 0x81b72480  // fmopa za0.s, p1/M, p1/M, z4.h, z23.h\n"
      "ld1h { z1.h }, p1/Z, [x23]\n"
      ".inst 0x81b724a1  // fmopa za1.s, p1/M, p1/M, z5.h, z23.h\n"
      ".inst 0x81b724c2  // fmopa za2.s, p1/M, p1/M, z6.h, z23.h\n"
      ".inst 0x81b724e3  // fmopa za3.s, p1/M, p1/M, z7.h, z23.h\n"
      ".inst 0xa041a764  // ld1h { z4.h-z7.h }, pn9.b/Z, [x27, #0x4, MUL VL]\n"
      ".inst 0x81b32780  // fmopa za0.s, p1/M, p1/M, z28.h, z19.h\n"
      "ld1h { z23.h }, p1/Z, [x23, #1, MUL VL]\n"
      ".inst 0x81b327a1  // fmopa za1.s, p1/M, p1/M, z29.h, z19.h\n"
      ".inst 0x81b327c2  // fmopa za2.s, p1/M, p1/M, z30.h, z19.h\n"
      ".inst 0x81b327e3  // fmopa za3.s, p1/M, p1/M, z31.h, z19.h\n"
      ".inst 0xa042a77c  // ld1h { z28.h-z31.h }, pn9.b/Z, [x27, #0x8, MUL VL]\n"
      "ld1h { z19.h }, p1/Z, [x23, #2, MUL VL]\n"
      ".inst 0x81a02580  // fmopa za0.s, p1/M, p1/M, z12.h, z0.h\n"
      ".inst 0x81a025a1  // fmopa za1.s, p1/M, p1/M, z13.h, z0.h\n"
      ".inst 0x81a025c2  // fmopa za2.s, p1/M, p1/M, z14.h, z0.h\n"
      ".inst 0x81a025e3  // fmopa za3.s, p1/M, p1/M, z15.h, z0.h\n"
      ".inst 0xa043a76c  // ld1h { z12.h-z15.h }, pn9.b/Z, [x27, #0xc, MUL VL]\n"
      "addvl x27, x27, #16\n"
      "ld1h { z0.h }, p1/Z, [x23, #3, MUL VL]\n"
      "addvl x23, x23, #4\n"
      "bgt 6b\n"
      "7:"  // K loop tail
      ".inst 0x81a12700  // fmopa za0.s, p1/M, p1/M, z24.h, z1.h\n"
      ".inst 0x81a12721  // fmopa za1.s, p1/M, p1/M, z25.h, z1.h\n"
      ".inst 0x81a12742  // fmopa za2.s, p1/M, p1/M, z26.h, z1.h\n"
      ".inst 0x81a12763  // fmopa za3.s, p1/M, p1/M, z27.h, z1.h\n"
      ".inst 0x81b72480  // fmopa za0.s, p1/M, p1/M, z4.h, z23.h\n"
      ".inst 0x81b724a1  // fmopa za1.s, p1/M, p1/M, z5.h, z23.h\n"
      ".inst 0x81b724c2  // fmopa za2.s, p1/M, p1/M, z6.h, z23.h\n"
      ".inst 0x81b724e3  // fmopa za3.s, p1/M, p1/M, z7.h, z23.h\n"
      ".inst 0x81b32780  // fmopa za0.s, p1/M, p1/M, z28.h, z19.h\n"
      ".inst 0x81b327a1  // fmopa za1.s, p1/M, p1/M, z29.h, z19.h\n"
      ".inst 0x81b327c2  // fmopa za2.s, p1/M, p1/M, z30.h, z19.h\n"
      ".inst 0x81b327e3  // fmopa za3.s, p1/M, p1/M, z31.h, z19.h\n"
      ".inst 0x81a02580  // fmopa za0.s, p1/M, p1/M, z12.h, z0.h\n"
      ".inst 0x81a025a1  // fmopa za1.s, p1/M, p1/M, z13.h, z0.h\n"
      ".inst 0x81a025c2  // fmopa za2.s, p1/M, p1/M, z14.h, z0.h\n"
      ".inst 0x81a025e3  // fmopa za3.s, p1/M, p1/M, z15.h, z0.h\n"
      "8:"  // K oddments
      "cbz x20, 10f\n"
      "9:"  // K oddments: Loop
      ".inst 0xa040a768  // ld1h { z8.h-z11.h }, pn9.b/Z, [x27]\n"
      "subs x20, x20, #0x1\n"
      "addvl x27, x27, #4\n"
      "ld1h { z12.h }, p1/Z, [x23]\n"
      "addvl x23, x23, #1\n"
      ".inst 0x81ac2500  // fmopa za0.s, p1/M, p1/M, z8.h, z12.h\n"
      ".inst 0x81ac2521  // fmopa za1.s, p1/M, p1/M, z9.h, z12.h\n"
      ".inst 0x81ac2542  // fmopa za2.s, p1/M, p1/M, z10.h, z12.h\n"
      ".inst 0x81ac2563  // fmopa za3.s, p1/M, p1/M, z11.h, z12.h\n"
      "bgt 9b\n"
      "10:"  // K oddments: End
      "tbz x16, #1, 14f\n"
      "tbz x16, #0, 12f\n"
      "mov x12, #0x0\n"
      "cntw x20\n"
      "11:"  // Store to partial result buffer: Store and refill: Loop
      ".inst 0xa040c5e4  // ld1w { z4.s-z7.s }, pn9.b/Z, [x15]\n"
      ".inst 0xc086041c  // mova { z28.s-z31.s }, za0h.s[x12]\n"
      ".inst 0xc0860434  // mova { z20.s-z23.s }, za1h.s[x12]\n"
      ".inst 0xa041c5f8  // ld1w { z24.s-z27.s }, pn9.b/Z, [x15, #0x4, MUL VL]\n"
      ".inst 0xc0860448  // mova { z8.s-z11.s }, za2h.s[x12]\n"
      ".inst 0xc0860460  // mova { z0.s-z3.s }, za3h.s[x12]\n"
      ".inst 0xa042c5ec  // ld1w { z12.s-z15.s }, pn9.b/Z, [x15, #0x8, MUL VL]\n"
      ".inst 0xa043c5f0  // ld1w { z16.s-z19.s }, pn9.b/Z, [x15, #0xc, MUL VL]\n"
      ".inst 0xc0840480  // mova za0h.s[x12], { z4.s-z7.s }\n"
      "addvl x15, x15, #16\n"
      ".inst 0xc0840701  // mova za1h.s[x12], { z24.s-z27.s }\n"
      ".inst 0xa060c5dc  // st1w { z28.s-z31.s }, pn9.b, [x14]\n"
      ".inst 0xc0840582  // mova za2h.s[x12], { z12.s-z15.s }\n"
      ".inst 0xa061c5d4  // st1w { z20.s-z23.s }, pn9.b, [x14, #0x4, MUL VL]\n"
      ".inst 0xc0840603  // mova za3h.s[x12], { z16.s-z19.s }\n"
      "add x12, x12, #0x4\n"
      ".inst 0xa062c5c8  // st1w { z8.s-z11.s }, pn9.b, [x14, #0x8, MUL VL]\n"
      "cmp x12, x20\n"
      ".inst 0xa063c5c0  // st1w { z0.s-z3.s }, pn9.b, [x14, #0xc, MUL VL]\n"
      "addvl x14, x14, #16\n"
      "blt 11b\n"
      "b 29f\n"
      "12:"  // Store to partial result buffer: Store only
      "mov x12, #0x0\n"
      "cntw x20\n"
      "13:"  // Store to partial result buffer: Store only: Loop
      ".inst 0xc0860410  // mova { z16.s-z19.s }, za0h.s[x12]\n"
      ".inst 0xc0860420  // mova { z0.s-z3.s }, za1h.s[x12]\n"
      ".inst 0xc0860458  // mova { z24.s-z27.s }, za2h.s[x12]\n"
      ".inst 0xc086046c  // mova { z12.s-z15.s }, za3h.s[x12]\n"
      ".inst 0xa060c5d0  // st1w { z16.s-z19.s }, pn9.b, [x14]\n"
      "add x12, x12, #0x4\n"
      ".inst 0xa061c5c0  // st1w { z0.s-z3.s }, pn9.b, [x14, #0x4, MUL VL]\n"
      "cmp x12, x20\n"
      ".inst 0xa062c5d8  // st1w { z24.s-z27.s }, pn9.b, [x14, #0x8, MUL VL]\n"
      ".inst 0xa063c5cc  // st1w { z12.s-z15.s }, pn9.b, [x14, #0xc, MUL VL]\n"
      "addvl x14, x14, #16\n"
      "blt 13b\n"
      "b 29f\n"
      "14:"  // Store to output array
      "ldr x26, [%x[args], %[offsetof_C]]\n"
      "sub x25, x13, x11\n"
      "cntw x24\n"
      "ld1rh { z21.h }, p1/Z, [%x[args], %[offsetof_KernelArgs_min]]\n"
      "ldr x23, [%x[args], %[offsetof_ldcb]]\n"
      "whilelt p0.s, x10, x9\n"
      "cmp x25, x24\n"
      "ld1rh { z20.h }, p1/Z, [%x[args], %[offsetof_KernelArgs_max]]\n"
      "csel x22, x25, x24, LT\n"
      "mov x12, #0x0\n"
      "add x26, x26, x10, LSL #1\n"  // C += n
      "lsr x21, x22, #0x2\n"
      "madd x26, x11, x23, x26\n"  // C += m * ldc
      "and x20, x22, #0x3\n"
      "cbz x21, 16f\n"
      "15:"  // Store to output array: Accumulator row 0 loop
      ".inst 0xc086041c  // mova { z28.s-z31.s }, za0h.s[x12]\n"
      "add x12, x12, #0x4\n"
      "fcvt z28.h, p1/m, z28.s\n"
      "fcvt z29.h, p1/m, z29.s\n"
      "cmp x12, x21, LSL #2\n"
      "fcvt z30.h, p1/m, z30.s\n"
      "fcvt z31.h, p1/m, z31.s\n"
      ".inst 0xc174cabc  // fclamp { z28.h-z31.h }, z21.h, z20.h\n"
      "st1h { z28.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z29.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z30.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z31.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "blt 15b\n"
      "16:"  // Store to output array: Accumulator row 0 oddments
      "cbz x20, 17f\n"
      ".inst 0xc0860410  // mova { z16.s-z19.s }, za0h.s[x12]\n"
      "subs x20, x20, #0x1\n"
      "fcvt z16.h, p1/m, z16.s\n"
      "fcvt z17.h, p1/m, z17.s\n"
      "fcvt z18.h, p1/m, z18.s\n"
      "fcvt z19.h, p1/m, z19.s\n"
      ".inst 0xc174cab0  // fclamp { z16.h-z19.h }, z21.h, z20.h\n"
      "st1h { z16.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 17f\n"
      "subs x20, x20, #0x1\n"
      "st1h { z17.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 17f\n"
      "st1h { z18.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "17:"  // Store to output array: Accumulator row 0 oddments: End
      "subs x25, x25, x22\n"
      "beq 27f\n"
      "cmp x25, x24\n"
      "mov x12, #0x0\n"
      "csel x22, x25, x24, LT\n"
      "lsr x21, x22, #0x2\n"
      "and x20, x22, #0x3\n"
      "cbz x21, 19f\n"
      "18:"  // Store to output array: Accumulator row 1 loop
      ".inst 0xc0860420  // mova { z0.s-z3.s }, za1h.s[x12]\n"
      "add x12, x12, #0x4\n"
      "fcvt z0.h, p1/m, z0.s\n"
      "fcvt z1.h, p1/m, z1.s\n"
      "cmp x12, x21, LSL #2\n"
      "fcvt z2.h, p1/m, z2.s\n"
      "fcvt z3.h, p1/m, z3.s\n"
      ".inst 0xc174caa0  // fclamp { z0.h-z3.h }, z21.h, z20.h\n"
      "st1h { z0.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z1.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z2.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z3.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "blt 18b\n"
      "19:"  // Store to output array: Accumulator row 1 oddments
      "cbz x20, 20f\n"
      ".inst 0xc0860438  // mova { z24.s-z27.s }, za1h.s[x12]\n"
      "subs x20, x20, #0x1\n"
      "fcvt z24.h, p1/m, z24.s\n"
      "fcvt z25.h, p1/m, z25.s\n"
      "fcvt z26.h, p1/m, z26.s\n"
      "fcvt z27.h, p1/m, z27.s\n"
      ".inst 0xc174cab8  // fclamp { z24.h-z27.h }, z21.h, z20.h\n"
      "st1h { z24.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 20f\n"
      "subs x20, x20, #0x1\n"
      "st1h { z25.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 20f\n"
      "st1h { z26.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "20:"  // Store to output array: Accumulator row 1 oddments: End
      "subs x25, x25, x22\n"
      "beq 27f\n"
      "cmp x25, x24\n"
      "mov x12, #0x0\n"
      "csel x22, x25, x24, LT\n"
      "lsr x21, x22, #0x2\n"
      "and x20, x22, #0x3\n"
      "cbz x21, 22f\n"
      "21:"  // Store to output array: Accumulator row 2 loop
      ".inst 0xc0860450  // mova { z16.s-z19.s }, za2h.s[x12]\n"
      "add x12, x12, #0x4\n"
      "fcvt z16.h, p1/m, z16.s\n"
      "fcvt z17.h, p1/m, z17.s\n"
      "cmp x12, x21, LSL #2\n"
      "fcvt z18.h, p1/m, z18.s\n"
      "fcvt z19.h, p1/m, z19.s\n"
      ".inst 0xc174cab0  // fclamp { z16.h-z19.h }, z21.h, z20.h\n"
      "st1h { z16.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z17.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z18.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z19.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "blt 21b\n"
      "22:"  // Store to output array: Accumulator row 2 oddments
      "cbz x20, 23f\n"
      ".inst 0xc086045c  // mova { z28.s-z31.s }, za2h.s[x12]\n"
      "subs x20, x20, #0x1\n"
      "fcvt z28.h, p1/m, z28.s\n"
      "fcvt z29.h, p1/m, z29.s\n"
      "fcvt z30.h, p1/m, z30.s\n"
      "fcvt z31.h, p1/m, z31.s\n"
      ".inst 0xc174cabc  // fclamp { z28.h-z31.h }, z21.h, z20.h\n"
      "st1h { z28.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 23f\n"
      "subs x20, x20, #0x1\n"
      "st1h { z29.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 23f\n"
      "st1h { z30.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "23:"  // Store to output array: Accumulator row 2 oddments: End
      "subs x25, x25, x22\n"
      "beq 27f\n"
      "cmp x25, x24\n"
      "mov x12, #0x0\n"
      "csel x20, x25, x24, LT\n"
      "lsr x21, x20, #0x2\n"
      "and x20, x20, #0x3\n"
      "cbz x21, 25f\n"
      "24:"  // Store to output array: Accumulator row 3 loop
      ".inst 0xc086047c  // mova { z28.s-z31.s }, za3h.s[x12]\n"
      "add x12, x12, #0x4\n"
      "fcvt z28.h, p1/m, z28.s\n"
      "fcvt z29.h, p1/m, z29.s\n"
      "cmp x12, x21, LSL #2\n"
      "fcvt z30.h, p1/m, z30.s\n"
      "fcvt z31.h, p1/m, z31.s\n"
      ".inst 0xc174cabc  // fclamp { z28.h-z31.h }, z21.h, z20.h\n"
      "st1h { z28.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z29.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z30.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "st1h { z31.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "blt 24b\n"
      "25:"  // Store to output array: Accumulator row 3 oddments
      "cbz x20, 26f\n"
      ".inst 0xc086047c  // mova { z28.s-z31.s }, za3h.s[x12]\n"
      "subs x20, x20, #0x1\n"
      "fcvt z28.h, p1/m, z28.s\n"
      "fcvt z29.h, p1/m, z29.s\n"
      "fcvt z30.h, p1/m, z30.s\n"
      "fcvt z31.h, p1/m, z31.s\n"
      ".inst 0xc174cabc  // fclamp { z28.h-z31.h }, z21.h, z20.h\n"
      "st1h { z28.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 26f\n"
      "subs x20, x20, #0x1\n"
      "st1h { z29.s }, p0, [x26]\n"
      "add x26, x26, x23\n"
      "beq 26f\n"
      "st1h { z30.s }, p0, [x26]\n"
      "26:"  // Store to output array: Accumulator row 3 oddments: End
      "27:"  // Store to output array: End
      "tbz x16, #0, 29f\n"
      "mov x12, #0x0\n"
      "cntw x20\n"
      "28:"  // Store to output array: Refill accumulators: Loop
      ".inst 0xa040c5f8  // ld1w { z24.s-z27.s }, pn9.b/Z, [x15]\n"
      ".inst 0xa041c5ec  // ld1w { z12.s-z15.s }, pn9.b/Z, [x15, #0x4, MUL VL]\n"
      ".inst 0xa042c5e0  // ld1w { z0.s-z3.s }, pn9.b/Z, [x15, #0x8, MUL VL]\n"
      ".inst 0xa043c5e8  // ld1w { z8.s-z11.s }, pn9.b/Z, [x15, #0xc, MUL VL]\n"
      ".inst 0xc0840700  // mova za0h.s[x12], { z24.s-z27.s }\n"
      "addvl x15, x15, #16\n"
      ".inst 0xc0840581  // mova za1h.s[x12], { z12.s-z15.s }\n"
      ".inst 0xc0840402  // mova za2h.s[x12], { z0.s-z3.s }\n"
      ".inst 0xc0840503  // mova za3h.s[x12], { z8.s-z11.s }\n"
      "add x12, x12, #0x4\n"
      "cmp x12, x20\n"
      "blt 28b\n"
      "29:"  // End block
      "incw x10\n"
      "cmp x10, x9\n"
      "blt 3b\n"
      "incw x11, ALL, MUL #4\n"
      "mov x10, #0x0\n"
      "cmp x11, x13\n"
      "mov x28, x27\n"
      "blt 3b\n"
      ".inst 0xd503467f  // SMSTOP\n"
      :
      : [args] "r" (&args), [offsetof_A] "I" (offsetof(KernelArgs, A)), [offsetof_B] "I" (offsetof(KernelArgs, B)), [offsetof_C] "I" (offsetof(KernelArgs, C)), [offsetof_K] "I" (offsetof(KernelArgs, K)), [offsetof_KernelArgs_max] "I" (offsetof(KernelArgs, max)), [offsetof_KernelArgs_min] "I" (offsetof(KernelArgs, min)), [offsetof_M] "I" (offsetof(KernelArgs, M)), [offsetof_N] "I" (offsetof(KernelArgs, N)), [offsetof_accumulator_buffer] "I" (offsetof(KernelArgs, accumulator_buffer)), [offsetof_bias] "I" (offsetof(KernelArgs, bias)), [offsetof_flags] "I" (offsetof(KernelArgs, flags)), [offsetof_kstride_bytes] "I" (offsetof(KernelArgs, kstride_bytes)), [offsetof_ldcb] "I" (offsetof(KernelArgs, ldcb))
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

}  // namespace arm_gemm

#endif  // ARM_COMPUTE_ENABLE_SME2
