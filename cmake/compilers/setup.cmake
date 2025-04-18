# Copyright (c) 2025 Arm Limited.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/clang.cmake)

# Common definitions for all projects.
set(
  ARM_COMPUTE_DEFINES

  ARCH_ARM
  ARM_COMPUTE_CPU_ENABLED
  ARM_COMPUTE_GRAPH_ENABLED

  # Kernels to build.
  ARM_COMPUTE_ENABLE_NEON
  ARM_COMPUTE_ENABLE_BF16
  ARM_COMPUTE_ENABLE_FP16
  ARM_COMPUTE_ENABLE_I8MM
  ARM_COMPUTE_ENABLE_SVE
  ARM_COMPUTE_ENABLE_SVE2
  ARM_COMPUTE_ENABLE_SVEF32MM
  ARM_COMPUTE_ENABLE_FIXED_FORMAT_KERNELS
  ENABLE_SVE
  ENABLE_SVE2
  ENABLE_NEON
  ENABLE_FP16_KERNELS
  ENABLE_FP32_KERNELS
  ENABLE_QASYMM8_KERNELS
  ENABLE_QASYMM8_SIGNED_KERNELS
  ENABLE_QSYMM16_KERNELS
  ENABLE_INTEGER_KERNELS
  ENABLE_NHWC_KERNELS
  ENABLE_NCHW_KERNELS

  # Features controlled by options.
  $<$<BOOL:${ARM_COMPUTE_ENABLE_ASSERTS}>:ARM_COMPUTE_ASSERTS_ENABLED>
  $<$<BOOL:${ARM_COMPUTE_ENABLE_CPPTHREADS}>:ARM_COMPUTE_CPP_SCHEDULER>
  $<$<BOOL:${ARM_COMPUTE_ENABLE_LOGGING}>:ARM_COMPUTE_LOGGING_ENABLED>
  $<$<BOOL:${ARM_COMPUTE_ENABLE_OPENMP}>:ARM_COMPUTE_OPENMP_SCHEDULER>
)

# * Select the correct compiler flags based on build configuration.
set(
  ARM_COMPUTE_CCXX_WARNING_FLAGS
  $<IF:$<BOOL:${ARM_COMPUTE_ENABLE_WARNINGS}>,${ARM_COMPUTE_STANDARD_WARNINGS},${ARM_COMPUTE_NO_WARN_FLAG}>
  CACHE STRING "Overrides ARM_COMPUTE_STANDARD_WARNINGS and ARM_COMPUTE_NO_WARN_FLAG."
)

set(
  ARM_COMPUTE_CCXX_FLAGS
  ${ARM_COMPUTE_CCXX_FLAGS_INIT}
  $<IF:$<CONFIG:Debug>,${ARM_COMPUTE_CCXX_FLAGS_DEBUG},${ARM_COMPUTE_CCXX_FLAGS_RELEASE}>
  CACHE STRING "Overrides ARM_COMPUTE_CCXX_FLAGS_INIT, ARM_COMPUTE_CCXX_FLAGS_DEBUG and ARM_COMPUTE_CCXX_FLAGS_RELEASE"
)

set(
  ARM_COMPUTE_LINKER_FLAGS
  ${ARM_COMPUTE_LINKER_FLAGS_INIT}
  $<IF:$<CONFIG:Debug>,${ARM_COMPUTE_LINKER_FLAGS_DEBUG},${ARM_COMPUTE_LINKER_FLAGS_RELEASE}>
  CACHE STRING "Active linker flags."
)

if(ARM_COMPUTE_ENABLE_SANITIZERS)
  set(ARM_COMPUTE_ASAN_COMPILER_FLAG $<$<CONFIG:DEBUG>:${ARM_COMPUTE_ASAN_COMPILER_FLAG_INIT}>)
  list(APPEND ARM_COMPUTE_LINKER_FLAGS $<$<CONFIG:DEBUG>:${ARM_COMPUTE_ASAN_LINKER_FLAG_INIT}>)
endif()

if(ARM_COMPUTE_ENABLE_CODECOVERAGE)
  set(ARM_COMPUTE_CODE_COVERAGE_COMPILER_FLAG $<$<CONFIG:DEBUG>:${ARM_COMPUTE_CODE_COVERAGE_COMPILER_FLAG}>)
  list(APPEND ARM_COMPUTE_LINKER_FLAGS $<$<CONFIG:DEBUG>:${ARM_COMPUTE_CODE_COVERAGE_LINKER_FLAG}>)
endif()

set(
  ARM_COMPUTE_LINK_LIBS
  $<$<BOOL:${ARM_COMPUTE_ENABLE_OPENMP}>:OpenMP::OpenMP_CXX>
  $<$<BOOL:${ARM_COMPUTE_ENABLE_OPENMP}>:OpenMP::OpenMP_C>
)

if(ARM_COMPUTE_USE_LIBCXX)
  list(APPEND ARM_COMPUTE_CCXX_FLAGS ${ARM_COMPUTE_LIBCXX_COMPILER_FLAG})
  list(APPEND ARM_COMPUTE_LINKER_FLAGS ${ARM_COMPUTE_LIBCXX_LINKER_FLAG})
endif()

if(ARM_COMPUTE_USE_LLD)
  list(APPEND ARM_COMPUTE_LINKER_FLAGS ${ARM_COMPUTE_LLD_LINKER_FLAGS})
endif()

set(
  ARM_COMPUTE_COMMON_CCXX_FLAGS
  ${ARM_COMPUTE_CCXX_FLAGS}
  ${ARM_COMPUTE_CCXX_WARNING_FLAGS}
  ${ARM_COMPUTE_ASAN_COMPILER_FLAG}
  ${ARM_COMPUTE_CODE_COVERAGE_COMPILER_FLAG}
)

set(
  ARM_COMPUTE_COMMON_CCXX_FLAGS
  ${ARM_COMPUTE_CCXX_FLAGS}
  ${ARM_COMPUTE_CCXX_WARNING_FLAGS}
  ${ARM_COMPUTE_ASAN_COMPILER_FLAG}
  ${ARM_COMPUTE_CODE_COVERAGE_COMPILER_FLAG}
)

# Add -march to arch values.
string(PREPEND ARM_COMPUTE_ARCH -march=)
string(PREPEND ARM_COMPUTE_CORE_FP16_ARCH -march=)
string(PREPEND ARM_COMPUTE_SVE_ARCH -march=)
string(PREPEND ARM_COMPUTE_SVE2_ARCH -march=)

# Remove any CMake additions so we have clean build line.
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_INIT "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "" FORCE)
