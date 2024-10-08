/*
 * Copyright (c) 2022, 2024 Arm Limited.
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

#include <algorithm>
#include <cstddef>
#include <arm_neon.h>

namespace arm_conv {
namespace winograd {
namespace output_transform {

void arm_fp32_4x4_3x3(
  unsigned int n_channels,
  const float* inptr,
  const size_t matrix_stride,
  const float* bptr,
  float *outptr,
  const size_t output_row_stride,
  const size_t output_col_stride,
  const float output_min,
  const float output_max
)
{
  constexpr auto output_tile_rows = 4u, output_tile_cols = 4u;

  // For each channel of the output
  for (; n_channels >= 4; n_channels -= 4)
  {
    // Matrices used and computed during this transform
    float32x4_t F[6][6], FZ[6][4], f[4][4], b;

    // Read a 6x6 tile in the Winograd domain
    for (auto i = 0u, m = 0u; i < 6; i++)
    {
      for (auto j = 0u; j < 6; j++, m++)
      {
        F[i][j] = vld1q_f32(inptr + m*matrix_stride);
      }
    }
    inptr += 4;

    // Compute the matrix F Z
    for (auto i = 0u; i < 6; i++)
    {
      // FZ[i][0] =  1*F[i][0] +  1*F[i][1] +  1*F[i][2] +  1*F[i][3] +  1*F[i][4];
      FZ[i][0] = vaddq_f32(vaddq_f32(vaddq_f32(F[i][0], F[i][1]), vaddq_f32(F[i][2], F[i][3])), F[i][4]);

      // FZ[i][1] =  1*F[i][1] + -1*F[i][2] +  2*F[i][3] + -2*F[i][4];
      FZ[i][1] = vmlaq_n_f32(vsubq_f32(F[i][1], F[i][2]), vsubq_f32(F[i][3], F[i][4]), 2.0f);

      // FZ[i][2] =  1*F[i][1] +  1*F[i][2] +  4*F[i][3] +  4*F[i][4];
      FZ[i][2] = vmlaq_n_f32(vaddq_f32(F[i][1], F[i][2]), vaddq_f32(F[i][3], F[i][4]), 4.0f);

      // FZ[i][3] =  1*F[i][1] + -1*F[i][2] +  8*F[i][3] + -8*F[i][4] +  1*F[i][5];
      FZ[i][3] = vaddq_f32(vmlaq_n_f32(vsubq_f32(F[i][1], F[i][2]), vsubq_f32(F[i][3], F[i][4]), 8.0f), F[i][5]);
    }

    // Compute the output tile f = ZT F Z
    for (auto j = 0u; j < 4; j++)
    {
      // f[0][j] =  1*FZ[0][j] +  1*FZ[1][j] +  1*FZ[2][j] +  1*FZ[3][j] +  1*FZ[4][j];
      f[0][j] = vaddq_f32(vaddq_f32(vaddq_f32(FZ[0][j], FZ[1][j]), vaddq_f32(FZ[2][j], FZ[3][j])), FZ[4][j]);

      // f[1][j] =  1*FZ[1][j] + -1*FZ[2][j] +  2*FZ[3][j] + -2*FZ[4][j];
      f[1][j] = vmlaq_n_f32(vsubq_f32(FZ[1][j], FZ[2][j]), vsubq_f32(FZ[3][j], FZ[4][j]), 2.0f);

      // f[2][j] =  1*FZ[1][j] +  1*FZ[2][j] +  4*FZ[3][j] +  4*FZ[4][j];
      f[2][j] = vmlaq_n_f32(vaddq_f32(FZ[1][j], FZ[2][j]), vaddq_f32(FZ[3][j], FZ[4][j]), 4.0f);

      // f[3][j] =  1*FZ[1][j] + -1*FZ[2][j] +  8*FZ[3][j] + -8*FZ[4][j] +  1*FZ[5][j];
      f[3][j] = vaddq_f32(vmlaq_n_f32(vsubq_f32(FZ[1][j], FZ[2][j]), vsubq_f32(FZ[3][j], FZ[4][j]), 8.0f), FZ[5][j]);
    }

    // Write out the output tile
    if (bptr != nullptr)
    {
      b = vld1q_f32(bptr);
      bptr += 4;
    }
    else
    {
      b = vdupq_n_f32(0.0f);
    }
    for (auto i = 0u; i < output_tile_rows; i++)
    {
      for (auto j = 0u; j < output_tile_cols; j++)
      {
        const auto y =
            vmaxq_f32(vminq_f32(vaddq_f32(f[i][j], b), vdupq_n_f32(output_max)),
                     vdupq_n_f32(output_min));
        vst1q_f32(outptr + i*output_row_stride + j*output_col_stride, y);
      }
    }
    outptr += 4;
  }
  for (; n_channels >= 2; n_channels -= 2)
  {
    // Matrices used and computed during this transform
    float32x2_t F[6][6], FZ[6][4], f[4][4], b;

    // Read a 6x6 tile in the Winograd domain
    for (auto i = 0u, m = 0u; i < 6; i++)
    {
      for (auto j = 0u; j < 6; j++, m++)
      {
        F[i][j] = vld1_f32(inptr + m*matrix_stride);
      }
    }
    inptr += 2;

    // Compute the matrix F Z
    for (auto i = 0u; i < 6; i++)
    {
      // FZ[i][0] =  1*F[i][0] +  1*F[i][1] +  1*F[i][2] +  1*F[i][3] +  1*F[i][4];
      FZ[i][0] = vadd_f32(vadd_f32(vadd_f32(F[i][0], F[i][1]), vadd_f32(F[i][2], F[i][3])), F[i][4]);

      // FZ[i][1] =  1*F[i][1] + -1*F[i][2] +  2*F[i][3] + -2*F[i][4];
      FZ[i][1] = vmla_n_f32(vsub_f32(F[i][1], F[i][2]), vsub_f32(F[i][3], F[i][4]), 2.0f);

      // FZ[i][2] =  1*F[i][1] +  1*F[i][2] +  4*F[i][3] +  4*F[i][4];
      FZ[i][2] = vmla_n_f32(vadd_f32(F[i][1], F[i][2]), vadd_f32(F[i][3], F[i][4]), 4.0f);

      // FZ[i][3] =  1*F[i][1] + -1*F[i][2] +  8*F[i][3] + -8*F[i][4] +  1*F[i][5];
      FZ[i][3] = vadd_f32(vmla_n_f32(vsub_f32(F[i][1], F[i][2]), vsub_f32(F[i][3], F[i][4]), 8.0f), F[i][5]);
    }

    // Compute the output tile f = ZT F Z
    for (auto j = 0u; j < 4; j++)
    {
      // f[0][j] =  1*FZ[0][j] +  1*FZ[1][j] +  1*FZ[2][j] +  1*FZ[3][j] +  1*FZ[4][j];
      f[0][j] = vadd_f32(vadd_f32(vadd_f32(FZ[0][j], FZ[1][j]), vadd_f32(FZ[2][j], FZ[3][j])), FZ[4][j]);

      // f[1][j] =  1*FZ[1][j] + -1*FZ[2][j] +  2*FZ[3][j] + -2*FZ[4][j];
      f[1][j] = vmla_n_f32(vsub_f32(FZ[1][j], FZ[2][j]), vsub_f32(FZ[3][j], FZ[4][j]), 2.0f);

      // f[2][j] =  1*FZ[1][j] +  1*FZ[2][j] +  4*FZ[3][j] +  4*FZ[4][j];
      f[2][j] = vmla_n_f32(vadd_f32(FZ[1][j], FZ[2][j]), vadd_f32(FZ[3][j], FZ[4][j]), 4.0f);

      // f[3][j] =  1*FZ[1][j] + -1*FZ[2][j] +  8*FZ[3][j] + -8*FZ[4][j] +  1*FZ[5][j];
      f[3][j] = vadd_f32(vmla_n_f32(vsub_f32(FZ[1][j], FZ[2][j]), vsub_f32(FZ[3][j], FZ[4][j]), 8.0f), FZ[5][j]);
    }

    // Write out the output tile
    if (bptr != nullptr)
    {
      b = vld1_f32(bptr);
      bptr += 2;
    }
    else
    {
      b = vdup_n_f32(0.0f);
    }
    for (auto i = 0u; i < output_tile_rows; i++)
    {
      for (auto j = 0u; j < output_tile_cols; j++)
      {
        const auto y =
            vmax_f32(vmin_f32(vadd_f32(f[i][j], b), vdup_n_f32(output_max)),
                     vdup_n_f32(output_min));
        vst1_f32(outptr + i*output_row_stride + j*output_col_stride, y);
      }
    }
    outptr += 2;
  }
  for (; n_channels; n_channels--)
  {
    // Matrices used and computed during this transform
    float F[6][6], FZ[6][4], f[4][4], b;

    // Read a 6x6 tile in the Winograd domain
    for (auto i = 0u, m = 0u; i < 6; i++)
    {
      for (auto j = 0u; j < 6; j++, m++)
      {
        F[i][j] = *(inptr + m*matrix_stride);
      }
    }
    inptr++;

    // Compute the matrix F Z
    for (auto i = 0u; i < 6; i++)
    {
      FZ[i][0] =  1*F[i][0] +  1*F[i][1] +  1*F[i][2] +  1*F[i][3] +  1*F[i][4];
      FZ[i][1] =  1*F[i][1] + -1*F[i][2] +  2*F[i][3] + -2*F[i][4];
      FZ[i][2] =  1*F[i][1] +  1*F[i][2] +  4*F[i][3] +  4*F[i][4];
      FZ[i][3] =  1*F[i][1] + -1*F[i][2] +  8*F[i][3] + -8*F[i][4] +  1*F[i][5];
    }

    // Compute the output tile f = ZT F Z
    for (auto j = 0u; j < 4; j++)
    {
      f[0][j] =  1*FZ[0][j] +  1*FZ[1][j] +  1*FZ[2][j] +  1*FZ[3][j] +  1*FZ[4][j];
      f[1][j] =  1*FZ[1][j] + -1*FZ[2][j] +  2*FZ[3][j] + -2*FZ[4][j];
      f[2][j] =  1*FZ[1][j] +  1*FZ[2][j] +  4*FZ[3][j] +  4*FZ[4][j];
      f[3][j] =  1*FZ[1][j] + -1*FZ[2][j] +  8*FZ[3][j] + -8*FZ[4][j] +  1*FZ[5][j];
    }

    // Write out the output tile
    if (bptr != nullptr)
    {
      b = *(bptr++);
    }
    else
    {
      b = 0.0f;
    }
    for (auto i = 0u; i < output_tile_rows; i++)
    {
      for (auto j = 0u; j < output_tile_cols; j++)
      {
        const auto y = std::max(std::min(f[i][j] + b, output_max), output_min);
        *(outptr + i*output_row_stride + j*output_col_stride) = y;
      }
    }
    outptr++;
  }
}

}  // namespace output_transform
}  // namespace winograd
}  // namespace arm_conv
