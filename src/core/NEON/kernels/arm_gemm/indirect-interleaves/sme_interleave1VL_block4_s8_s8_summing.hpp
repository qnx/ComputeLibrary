/*
 * Copyright (c) 2022-2024 Arm Limited.
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

#if defined(ARM_COMPUTE_ENABLE_SME)

template <>
void interleave_block<1, 4, VLType::SME, true>(
  int8_t * &out, const int8_t * const *in,
  size_t width, size_t height, size_t row_offset, bool first
)
{
  __asm__ __volatile__(
      ".inst 0xd503477f  // SMSTART ZA\n"
      "mov x23, %x[width]\n"
      "mov x21, %x[width]\n"
      "cntb x20\n"
      "incb x23\n"
      "mov z18.b, #0x1\n"
      "mov z17.s, #0x0\n"
      "sub x10, x20, #0x1\n"
      "cntw x9\n"
      "sub x23, x23, #0x1\n"
      "ands x10, x21, x10\n"
      "udiv x23, x23, x20\n"  // n_passes = ceildiv(width, VL<T>)
      "csel x10, x10, x20, NE\n"
      "lsl x22, %x[height], #0x1\n"  // height * 2
      "lsl x21, x9, #0x1\n"
      "sub x20, x23, #0x1\n"
      "add x10, x10, #0x3\n"
      "whilelt p9.b, XZR, x22\n"
      "whilelt p8.b, x21, x22\n"
      "mov x28, #0x0\n"
      "ptrue p4.b\n"
      "lsr x20, x20, #0x1\n"  // n_loops = (n_passes - 1) / 2
      "and x27, x23, #0x1\n"  // odd_tail = bool(n_passes & 0x1)
      "lsr x10, x10, #0x2\n"
      "sub x26, x9, #0x2\n"
      "ptrue p11.s\n"
      "zip1 p10.b, p9.b, p8.b\n"
      "mov x25, %x[row_offset]\n"
      "mov x24, %x[out]\n"
      "whilelt p9.b, x28, %x[width]\n"
      "whilelt p8.b, x28, %x[width]\n"
      "cbnz %x[first], 1f\n"
      "addvl x24, x24, #-1\n"
      "ld1w { z17.s }, p4/Z, [x24]\n"
      "1:"  // K loop: Load row sums: End
      "mov x23, %x[in]\n"
      "mov x12, #0x0\n"
      "ldr x22, [x23, #0x0]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "cbz x26, 3f\n"
      "2:"  // K loop: Charge: Loop
      ".inst 0x25246141  // psel p1.b, p8.b/Z, p10.b[w12]\n"
      ".inst 0x25646140  // psel p0.b, p8.b/Z, p10.b[w12, #4]\n"
      ".inst 0xe01906c0  // ld1b { za0h.b[x12] }, p1/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      ".inst 0xe01902a4  // ld1b { za0h.b[x12, #4] }, p0/Z, [x21, x25]\n"
      "add x12, x12, #0x8\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "cmp x12, x26, LSL #2\n"
      "blt 2b\n"
      "3:"  // K loop: Charge: End
      ".inst 0x25246141  // psel p1.b, p8.b/Z, p10.b[w12]\n"
      ".inst 0x25646140  // psel p0.b, p8.b/Z, p10.b[w12, #4]\n"
      "mov x23, %x[in]\n"
      "incb x28\n"
      ".inst 0xe01906c0  // ld1b { za0h.b[x12] }, p1/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      ".inst 0xe01902a4  // ld1b { za0h.b[x12, #4] }, p0/Z, [x21, x25]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "incb x25\n"
      "cbz x20, 9f\n"
      "mov x20, x20\n"
      "4:"  // K loop: Main loop
      "whilelt p8.b, x28, %x[width]\n"
      "mov x12, #0x0\n"
      "mov x14, #0x0\n"
      "cbz x26, 6f\n"
      "5:"  // K loop: Main loop: First: Loop
      ".inst 0x25346143  // psel p3.b, p8.b/Z, p10.b[w12, #2]\n"
      ".inst 0x25746142  // psel p2.b, p8.b/Z, p10.b[w12, #6]\n"
      ".inst 0x25266d21  // psel p1.b, p11.b/Z, p9.b[w14]\n"
      ".inst 0x252e6d20  // psel p0.b, p11.b/Z, p9.b[w14, #1]\n"
      ".inst 0xe0190ec2  // ld1b { za0h.b[x12, #2] }, p3/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      ".inst 0xe0190aa6  // ld1b { za0h.b[x12, #6] }, p2/Z, [x21, x25]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "add x12, x12, #0x8\n"
      ".inst 0xc082d010  // mova z16.s, p4/M, za0v.s[x14]\n"
      ".inst 0xe0bfc700  // st1w { za0v.s[x14] }, p1/Z, [x24, XZR, LSL #2]\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xe0a9c301  // st1w { za0v.s[x14, #1] }, p0/Z, [x24, x9, LSL #2]\n"
      ".inst 0xc082d030  // mova z16.s, p4/M, za0v.s[x14, #1]\n"
      "add x14, x14, #0x2\n"
      "cmp x14, x26\n"
      "addvl x24, x24, #2\n"
      "sdot z17.s, z16.b, z18.b\n"
      "blt 5b\n"
      "6:"  // K loop: Main loop: First: Tail
      ".inst 0x25346143  // psel p3.b, p8.b/Z, p10.b[w12, #2]\n"
      ".inst 0x25746142  // psel p2.b, p8.b/Z, p10.b[w12, #6]\n"
      ".inst 0x25266d21  // psel p1.b, p11.b/Z, p9.b[w14]\n"
      ".inst 0x252e6d20  // psel p0.b, p11.b/Z, p9.b[w14, #1]\n"
      "mov x23, %x[in]\n"
      "whilelt p9.b, x28, %x[width]\n"
      ".inst 0xe0190ec2  // ld1b { za0h.b[x12, #2] }, p3/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      "incb x28\n"
      "mov x13, #0x0\n"
      ".inst 0xe0190aa6  // ld1b { za0h.b[x12, #6] }, p2/Z, [x21, x25]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "incb x25\n"
      ".inst 0xc082d010  // mova z16.s, p4/M, za0v.s[x14]\n"
      ".inst 0xe0bfc700  // st1w { za0v.s[x14] }, p1/Z, [x24, XZR, LSL #2]\n"
      "whilelt p8.b, x28, %x[width]\n"
      "mov x12, #0x0\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xc082d030  // mova z16.s, p4/M, za0v.s[x14, #1]\n"
      ".inst 0xe0a9c301  // st1w { za0v.s[x14, #1] }, p0/Z, [x24, x9, LSL #2]\n"
      "addvl x24, x24, #2\n"
      "sdot z17.s, z16.b, z18.b\n"
      "cbz x26, 8f\n"
      "7:"  // K loop: Main loop: Second: Loop
      ".inst 0x25256143  // psel p3.b, p8.b/Z, p10.b[w13]\n"
      ".inst 0x25656142  // psel p2.b, p8.b/Z, p10.b[w13, #4]\n"
      ".inst 0x25246d21  // psel p1.b, p11.b/Z, p9.b[w12]\n"
      ".inst 0x252c6d20  // psel p0.b, p11.b/Z, p9.b[w12, #1]\n"
      ".inst 0xe0192ec0  // ld1b { za0h.b[x13] }, p3/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      ".inst 0xe0192aa4  // ld1b { za0h.b[x13, #4] }, p2/Z, [x21, x25]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "add x13, x13, #0x8\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      ".inst 0xe0bf8708  // st1w { za2v.s[x12] }, p1/Z, [x24, XZR, LSL #2]\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xe0a98309  // st1w { za2v.s[x12, #1] }, p0/Z, [x24, x9, LSL #2]\n"
      ".inst 0xc0829130  // mova z16.s, p4/M, za2v.s[x12, #1]\n"
      "add x12, x12, #0x2\n"
      "cmp x12, x26\n"
      "addvl x24, x24, #2\n"
      "sdot z17.s, z16.b, z18.b\n"
      "blt 7b\n"
      "8:"  // K loop: Main loop: Second: Tail
      ".inst 0x25256143  // psel p3.b, p8.b/Z, p10.b[w13]\n"
      ".inst 0x25656142  // psel p2.b, p8.b/Z, p10.b[w13, #4]\n"
      ".inst 0x25246d21  // psel p1.b, p11.b/Z, p9.b[w12]\n"
      ".inst 0x252c6d20  // psel p0.b, p11.b/Z, p9.b[w12, #1]\n"
      "mov x23, %x[in]\n"
      "whilelt p9.b, x28, %x[width]\n"
      ".inst 0xe0192ec0  // ld1b { za0h.b[x13] }, p3/Z, [x22, x25]\n"
      "ldr x22, [x23, #0x0]\n"
      "subs x20, x20, #0x1\n"
      "incb x28\n"
      ".inst 0xe0192aa4  // ld1b { za0h.b[x13, #4] }, p2/Z, [x21, x25]\n"
      "ldr x21, [x23, #0x8]\n"
      "add x23, x23, #0x10\n"
      "incb x25\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      ".inst 0xe0bf8708  // st1w { za2v.s[x12] }, p1/Z, [x24, XZR, LSL #2]\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xc0829130  // mova z16.s, p4/M, za2v.s[x12, #1]\n"
      ".inst 0xe0a98309  // st1w { za2v.s[x12, #1] }, p0/Z, [x24, x9, LSL #2]\n"
      "addvl x24, x24, #2\n"
      "sdot z17.s, z16.b, z18.b\n"
      "bgt 4b\n"
      "9:"  // K loop: Tails
      "cbnz x27, 12f\n"
      "mov x23, %x[in]\n"
      "whilelt p8.b, x28, %x[width]\n"
      "mov x13, #0x0\n"
      "mov x12, #0x0\n"
      "10:"  // K loop: Tails: Even: First
      ".inst 0x25306d21  // psel p1.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0x25356140  // psel p0.b, p8.b/Z, p10.b[w13, #2]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xe0bf8700  // st1w { za0v.s[x12] }, p1/Z, [x24, XZR, LSL #2]\n"
      "add x12, x12, #0x1\n"
      "addvl x24, x24, #1\n"
      "ldr x20, [x23, #0x0]\n"
      "cmp x12, x9\n"
      "add x23, x23, #0x8\n"
      ".inst 0xe0192282  // ld1b { za0h.b[x13, #2] }, p0/Z, [x20, x25]\n"
      "add x13, x13, #0x4\n"
      "blt 10b\n"
      "whilelt p9.b, x28, %x[width]\n"
      "whilelt p8.b, x28, %x[width]\n"
      "mov x20, #0x0\n"
      "mov x12, #0x0\n"
      "11:"  // K loop: Tails: Even: Second
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      "add x20, x20, #0x4\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xe0bf8308  // st1w { za2v.s[x12] }, p0/Z, [x24, XZR, LSL #2]\n"
      "add x12, x12, #0x1\n"
      "addvl x24, x24, #1\n"
      "cmp x12, x10\n"
      "blt 11b\n"
      "whilelt p8.b, x28, %x[width]\n"
      "b 14f\n"
      "12:"  // K loop: Tails: Odd
      "mov x12, #0x0\n"
      "13:"  // K loop: Tails: Odd: Loop
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      "sdot z17.s, z16.b, z18.b\n"
      ".inst 0xe0bf8300  // st1w { za0v.s[x12] }, p0/Z, [x24, XZR, LSL #2]\n"
      "add x12, x12, #0x1\n"
      "addvl x24, x24, #1\n"
      "cmp x12, x10\n"
      "blt 13b\n"
      "14:"  // K loop: End
      "st1w { z17.s }, p4, [x24]\n"
      "addvl x24, x24, #1\n"
      ".inst 0xd503467f  // SMSTOP\n"
      "mov %x[out], x24\n"
      : [out] "+&r" (out)
      : [first] "r" (first), [height] "r" (height), [in] "r" (in), [row_offset] "r" (row_offset), [width] "r" (width)
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x9", "x10", "x12", "x13", "x14", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

#endif  // defined(ARM_COMPUTE_ENABLE_SME)
