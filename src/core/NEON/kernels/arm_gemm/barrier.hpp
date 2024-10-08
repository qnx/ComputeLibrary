/*
 * Copyright (c) 2019-2020, 2024 Arm Limited.
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
#pragma once

#ifndef NO_MULTI_THREADING

#include <atomic>

namespace arm_gemm {

class barrier {
private:
    unsigned int        m_threads;

    std::atomic<unsigned int> m_waiters;
    std::atomic<unsigned int> m_leavers;

public:
    barrier(unsigned int threads) : m_threads(threads), m_waiters(0), m_leavers(0) { }

    // Add a move constructor because these objects might be moved around at setup time.
    // Moving while the barrier is active won't work.
    barrier(barrier &&other) : m_threads(other.m_threads), m_waiters(0), m_leavers(0) {
        // This doesn't make it safe, but will have a chance of firing if something odd is occurring.
        assert(other.m_waiters==0);
        assert(other.m_leavers==0);
    }

    /* This isn't safe if any thread is waiting... */
    void set_nthreads(unsigned int nthreads) {
        m_threads = nthreads;
    }

    void arrive_and_wait() {
        m_waiters++;

        while (m_waiters != m_threads) {
            ; /* spin */
        }

        unsigned int v = m_leavers.fetch_add(1);

        if (v == (m_threads - 1)) {
            m_waiters -= m_threads;
            m_leavers = 0;
        } else {
            while (m_leavers > 0) {
                ; /* spin */
            }
        }
    }
};

} // namespace arm_gemm

#else

namespace arm_gemm {

class barrier {
public:
    barrier(unsigned int) { }

    void arrive_and_wait() { }
    void set_nthreads(unsigned int ) { }
};

} // namespace arm_gemm

#endif
