/*
* Copyright (c) 2019
* Author(s): Marcus D. R. Klarqvist, Wojciech Muła, and Daniel Lemire
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*/
#include <string.h> //memset
#include <assert.h> //assert

#include "pospopcnt.h"

#if __clang__ == 1 || __llvm__ == 1
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsequenced"
#endif

int pospopcnt_u16(const uint16_t* data, uint32_t len, uint32_t* flags) {
#if POSPOPCNT_SIMD_VERSION >= 6
    if (len < 32) return(pospopcnt_u16_sse_sad(data, len, flags)); // small
    else if (len < 256)  return(pospopcnt_u16_sse_blend_popcnt_unroll8(data, len, flags)); // small
    else if (len < 512)  return(pospopcnt_u16_avx512bw_blend_popcnt_unroll8(data, len, flags)); // medium
    else if (len < 4096) return(pospopcnt_u16_avx512bw_adder_forest(data, len, flags)); // medium3
    else return(pospopcnt_u16_avx512_harley_seal(data, len, flags)); // fix
#elif POSPOPCNT_SIMD_VERSION >= 5
    if (len < 128) return(pospopcnt_u16_sse_sad(data, len, flags)); // small
    else if (len < 1024) return(pospopcnt_u16_avx2_blend_popcnt_unroll8(data, len, flags)); // medium
    else return(pospopcnt_u16_avx2_harley_seal(data, len, flags)); // large
#elif POSPOPCNT_SIMD_VERSION >= 3
    return(pospopcnt_u16_sse_harley_seal(data, len, flags));
#else
    #ifndef _MSC_VER
        return(pospopcnt_u16_scalar_umul128_unroll2(data, len, flags)); // fallback scalar
    #else
        return(pospopcnt_u16_scalar_naive(data, len, flags));
    #endif
#endif
}

int pospopcnt_u16_method(PPOPCNT_U16_METHODS method, const uint16_t* data, uint32_t len, uint32_t* flags) {
    pospopcnt_u16_method_type pospopcnt_u16 = get_pospopcnt_u16_method(method);
    return pospopcnt_u16(data, len, flags);
}

pospopcnt_u16_method_type get_pospopcnt_u16_method(PPOPCNT_U16_METHODS method) {
    switch(method) {
    case(PPOPCNT_AUTO): return pospopcnt_u16;
    case(PPOPCNT_SCALAR): return pospopcnt_u16_scalar_naive;
    case(PPOPCNT_SCALAR_NOSIMD): return pospopcnt_u16_scalar_naive_nosimd;
    case(PPOPCNT_SCALAR_PARTITION): return pospopcnt_u16_scalar_partition;
    case(PPOPCNT_SCALAR_HIST1X4): return pospopcnt_u16_scalar_hist1x4;
    case(PPOPCNT_SCALAR_UMUL128): return pospopcnt_u16_scalar_umul128;
    case(PPOPCNT_SCALAR_UMUL128_UR2): return pospopcnt_u16_scalar_umul128_unroll2;
    case(PPOPCNT_SSE_SINGLE): return pospopcnt_u16_sse_single;
    case(PPOPCNT_SSE_BLEND_POPCNT): return pospopcnt_u16_sse_blend_popcnt;
    case(PPOPCNT_SSE_BLEND_POPCNT_UR4): return pospopcnt_u16_sse_blend_popcnt_unroll4;
    case(PPOPCNT_SSE_BLEND_POPCNT_UR8): return pospopcnt_u16_sse_blend_popcnt_unroll8;
    case(PPOPCNT_SSE_BLEND_POPCNT_UR16): return pospopcnt_u16_sse_blend_popcnt_unroll16;
    case(PPOPCNT_SSE_SAD): return pospopcnt_u16_sse_sad;
    case(PPOPCNT_SSE_HARLEY_SEAL): return pospopcnt_u16_sse_harley_seal;
    case(PPOPCNT_SSE_HARLEY_SEAL_IMPROVED): return pospopcnt_u16_sse_harley_seal_improved;
    case(PPOPCNT_AVX2_POPCNT): return pospopcnt_u16_avx2_popcnt;
    case(PPOPCNT_AVX2): return pospopcnt_u16_avx2;
    case(PPOPCNT_AVX2_POPCNT_NAIVE): return pospopcnt_u16_avx2_naive_counter;
    case(PPOPCNT_AVX2_SINGLE): return pospopcnt_u16_avx2_single;
    case(PPOPCNT_AVX2_LEMIRE1): return pospopcnt_u16_avx2_lemire;
    case(PPOPCNT_AVX2_LEMIRE2): return pospopcnt_u16_avx2_lemire2;
    case(PPOPCNT_AVX2_BLEND_POPCNT): return pospopcnt_u16_avx2_blend_popcnt;
    case(PPOPCNT_AVX2_BLEND_POPCNT_UR4): return pospopcnt_u16_avx2_blend_popcnt_unroll4;
    case(PPOPCNT_AVX2_BLEND_POPCNT_UR8): return pospopcnt_u16_avx2_blend_popcnt_unroll8;
    case(PPOPCNT_AVX2_BLEND_POPCNT_UR16): return pospopcnt_u16_avx2_blend_popcnt_unroll16;
    case(PPOPCNT_AVX2_ADDER_FOREST): return pospopcnt_u16_avx2_adder_forest;
    case(PPOPCNT_AVX2_HARLEY_SEAL): return pospopcnt_u16_avx2_harley_seal;
    case(PPOPCNT_AVX2_HARLEY_SEAL_IMPROVED): return pospopcnt_u16_avx2_harley_seal_improved;
    case(PPOPCNT_AVX512): return pospopcnt_u16_avx512;
    case(PPOPCNT_AVX512BW_MASK32): return pospopcnt_u16_avx512bw_popcnt32_mask;
    case(PPOPCNT_AVX512BW_MASK64): return pospopcnt_u16_avx512bw_popcnt64_mask;
    case(PPOPCNT_AVX512_MASKED_OPS): return pospopcnt_u16_avx512_masked_ops;
    case(PPOPCNT_AVX512_POPCNT): return pospopcnt_u16_avx512_popcnt;
    case(PPOPCNT_AVX512BW_BLEND_POPCNT): return pospopcnt_u16_avx512bw_blend_popcnt;
    case(PPOPCNT_AVX512BW_BLEND_POPCNT_UR4): return pospopcnt_u16_avx512bw_blend_popcnt_unroll4;
    case(PPOPCNT_AVX512BW_BLEND_POPCNT_UR8): return pospopcnt_u16_avx512bw_blend_popcnt_unroll8;
    case(PPOPCNT_AVX512BW_ADDER_FOREST): return pospopcnt_u16_avx512bw_adder_forest;
    case(PPOPCNT_AVX512_MULA2): return pospopcnt_u16_avx512_mula2;
    case(PPOPCNT_AVX512BW_HARLEY_SEAL): return pospopcnt_u16_avx512bw_harley_seal;
    case(PPOPCNT_AVX512VBMI_HARLEY_SEAL): return pospopcnt_u16_avx512vbmi_harley_seal;
    case PPOPCNT_NUMBER_METHODS: break; /* -Wswitch */
    }
    assert(0);
    return 0; /* unreachable, but some compilers complain without it */
}

pospopcnt_u8_method_type get_pospopcnt_u8_method(PPOPCNT_U8_METHODS method) {
    switch(method) {
    case PPOPCNT_U8_AUTO: return pospopcnt_u8_scalar_naive; /* TODO: implement something similar to pospopcnt_u16 */
    case PPOPCNT_U8_SCALAR: return pospopcnt_u8_scalar_naive;
    case PPOPCNT_U8_SCALAR_NOSIMD: return pospopcnt_u8_scalar_naive_nosimd;
    case PPOPCNT_U8_SCALAR_PARTITION: return pospopcnt_u8_scalar_partition;
    case PPOPCNT_U8_SCALAR_HIST1X4: return pospopcnt_u8_scalar_hist1x4;
    case PPOPCNT_U8_SCALAR_UMUL128: return pospopcnt_u8_scalar_umul128;
    case PPOPCNT_U8_SCALAR_UMUL128_UR2: return pospopcnt_u8_scalar_umul128_unroll2;
    case PPOPCNT_U8_SSE_SINGLE: return pospopcnt_u8_sse_single;
    case PPOPCNT_U8_SSE_BLEND_POPCNT: return pospopcnt_u8_sse_blend_popcnt;
    case PPOPCNT_U8_SSE_BLEND_POPCNT_UR4: return pospopcnt_u8_sse_blend_popcnt_unroll4;
    case PPOPCNT_U8_SSE_BLEND_POPCNT_UR8: return pospopcnt_u8_sse_blend_popcnt_unroll8;
    case PPOPCNT_U8_SSE_BLEND_POPCNT_UR16: return pospopcnt_u8_sse_blend_popcnt_unroll16;
    case PPOPCNT_U8_SSE_SAD: return pospopcnt_u8_sse_sad;
    case PPOPCNT_U8_SSE_HARLEY_SEAL: return pospopcnt_u8_sse_harley_seal;
    case PPOPCNT_U8_SSE_POPCNT4BIT: return pospopcnt_u8_sse_popcnt4bit;
    case PPOPCNT_U8_SSE_HORIZREDUCE: return pospopcnt_u8_sse_horizreduce;
    case PPOPCNT_U8_AVX2_POPCNT: return pospopcnt_u8_avx2_popcnt;
    case PPOPCNT_U8_AVX2: return pospopcnt_u8_avx2;
    case PPOPCNT_U8_AVX2_POPCNT_NAIVE: return pospopcnt_u8_avx2_naive_counter;
    case PPOPCNT_U8_AVX2_SINGLE: return pospopcnt_u8_avx2_single;
    case PPOPCNT_U8_AVX2_LEMIRE1: return pospopcnt_u8_avx2_lemire;
    case PPOPCNT_U8_AVX2_LEMIRE2: return pospopcnt_u8_avx2_lemire2;
    case PPOPCNT_U8_AVX2_BLEND_POPCNT: return pospopcnt_u8_avx2_blend_popcnt;
    case PPOPCNT_U8_AVX2_BLEND_POPCNT_UR4: return pospopcnt_u8_avx2_blend_popcnt_unroll4;
    case PPOPCNT_U8_AVX2_BLEND_POPCNT_UR8: return pospopcnt_u8_avx2_blend_popcnt_unroll8;
    case PPOPCNT_U8_AVX2_BLEND_POPCNT_UR16: return pospopcnt_u8_avx2_blend_popcnt_unroll16;
    case PPOPCNT_U8_AVX2_ADDER_FOREST: return pospopcnt_u8_avx2_adder_forest;
    case PPOPCNT_U8_AVX2_HARLEY_SEAL: return pospopcnt_u8_avx2_harley_seal;
    case PPOPCNT_U8_AVX2_POPCNT4BIT: return pospopcnt_u8_avx2_popcnt4bit;
    case PPOPCNT_U8_AVX2_HORIZREDUCE: return pospopcnt_u8_avx2_horizreduce;
    case PPOPCNT_U8_AVX512: return pospopcnt_u8_avx512;
    case PPOPCNT_U8_AVX512BW_MASK32: return pospopcnt_u8_avx512bw_popcnt32_mask;
    case PPOPCNT_U8_AVX512BW_MASK64: return pospopcnt_u8_avx512bw_popcnt64_mask;
    case PPOPCNT_U8_AVX512_MASKED_OPS: return pospopcnt_u8_avx512_masked_ops;
    case PPOPCNT_U8_AVX512_POPCNT: return pospopcnt_u8_avx512_popcnt;
    case PPOPCNT_U8_AVX512BW_BLEND_POPCNT: return pospopcnt_u8_avx512bw_blend_popcnt;
    case PPOPCNT_U8_AVX512BW_BLEND_POPCNT_UR4: return pospopcnt_u8_avx512bw_blend_popcnt_unroll4;
    case PPOPCNT_U8_AVX512BW_BLEND_POPCNT_UR8: return pospopcnt_u8_avx512bw_blend_popcnt_unroll8;
    case PPOPCNT_U8_AVX512BW_ADDER_FOREST: return pospopcnt_u8_avx512bw_adder_forest;
    case PPOPCNT_U8_AVX512_MULA2: return pospopcnt_u8_avx512_mula2;
    case PPOPCNT_U8_AVX512BW_HARLEY_SEAL: return pospopcnt_u8_avx512bw_harley_seal;
    case PPOPCNT_U8_AVX512BW_POPCNT4BIT: return pospopcnt_u8_avx512bw_popcnt4bit;
    case PPOPCNT_U8_AVX512BW_SADBW: return pospopcnt_u8_avx512bw_sadbw;
    case PPOPCNT_U8_AVX512VBMI_HARLEY_SEAL: return pospopcnt_u8_avx512vbmi_harley_seal;
    case PPOPCNT_U8_NUMBER_METHODS: break; /* -Wswitch */
    }
    assert(0);
    return 0; /* unreachable, but some compilers complain without it */
}

pospopcnt_u32_method_type get_pospopcnt_u32_method(PPOPCNT_U32_METHODS method) {
    switch(method) {
    case PPOPCNT_U32_AUTO: return pospopcnt_u32_scalar_naive; /* TODO: implement something similar to pospopcnt_u16 */
    case PPOPCNT_U32_SCALAR: return pospopcnt_u32_scalar_naive;
    case PPOPCNT_U32_SSE_HARLEY_SEAL: return pospopcnt_u32_sse_harley_seal;
    case PPOPCNT_U32_SSE_HARLEY_SEAL_IMPROVED: return pospopcnt_u32_sse_harley_seal_improved;
    case PPOPCNT_U32_AVX2_HARLEY_SEAL: return pospopcnt_u32_avx2_harley_seal;
    case PPOPCNT_U32_AVX2_HARLEY_SEAL_IMPROVED: return pospopcnt_u32_avx2_harley_seal_improved;
    case PPOPCNT_U32_NUMBER_METHODS: break; /* -Wswitch */
    }
    assert(0);
    return 0; /* unreachable, but some compilers complain without it */
}

void pospopcnt_u8_scalar_naive(const uint8_t* data, size_t len, uint32_t* out) {
    for (size_t i = 0; i < len; ++i) {
        for (int j = 0; j < 8; ++j) {
            out[j] += ((data[i] & (1 << j)) >> j);
        }
    }
}

#define pospopcnt_u16_stub(name) \
    int name(const uint16_t* data, uint32_t len, uint32_t* flags) { (void)data; (void)len; (void)flags; return(0); }

#define pospopcnt_u8_stub(name) \
    void name(const uint8_t* data, size_t len, uint32_t* flags) { (void)data; (void)len; (void)flags; }

#define pospopcnt_u32_stub(name) \
    void name(const uint32_t* data, size_t len, uint32_t* flags) { (void)data; (void)len; (void)flags; }

// A pospopcnt 8-bit procedure can be expressed with a 16-bit procedure
// Following macro is used to define such wrapper.
#define make_pospopcnt_u8_from_u16(u8_function, pospopcnt_u16_function) \
void u8_function(const uint8_t* data, size_t len, uint32_t* flags) {    \
    uint32_t pospopcnt16[16] = {0};                                     \
    pospopcnt_u16_function((uint16_t*)data, len/2, pospopcnt16);        \
    for (int i=0; i < 8; i++)                                           \
        flags[i] = pospopcnt16[i + 0] + pospopcnt16[i + 8];             \
    if (len % 2 == 1) /* update the last byte, if len is odd */         \
        pospopcnt_u8_scalar_naive_single(data[len - 1], flags);         \
}


void pospopcnt_u8_scalar_naive_single(uint8_t data, uint32_t* out) {
    for (int i = 0; i < 8; ++i)
        out[i] += ((data & (1 << i)) >> i);
}


void pospopcnt_u32_scalar_naive(const uint32_t* data, size_t len, uint32_t* out) {
    for (size_t i = 0; i < len; ++i) {
        for (int j = 0; j < 32; ++j) {
            out[j] += ((data[i] & (1 << j)) >> j);
        }
    }
}

#if POSPOPCNT_SIMD_VERSION >= 5
int pospopcnt_u16_avx2_popcnt(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m256i masks[16];
    __m256i stubs[16];
    for (int i = 0; i < 16; ++i) {
        masks[i] = _mm256_set1_epi16(1 << i);
        stubs[i] = _mm256_set1_epi16(0);
    }

    uint32_t out_counters[16] = {0};

    const __m256i* data_vectors = (const __m256i*)(data);
    const uint32_t n_cycles = len / 16;
    const uint32_t n_cycles_updates = n_cycles / 16;

#define UPDATE(idx, shift) stubs[idx] = _mm256_or_si256(stubs[idx], _mm256_slli_epi16(_mm256_srli_epi16(_mm256_and_si256(_mm256_loadu_si256(data_vectors+pos), masks[idx]),  idx), shift));
#define ITERATION(idx) {                                           \
    UPDATE(0,idx);  UPDATE(1,idx);  UPDATE(2,idx);  UPDATE(3,idx); \
    UPDATE(4,idx);  UPDATE(5,idx);  UPDATE(6,idx);  UPDATE(7,idx); \
    UPDATE(8,idx);  UPDATE(9,idx);  UPDATE(10,idx); UPDATE(11,idx);\
    UPDATE(12,idx); UPDATE(13,idx); UPDATE(14,idx); UPDATE(15,idx);\
    ++pos;                                                         \
}
#define BLOCK {                                                \
    ITERATION(0);  ITERATION(1);  ITERATION(2);  ITERATION(3); \
    ITERATION(4);  ITERATION(5);  ITERATION(6);  ITERATION(7); \
    ITERATION(8);  ITERATION(9);  ITERATION(10); ITERATION(11);\
    ITERATION(12); ITERATION(13); ITERATION(14); ITERATION(15);\
}

    uint32_t pos = 0;
    for (size_t i = 0; i < n_cycles_updates; ++i) {
        BLOCK // unrolled
        
        // Not unrolled
        /*
        for (int c = 0; c < 16; ++c) { // 16 iterations per register
            ITERATION(c)
            // for (int j = 0; j < 16; ++j) { // each 1-hot per register
            //     UPDATE(j,c)
            // }
        }
        */

        for (int j = 0; j < 16; ++j) {
            PIL_POPCOUNT_AVX2(out_counters[j], stubs[j])
            stubs[j] = _mm256_set1_epi16(0);
        }
    }

    // residual
    for (size_t i = pos*16; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            out_counters[j] += ((data[i] & (1 << j)) >> j);
        }
    }

    for (int i = 0; i < 16; ++i)
        flags[i] = out_counters[i];

#undef BLOCK
#undef ITERATION
#undef UPDATE

    return 0;
}

int pospopcnt_u16_avx2(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m256i masks[16];
    __m256i counters[16];
    for (int i = 0; i < 16; ++i) {
        masks[i]    = _mm256_set1_epi16(1 << i);
        counters[i] = _mm256_set1_epi16(0);
    }
    uint32_t out_counters[16] = {0};

    const __m256i hi_mask = _mm256_set1_epi32((int32_t)0xFFFF0000);
    const __m256i lo_mask = _mm256_set1_epi32(0x0000FFFF);
    const __m256i* data_vectors = (const __m256i*)(data);
    const uint32_t n_cycles = len / 16;
    const uint32_t n_update_cycles = n_cycles / 65536;

#define UPDATE(idx) counters[idx]  = _mm256_add_epi16(counters[idx],  _mm256_srli_epi16(_mm256_and_si256(_mm256_loadu_si256(data_vectors+pos), masks[idx]),  idx))
#define ITERATION  {                               \
    UPDATE(0);  UPDATE(1);  UPDATE(2);  UPDATE(3); \
    UPDATE(4);  UPDATE(5);  UPDATE(6);  UPDATE(7); \
    UPDATE(8);  UPDATE(9);  UPDATE(10); UPDATE(11);\
    UPDATE(12); UPDATE(13); UPDATE(14); UPDATE(15);\
    ++pos;                                         \
}
    uint32_t pos = 0;
    for (size_t i = 0; i < n_update_cycles; ++i) { // each block of 2^16 values
        for (int k = 0; k < 65536; ++k) // max sum of each 16-bit value in a register
            ITERATION // unrolled

        // Compute vector sum
        for (int k = 0; k < 16; ++k) { // each flag register
            // Accumulator
            // ((16-bit high & 16 high) >> 16) + (16-bit low & 16-low)
            __m256i x = _mm256_add_epi32(
                           _mm256_srli_epi32(_mm256_and_si256(counters[k], hi_mask), 16),
                           _mm256_and_si256(counters[k], lo_mask));
            __m256i t1 = _mm256_hadd_epi32(x,x);
            __m256i t2 = _mm256_hadd_epi32(t1,t1);
            __m128i t4 = _mm_add_epi32(_mm256_castsi256_si128(t2),_mm256_extractf128_si256(t2,1));
            out_counters[k] += _mm_cvtsi128_si32(t4);

            /*
            // Naive counter
            uint16_t* d = (uint16_t*)(&counters[k]);
            for (int j = 0; j < 16; ++j) { // each uint16_t in the register
                out_counters[k] += d[j];
            }
            */

            counters[k] = _mm256_set1_epi16(0);
        }
    }

    // residual
    for (size_t i = pos*16; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

#undef ITERATION
#undef UPDATE

    return 0;
}

int pospopcnt_u16_avx2_naive_counter(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m256i masks[16];
    __m256i counters[16];
    for (int i = 0; i < 16; ++i) {
        masks[i]    = _mm256_set1_epi16(1 << i);
        counters[i] = _mm256_set1_epi16(0);
    }
    uint32_t out_counters[16] = {0};

    const __m256i* data_vectors = (const __m256i*)(data);
    const uint32_t n_cycles = len / 16;
    const uint32_t n_update_cycles = n_cycles / 65536;

#define UPDATE(idx) counters[idx]  = _mm256_add_epi16(counters[idx],  _mm256_srli_epi16(_mm256_and_si256(_mm256_loadu_si256(data_vectors+pos), masks[idx]),  idx))

    uint32_t pos = 0;
    for (size_t i = 0; i < n_update_cycles; ++i) { // each block of 2^16 values
        for (int k = 0; k < 65536; ++pos,++k) { // max sum of each 16-bit value in a register
            for (int p = 0; p < 16; ++p) // Not unrolled
                UPDATE(p);
        }

        // Compute vector sum
        for (int k = 0; k < 16; ++k) { // each flag register
            // Naive counter
            uint16_t* d = (uint16_t*)(&counters[k]);
            for (int j = 0; j < 16; ++j) // each uint16_t in the register
                out_counters[k] += d[j];

            counters[k] = _mm256_set1_epi16(0);
        }
    }

    // residual
    for (size_t i = pos*16; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

#undef UPDATE

    return 0;
}

int pospopcnt_u16_avx2_single(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m256i counter = _mm256_set1_epi16(0);
    const __m256i one_mask =  _mm256_set1_epi16(1);
    // set_epi is parameterized backwards (15->0)
    const __m256i masks = _mm256_set_epi16(-32768, 1 << 14, 1 << 13, 1 << 12,
                                           1 << 11, 1 << 10, 1 << 9,  1 << 8,
                                           1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                           1 << 3,  1 << 2,  1 << 1,  1 << 0);
    uint32_t out_counters[16] = {0};
    const __m256i* data_vectors = (const __m256i*)(data);
    const uint32_t n_cycles = len / 16;
    const uint32_t n_update_cycles = n_cycles / 4096;

#define UPDATE(idx) counter = _mm256_add_epi16(counter, _mm256_and_si256(_mm256_cmpeq_epi16(_mm256_and_si256(_mm256_set1_epi16(_mm256_extract_epi16(_mm256_loadu_si256(data_vectors+pos), idx)), masks), masks), one_mask));
#define BLOCK {                                 \
    UPDATE(0)  UPDATE(1)  UPDATE(2)  UPDATE(3)  \
    UPDATE(4)  UPDATE(5)  UPDATE(6)  UPDATE(7)  \
    UPDATE(8)  UPDATE(9)  UPDATE(10) UPDATE(11) \
    UPDATE(12) UPDATE(13) UPDATE(14) UPDATE(15) \
}

    uint32_t pos = 0;
    for (size_t i = 0; i < n_update_cycles; ++i) { // each block of 65536 values
        for (int k = 0; k < 4096; ++k, ++pos) { // max sum of each 16-bit value in a register (65536/16)
            BLOCK
        }

        // Compute vector sum
        // Unroll to prevent clang (LLVM) from throwing constant integer error such as:
        // error: argument to '__builtin_ia32_vec_ext_v16hi' must be a constant integer
        out_counters[0]  += _mm256_extract_epi16(counter,  0);
        out_counters[1]  += _mm256_extract_epi16(counter,  1);
        out_counters[2]  += _mm256_extract_epi16(counter,  2);
        out_counters[3]  += _mm256_extract_epi16(counter,  3);
        out_counters[4]  += _mm256_extract_epi16(counter,  4);
        out_counters[5]  += _mm256_extract_epi16(counter,  5);
        out_counters[6]  += _mm256_extract_epi16(counter,  6);
        out_counters[7]  += _mm256_extract_epi16(counter,  7);
        out_counters[8]  += _mm256_extract_epi16(counter,  8);
        out_counters[9]  += _mm256_extract_epi16(counter,  9);
        out_counters[10] += _mm256_extract_epi16(counter, 10);
        out_counters[11] += _mm256_extract_epi16(counter, 11);
        out_counters[12] += _mm256_extract_epi16(counter, 12);
        out_counters[13] += _mm256_extract_epi16(counter, 13);
        out_counters[14] += _mm256_extract_epi16(counter, 14);
        out_counters[15] += _mm256_extract_epi16(counter, 15);

        counter = _mm256_set1_epi16(0);
    }

#undef UPDATE
#undef BLOCK

    // residual
    for (size_t i = pos*16; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i)
        flags[i] = out_counters[i];

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_popcnt, pospopcnt_u16_avx2_popcnt)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2, pospopcnt_u16_avx2)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_naive_counter, pospopcnt_u16_avx2_naive_counter)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_single, pospopcnt_u16_avx2_single)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx2_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_avx2)
pospopcnt_u16_stub(pospopcnt_u16_avx2_naive_counter)
pospopcnt_u16_stub(pospopcnt_u16_avx2_single)
pospopcnt_u8_stub(pospopcnt_u8_avx2_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_avx2)
pospopcnt_u8_stub(pospopcnt_u8_avx2_naive_counter)
pospopcnt_u8_stub(pospopcnt_u8_avx2_single)
#endif

#if POSPOPCNT_SIMD_VERSION >= 3
int pospopcnt_u16_sse_single(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m128i counterLo = _mm_set1_epi16(0);
    __m128i counterHi = _mm_set1_epi16(0);
    const __m128i one_mask =  _mm_set1_epi16(1);
    // set_epi is parameterized backwards (15->0)
    const __m128i masksLo = _mm_set_epi16(-32768, 1 << 14, 1 << 13, 1 << 12,
                                          1 << 11, 1 << 10, 1 << 9,  1 << 8);
    const __m128i masksHi = _mm_set_epi16(1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                          1 << 3,  1 << 2,  1 << 1,  1 << 0);

    uint32_t out_counters[16] = {0};
    const __m128i* data_vectors = (const __m128i*)(data);
    const uint32_t n_cycles = len / 8;
    const uint32_t n_update_cycles = n_cycles / 4096;

#define UPDATE_LO(idx) counterLo = _mm_add_epi16(counterLo, _mm_and_si128(_mm_cmpeq_epi16(_mm_and_si128(_mm_set1_epi16(_mm_extract_epi16(_mm_loadu_si128(data_vectors+pos), idx)), masksLo), masksLo), one_mask));
#define UPDATE_HI(idx) counterHi = _mm_add_epi16(counterHi, _mm_and_si128(_mm_cmpeq_epi16(_mm_and_si128(_mm_set1_epi16(_mm_extract_epi16(_mm_loadu_si128(data_vectors+pos), idx)), masksHi), masksHi), one_mask));
#define BLOCK {                                         \
    UPDATE_LO(0) UPDATE_LO(1) UPDATE_LO(2) UPDATE_LO(3) \
    UPDATE_LO(4) UPDATE_LO(5) UPDATE_LO(6) UPDATE_LO(7) \
    UPDATE_HI(0) UPDATE_HI(1) UPDATE_HI(2) UPDATE_HI(3) \
    UPDATE_HI(4) UPDATE_HI(5) UPDATE_HI(6) UPDATE_HI(7) \
}
#define UH(idx) out_counters[idx] += _mm_extract_epi16(counterLo, idx - 8);
#define UL(idx) out_counters[idx] += _mm_extract_epi16(counterHi, idx);

    uint32_t pos = 0;
    for (size_t i = 0; i < n_update_cycles; ++i) { // each block of 65536 values
        for (int k = 0; k < 4096; ++k, ++pos) { // max sum of each 16-bit value in a register (65536/16)
            BLOCK
        }

        // Compute vector sum (unroll to prevent possible compiler errors
        // regarding constness of parameter N in _mm_extract_epi16).
        UL(0)  UL(1)  UL(2)  UL(3)
        UL(4)  UL(5)  UL(6)  UL(7)
        UH(8)  UH(9)  UH(10) UH(11)
        UH(12) UH(13) UH(14) UH(15)
        counterLo = _mm_set1_epi16(0);
        counterHi = _mm_set1_epi16(0);
    }

#undef UL
#undef UH
#undef BLOCK
#undef UPDATE_HI
#undef UPDATE_LO

    // residual
    for (size_t i = pos*8; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

    return 0;
}

// By @aqrit (https://github.com/aqrit)
// @see: https://gist.github.com/aqrit/cb52b2ac5b7d0dfe9319c09d27237bf3
int pospopcnt_u16_sse_sad(const uint16_t* data, uint32_t len, uint32_t* flag_counts) {
    const __m128i zero = _mm_setzero_si128();
    const __m128i mask_lo_byte = _mm_srli_epi16(_mm_cmpeq_epi8(zero, zero), 8);
    const __m128i mask_lo_cnt  = _mm_srli_epi16(mask_lo_byte, 2);
    const __m128i mask_bits_a  = _mm_set1_epi8(0x41); // 01000001
    const __m128i mask_bits_b  = _mm_add_epi8(mask_bits_a, mask_bits_a);
    uint32_t buffer[16];

    __m128i counterA = zero;
    __m128i counterB = zero;
    __m128i counterC = zero;
    __m128i counterD = zero;

    for (const uint16_t* end = &data[(len & ~31)]; data != end; data += 32) {
        __m128i r0 = _mm_loadu_si128((__m128i*)&data[0]);
        __m128i r1 = _mm_loadu_si128((__m128i*)&data[8]);
        __m128i r2 = _mm_loadu_si128((__m128i*)&data[16]);
        __m128i r3 = _mm_loadu_si128((__m128i*)&data[24]);
        __m128i r4, r5, r6, r7;

        // seperate LOBYTE and HIBYTE of each WORD
        // (emulate PSHUFB F,D,B,9,7,5,3,1, E,C,A,8,6,4,2,0)
        r4 = _mm_and_si128(mask_lo_byte, r0);
        r5 = _mm_and_si128(mask_lo_byte, r1);
        r6 = _mm_and_si128(mask_lo_byte, r2);
        r7 = _mm_and_si128(mask_lo_byte, r3);
        r0 = _mm_srli_epi16(r0, 8);
        r1 = _mm_srli_epi16(r1, 8);
        r2 = _mm_srli_epi16(r2, 8);
        r3 = _mm_srli_epi16(r3, 8);
        r0 = _mm_packus_epi16(r0, r4);
        r1 = _mm_packus_epi16(r1, r5);
        r2 = _mm_packus_epi16(r2, r6);
        r3 = _mm_packus_epi16(r3, r7);

        // isolate bits to count
        r4 = _mm_and_si128(mask_bits_a, r0);
        r5 = _mm_and_si128(mask_bits_a, r1);
        r6 = _mm_and_si128(mask_bits_a, r2);
        r7 = _mm_and_si128(mask_bits_a, r3);

        // horizontal sum of qwords
        r4 = _mm_sad_epu8(r4, zero);
        r5 = _mm_sad_epu8(r5, zero);
        r6 = _mm_sad_epu8(r6, zero);
        r7 = _mm_sad_epu8(r7, zero);

        // sum 6-bit counts
        r4 = _mm_add_epi16(r4,r5);
        r4 = _mm_add_epi16(r4,r6);
        r4 = _mm_add_epi16(r4,r7);

        // unpack 6-bit counts to 32-bits
        r5 = _mm_and_si128(mask_lo_cnt, r4);
        r4 = _mm_srli_epi16(r4, 6);
        r4 = _mm_packs_epi32(r4, r5);

        // accumulate
        counterA = _mm_add_epi32(counterA, r4);

        // do it again...
        r4 = _mm_and_si128(mask_bits_b, r0);
        r5 = _mm_and_si128(mask_bits_b, r1);
        r6 = _mm_and_si128(mask_bits_b, r2);
        r7 = _mm_and_si128(mask_bits_b, r3);

        r4 = _mm_sad_epu8(r4, zero);
        r5 = _mm_sad_epu8(r5, zero);
        r6 = _mm_sad_epu8(r6, zero);
        r7 = _mm_sad_epu8(r7, zero);

        r4 = _mm_add_epi16(r4,r5);
        r4 = _mm_add_epi16(r4,r6);
        r4 = _mm_add_epi16(r4,r7);

        r5 = _mm_avg_epu8(zero, r4); // shift right 1
        r5 = _mm_and_si128(r5, mask_lo_cnt);
        r4 = _mm_srli_epi16(r4, 7);
        r4 = _mm_packs_epi32(r4, r5);

        counterB = _mm_add_epi32(counterB, r4); // accumulate

        // rotate right 4
        r4 = _mm_slli_epi16(r0, 12);
        r5 = _mm_slli_epi16(r1, 12);
        r6 = _mm_slli_epi16(r2, 12);
        r7 = _mm_slli_epi16(r3, 12);
        r0 = _mm_srli_epi16(r0, 4);
        r1 = _mm_srli_epi16(r1, 4);
        r2 = _mm_srli_epi16(r2, 4);
        r3 = _mm_srli_epi16(r3, 4);
        r0 = _mm_or_si128(r0, r4);
        r1 = _mm_or_si128(r1, r5);
        r2 = _mm_or_si128(r2, r6);
        r3 = _mm_or_si128(r3, r7);

        // do it again...
        r4 = _mm_and_si128(mask_bits_a, r0);
        r5 = _mm_and_si128(mask_bits_a, r1);
        r6 = _mm_and_si128(mask_bits_a, r2);
        r7 = _mm_and_si128(mask_bits_a, r3);

        r4 = _mm_sad_epu8(r4, zero);
        r5 = _mm_sad_epu8(r5, zero);
        r6 = _mm_sad_epu8(r6, zero);
        r7 = _mm_sad_epu8(r7, zero);

        r4 = _mm_add_epi16(r4,r5);
        r4 = _mm_add_epi16(r4,r6);
        r4 = _mm_add_epi16(r4,r7);

        r5 = _mm_and_si128(mask_lo_cnt, r4);
        r4 = _mm_srli_epi16(r4, 6);
        r4 = _mm_packs_epi32(r4, r5);

        counterC = _mm_add_epi32(counterC, r4); // accumulate

        // do it again...
        r0 = _mm_and_si128(r0, mask_bits_b);
        r1 = _mm_and_si128(r1, mask_bits_b);
        r2 = _mm_and_si128(r2, mask_bits_b);
        r3 = _mm_and_si128(r3, mask_bits_b);

        r0 = _mm_sad_epu8(r0, zero);
        r1 = _mm_sad_epu8(r1, zero);
        r2 = _mm_sad_epu8(r2, zero);
        r3 = _mm_sad_epu8(r3, zero);

        r0 = _mm_add_epi16(r0,r1);
        r0 = _mm_add_epi16(r0,r2);
        r0 = _mm_add_epi16(r0,r3);

        r1 = _mm_avg_epu8(zero, r0);
        r1 = _mm_and_si128(r1, mask_lo_cnt);
        r0 = _mm_srli_epi16(r0, 7);
        r0 = _mm_packs_epi32(r0, r1);

        counterD = _mm_add_epi32(counterD, r0); // accumulate
    }

    // transpose then store counters
    __m128i counter_1098 = _mm_unpackhi_epi32(counterA, counterB);
    __m128i counter_76FE = _mm_unpacklo_epi32(counterA, counterB);
    __m128i counter_32BA = _mm_unpacklo_epi32(counterC, counterD);
    __m128i counter_54DC = _mm_unpackhi_epi32(counterC, counterD);
    __m128i counter_7654 = _mm_unpackhi_epi64(counter_54DC, counter_76FE);
    __m128i counter_FEDC = _mm_unpacklo_epi64(counter_54DC, counter_76FE);
    __m128i counter_3210 = _mm_unpackhi_epi64(counter_1098, counter_32BA);
    __m128i counter_BA98 = _mm_unpacklo_epi64(counter_1098, counter_32BA);

    
    _mm_storeu_si128((__m128i*)&buffer[0], counter_3210);
    _mm_storeu_si128((__m128i*)&buffer[4], counter_7654);
    _mm_storeu_si128((__m128i*)&buffer[8], counter_BA98);
    _mm_storeu_si128((__m128i*)&buffer[12], counter_FEDC);
    for (int i = 0; i < 16; ++i) flag_counts[i] += buffer[i];

    // scalar tail loop
    int tail = len & 31;
    if (tail != 0) {
        uint64_t countsA = 0;
        uint64_t countsB = 0;
        do {
            // zero-extend a bit to 8-bits then accumulate
            // (emulate pdep)
            const uint64_t mask_01 = UINT64_C(0x0101010101010101);// 100000001000000010000000100000001000000010000000100000001
            const uint64_t magic   = UINT64_C(0x0000040010004001);// 000000000000001000000000000010000000000000100000000000001
                                                                  // 1+(1<<14)+(1<<28)+(1<<42)
            uint64_t x = *data++;
            countsA += ((x & 0x5555) * magic) & mask_01; // 0101010101010101
            countsB += (((x >> 1) & 0x5555) * magic) & mask_01;
        } while (--tail);

        // transpose then store counters
        flag_counts[0]  += countsA & 0xFF;
        flag_counts[8]  += (countsA >>  8) & 0xFF;
        flag_counts[2]  += (countsA >> 16) & 0xFF;
        flag_counts[10] += (countsA >> 24) & 0xFF;
        flag_counts[4]  += (countsA >> 32) & 0xFF;
        flag_counts[12] += (countsA >> 40) & 0xFF;
        flag_counts[6]  += (countsA >> 48) & 0xFF;
        flag_counts[14] += (countsA >> 56) & 0xFF;
        flag_counts[1]  += countsB & 0xFF;
        flag_counts[9]  += (countsB >>  8) & 0xFF;
        flag_counts[3]  += (countsB >> 16) & 0xFF;
        flag_counts[11] += (countsB >> 24) & 0xFF;
        flag_counts[5]  += (countsB >> 32) & 0xFF;
        flag_counts[13] += (countsB >> 40) & 0xFF;
        flag_counts[7]  += (countsB >> 48) & 0xFF;
        flag_counts[15] += (countsB >> 56) & 0xFF;
    }

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_single, pospopcnt_u16_sse_single)
make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_sad, pospopcnt_u16_sse_sad)
#else
pospopcnt_u16_stub(pospopcnt_u16_sse_single)
pospopcnt_u16_stub(pospopcnt_u16_sse_sad)
pospopcnt_u8_stub(pospopcnt_u8_sse_single)
pospopcnt_u8_stub(pospopcnt_u8_sse_sad)
#endif

#if !defined(__clang__) && !defined(_MSC_VER)
__attribute__((optimize("no-tree-vectorize")))
#endif
int pospopcnt_u16_scalar_naive_nosimd(const uint16_t* data, uint32_t len, uint32_t* flags) {
    for (uint32_t i = 0; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

    return 0;
}

int pospopcnt_u16_scalar_naive(const uint16_t* data, uint32_t len, uint32_t* flags) {
    for (uint32_t i = 0; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

    return 0;
}

int pospopcnt_u16_scalar_partition(const uint16_t* data, uint32_t len, uint32_t* flags) {
    uint32_t low[256] = {0}, high[256] = {0};

    for (uint32_t i = 0; i < len; ++i) {
        ++low[data[i] & 255];
        ++high[(data[i] >> 8) & 255];
    }

    for (int i = 0; i < 256; ++i) {
        for (int k = 0; k < 8; ++k) {
            flags[k] += ((i & (1 << k)) >> k) * low[i];
        }
    }

    for (int i = 0; i < 256; ++i) {
        for (int k = 0; k < 8; ++k) {
            flags[k+8] += ((i & (1 << k)) >> k) * high[i];
        }
    }

    return 0;
}

int pospopcnt_u16_scalar_hist1x4(const uint16_t* data, uint32_t len, uint32_t* flags) {
     uint32_t low[256] = {0}, high[256] = {0};

     uint32_t i = 0;
     for (i = 0; i < (len & ~3); i+=4) {
          ++low[data[i+0] & 255];
          ++high[(data[i+0] >> 8) & 255];
          ++low[data[i+1] & 255];
          ++high[(data[i+1] >> 8) & 255];
          ++low[data[i+2] & 255];
          ++high[(data[i+2] >> 8) & 255];
          ++low[data[i+3] & 255];
          ++high[(data[i+3] >> 8) & 255];
     }
     while (i < len) {
          ++low[data[i] & 255];
          ++high[(data[i++] >> 8) & 255];
     }

     for (int i = 0; i < 256; ++i) {
        for (int k = 0; k < 8; ++k) {
            flags[k] += ((i & (1 << k)) >> k) * low[i];
        }
    }

    for (int i = 0; i < 256; ++i) {
        for (int k = 0; k < 8; ++k) {
            flags[k+8] += ((i & (1 << k)) >> k) * high[i];
        }
    }

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_scalar_partition, pospopcnt_u16_scalar_partition)
make_pospopcnt_u8_from_u16(pospopcnt_u8_scalar_naive_nosimd, pospopcnt_u16_scalar_naive_nosimd)
make_pospopcnt_u8_from_u16(pospopcnt_u8_scalar_hist1x4, pospopcnt_u16_scalar_hist1x4)

#ifndef _MSC_VER
// By @aqrit (https://github.com/aqrit)
// @see: https://gist.github.com/aqrit/c729815b0165c139d0bac642ab7ee104
int pospopcnt_u16_scalar_umul128(const uint16_t* in, uint32_t n, uint32_t* out) {
    const uint64_t mask_bits = UINT64_C(0x1249124912491249); // 00000000 00000100 01110000 00010010 10001110 10110000 01101010 11110001
    const uint64_t mask_cnts = UINT64_C(0x003801C00E007007); // 00000000 00111000 00000001 11000000 00001110 00000000 01110000 00001000
    const uint64_t mask_0001 = UINT64_C(0x0001000100010001); // 00000000 00000001 00000000 00000001 00000000 00000001 00000000 00000001

    while (n >= 4) {
        uint64_t counter_a = 0; // 5 packed 12-bit counters (4 unused bits).
        uint64_t counter_b = 0;
        uint64_t counter_c = 0;
        uint64_t counter_d = 0;

        // Terminate before overflowing the counters.
        uint32_t len = ((n < 0x0FFF) ? n : 0x0FFF) & ~3;
        n -= len;
        for (const uint16_t* end = &in[len]; in != end; in += 4) {
            uint64_t v = pospopcnt_loadu_u64(in);
            uint64_t a = v & mask_bits;         // 0b0001001001001001
            uint64_t b = (v >> 1)  & mask_bits; // 0b0010010010010010 >> 1
            uint64_t c = (v >> 2)  & mask_bits; // 0b0100100100100100 >> 2
            uint64_t d = (v >> 15) & mask_0001; // 0b1000000000000000 >> 15
            uint64_t hi_a, hi_b, hi_c;
            a = pospopcnt_umul128(a, mask_0001, &hi_a);
            b = pospopcnt_umul128(b, mask_0001, &hi_b);
            c = pospopcnt_umul128(c, mask_0001, &hi_c);
            a += hi_a; // Broadcast 3-bit counts.
            b += hi_b;
            c += hi_c;
            counter_a += a & mask_cnts;
            counter_b += b & mask_cnts;
            counter_c += c & mask_cnts;
            counter_d += d;
        }

        out[0]  += counter_a & 0x0FFF;
        out[1]  += counter_b & 0x0FFF;
        out[2]  += counter_c & 0x0FFF;
        out[3]  += (counter_a >> 51);
        out[4]  += (counter_b >> 51);
        out[5]  += (counter_c >> 51);
        out[6]  += (counter_a >> 38) & 0x0FFF;
        out[7]  += (counter_b >> 38) & 0x0FFF;
        out[8]  += (counter_c >> 38) & 0x0FFF;
        out[9]  += (counter_a >> 25) & 0x0FFF;
        out[10] += (counter_b >> 25) & 0x0FFF;
        out[11] += (counter_c >> 25) & 0x0FFF;
        out[12] += (counter_a >> 12) & 0x0FFF;
        out[13] += (counter_b >> 12) & 0x0FFF;
        out[14] += (counter_c >> 12) & 0x0FFF;
        out[15] += (counter_d * mask_0001) >> 48;
    }

    // Residual words.
    for (const uint16_t* tail_end = &in[n]; in != tail_end; ++in) {
        uint16_t x = *in;
        for (int i = 0; i != 16; ++i) {
            out[i] += x & 1;
            x >>= 1;
        }
    }

    return 0;
}

int pospopcnt_u16_scalar_umul128_unroll2(const uint16_t* in, uint32_t n, uint32_t* out) {
    while (n >= 8) {
        uint64_t counter_a = 0; // 4 packed 12-bit counters
        uint64_t counter_b = 0;
        uint64_t counter_c = 0;
        uint64_t counter_d = 0;

        // end before overflowing the counters
        uint32_t len = ((n < 0x0FFF) ? n : 0x0FFF) & ~7;
        n -= len;
        for (const uint16_t* end = &in[len]; in != end; in += 8) {
            const uint64_t mask_a = UINT64_C(0x1111111111111111);
            const uint64_t mask_b = mask_a + mask_a;
            const uint64_t mask_c = mask_b + mask_b;
            const uint64_t mask_0001 = UINT64_C(0x0001000100010001);
            const uint64_t mask_cnts = UINT64_C(0x000000F00F00F00F);

            uint64_t v0 = pospopcnt_loadu_u64(&in[0]);
            uint64_t v1 = pospopcnt_loadu_u64(&in[4]);

            uint64_t a = (v0 & mask_a) + (v1 & mask_a);
            uint64_t b = ((v0 & mask_b) + (v1 & mask_b)) >> 1;
            uint64_t c = ((v0 & mask_c) + (v1 & mask_c)) >> 2;
            uint64_t d = ((v0 >> 3) & mask_a) + ((v1 >> 3) & mask_a);

            uint64_t hi;
            a = pospopcnt_umul128(a, mask_0001, &hi);
            a += hi; // broadcast 4-bit counts
            b = pospopcnt_umul128(b, mask_0001, &hi);
            b += hi;
            c = pospopcnt_umul128(c, mask_0001, &hi);
            c += hi;
            d = pospopcnt_umul128(d, mask_0001, &hi);
            d += hi;

            counter_a += a & mask_cnts;
            counter_b += b & mask_cnts;
            counter_c += c & mask_cnts;
            counter_d += d & mask_cnts;
        }

        out[0] += counter_a & 0x0FFF;
        out[1] += counter_b & 0x0FFF;
        out[2] += counter_c & 0x0FFF;
        out[3] += counter_d & 0x0FFF;
        out[4] += (counter_a >> 36);
        out[5] += (counter_b >> 36);
        out[6] += (counter_c >> 36);
        out[7] += (counter_d >> 36);
        out[8] += (counter_a >> 24) & 0x0FFF;
        out[9] += (counter_b >> 24) & 0x0FFF;
        out[10] += (counter_c >> 24) & 0x0FFF;
        out[11] += (counter_d >> 24) & 0x0FFF;
        out[12] += (counter_a >> 12) & 0x0FFF;
        out[13] += (counter_b >> 12) & 0x0FFF;
        out[14] += (counter_c >> 12) & 0x0FFF;
        out[15] += (counter_d >> 12) & 0x0FFF;
    }

    // assert(n < 8)
    if (n != 0) {
        uint64_t tail_counter_a = 0;
        uint64_t tail_counter_b = 0;
        do { // zero-extend a bit to 8-bits (emulate pdep) then accumulate
            const uint64_t mask_01 = UINT64_C(0x0101010101010101);
            const uint64_t magic   = UINT64_C(0x0000040010004001); // 1+(1<<14)+(1<<28)+(1<<42)
            uint64_t x = *in++;
            tail_counter_a += ((x & 0x5555) * magic) & mask_01; // 0101010101010101
            tail_counter_b += (((x >> 1) & 0x5555) * magic) & mask_01;
        } while (--n);

        out[0]  += tail_counter_a & 0xFF;
        out[8]  += (tail_counter_a >>  8) & 0xFF;
        out[2]  += (tail_counter_a >> 16) & 0xFF;
        out[10] += (tail_counter_a >> 24) & 0xFF;
        out[4]  += (tail_counter_a >> 32) & 0xFF;
        out[12] += (tail_counter_a >> 40) & 0xFF;
        out[6]  += (tail_counter_a >> 48) & 0xFF;
        out[14] += (tail_counter_a >> 56) & 0xFF;
        out[1]  += tail_counter_b & 0xFF;
        out[9]  += (tail_counter_b >>  8) & 0xFF;
        out[3]  += (tail_counter_b >> 16) & 0xFF;
        out[11] += (tail_counter_b >> 24) & 0xFF;
        out[5]  += (tail_counter_b >> 32) & 0xFF;
        out[13] += (tail_counter_b >> 40) & 0xFF;
        out[7]  += (tail_counter_b >> 48) & 0xFF;
        out[15] += (tail_counter_b >> 56) & 0xFF;
    }

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_scalar_umul128, pospopcnt_u16_scalar_umul128)
make_pospopcnt_u8_from_u16(pospopcnt_u8_scalar_umul128_unroll2, pospopcnt_u16_scalar_umul128_unroll2)
#else 
pospopcnt_u16_stub(pospopcnt_u16_scalar_umul128)
pospopcnt_u16_stub(pospopcnt_u16_scalar_umul128_unroll2)
pospopcnt_u8_stub(pospopcnt_u8_scalar_umul128)
pospopcnt_u8_stub(pospopcnt_u8_scalar_umul128_unroll2)
#endif

#if POSPOPCNT_SIMD_VERSION >= 6
#if defined(__AVX512BW__) && __AVX512BW__ == 1
int pospopcnt_u16_avx512bw_popcnt32_mask(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m512i masks[16];
    for (int i = 0; i < 16; ++i) {
        masks[i] = _mm512_set1_epi32(((1 << i) << 16) | (1 << i));
    }
    uint32_t out_counters[16] = {0};

    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

#define UPDATE(pos) out_counters[pos] += PIL_POPCOUNT((uint64_t)_mm512_cmpeq_epu16_mask(_mm512_and_epi32(_mm512_loadu_si512(data_vectors+i), masks[pos]), masks[pos]));
#define BLOCK {                                 \
    UPDATE(0)  UPDATE(1)  UPDATE(2)  UPDATE(3)  \
    UPDATE(4)  UPDATE(5)  UPDATE(6)  UPDATE(7)  \
    UPDATE(8)  UPDATE(9)  UPDATE(10) UPDATE(11) \
    UPDATE(12) UPDATE(13) UPDATE(14) UPDATE(15) \
}

    for (size_t i = 0; i < n_cycles; ++i) {
        BLOCK
    }

    // residual
    for (size_t i = n_cycles*32; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

#undef BLOCK
#undef UPDATE

    return 0;
}

int pospopcnt_u16_avx512bw_popcnt64_mask(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m512i masks[16];
    for (int i = 0; i < 16; ++i) {
        masks[i] = _mm512_set1_epi32(((1 << i) << 16) | (1 << i));
    }
    uint32_t out_counters[16] = {0};

    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

#define UPDATE(pos,add) (uint64_t)_mm512_cmpeq_epu16_mask(_mm512_and_epi32(_mm512_loadu_si512(data_vectors+i+add), masks[pos]), masks[pos])
#define UP(pos) out_counters[pos] += PIL_POPCOUNT((UPDATE(pos,0) << 32) | UPDATE(pos,1));
#define BLOCK {                 \
    UP(0)  UP(1)  UP(2)  UP(3)  \
    UP(4)  UP(5)  UP(6)  UP(7)  \
    UP(8)  UP(9)  UP(10) UP(11) \
    UP(12) UP(13) UP(14) UP(15) \
}
    
    size_t i = 0;
    for (/**/; i + 2 < n_cycles; i += 2) {
        BLOCK
    }

    // residual
    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

#undef BLOCK
#undef UP
#undef UPDATE

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_popcnt32_mask, pospopcnt_u16_avx512bw_popcnt32_mask)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_popcnt64_mask, pospopcnt_u16_avx512bw_popcnt64_mask)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_popcnt32_mask)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_popcnt64_mask)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt32_mask)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt64_mask)
#endif

int pospopcnt_u16_avx512_popcnt(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m512i masks[16];
    __m512i counters[16];
    __m512i stubs[16];
    for (int i = 0; i < 16; ++i) {
        masks[i]    = _mm512_set1_epi32(((1 << i) << 16) | (1 << i));
        counters[i] = _mm512_set1_epi32(0);
        stubs[i] = _mm512_set1_epi32(0);
    }
    uint32_t out_counters[16] = {0};

    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

#define UPDATE(pos,shift) stubs[pos] = _mm512_or_si512(stubs[pos], _mm512_slli_epi16(_mm512_srli_epi16(_mm512_and_si512(_mm512_loadu_si512(data_vectors+i+j), masks[pos]), pos), shift));
#define BLOCK(shift) {                                                  \
    UPDATE(0,shift)  UPDATE(1,shift)  UPDATE(2,shift)  UPDATE(3,shift)  \
    UPDATE(4,shift)  UPDATE(5,shift)  UPDATE(6,shift)  UPDATE(7,shift)  \
    UPDATE(8,shift)  UPDATE(9,shift)  UPDATE(10,shift) UPDATE(11,shift) \
    UPDATE(12,shift) UPDATE(13,shift) UPDATE(14,shift) UPDATE(15,shift) \
}
#define UC(pos) {                         \
    counters[pos] = _mm512_add_epi32(counters[pos], avx512_popcount(stubs[pos])); \
    stubs[pos]    = _mm512_set1_epi32(0); \
}
#define UC_BLOCK {              \
    UC(0)  UC(1)  UC(2)  UC(3)  \
    UC(4)  UC(5)  UC(6)  UC(7)  \
    UC(8)  UC(9)  UC(10) UC(11) \
    UC(12) UC(13) UC(14) UC(15) \
}

    size_t i = 0;
    for (/**/; i + 16 < n_cycles; i += 16) {
        for (int j = 0; j < 16; ++j) {
            BLOCK(j)
        }

        UC_BLOCK
    }

#undef UC_BLOCK
#undef UC
#undef BLOCK
#undef UPDATE

    // residual
    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) {
        uint32_t* v = (uint32_t*)(&counters[i]);
        for (int j = 0; j < 16; ++j)
            out_counters[i] += v[j];
    }
    
    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

    return 0;
}

int pospopcnt_u16_avx512(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m512i masks[16];
    __m512i counters[16];
    const __m512i one_mask = _mm512_set1_epi32(1);
    for (int i = 0; i < 16; ++i) {
        masks[i]    = _mm512_set1_epi16(1 << i);
        counters[i] = _mm512_set1_epi32(0);
    }
    uint32_t out_counters[16] = {0};

    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

#define UPDATE(pos) {                                            \
    __m512i a   = _mm512_and_epi32(_mm512_loadu_si512(data_vectors+i), masks[pos]); \
    __m512i d   = _mm512_add_epi32(_mm512_and_epi32(_mm512_srli_epi32(a, pos), one_mask), _mm512_srli_epi32(a, pos+16)); \
    counters[pos] = _mm512_add_epi32(counters[pos], d);          \
}
#define BLOCK {                                 \
    UPDATE(0)  UPDATE(1)  UPDATE(2)  UPDATE(3)  \
    UPDATE(4)  UPDATE(5)  UPDATE(6)  UPDATE(7)  \
    UPDATE(8)  UPDATE(9)  UPDATE(10) UPDATE(11) \
    UPDATE(12) UPDATE(13) UPDATE(14) UPDATE(15) \
}

    for (uint32_t i = 0; i < n_cycles; ++i) {
        BLOCK
    }

#undef BLOCK
#undef UPDATE

    // residual
    for (uint32_t i = n_cycles*32; i < len; ++i) {
        for (int j = 0; j < 16; ++j)
            out_counters[j] += ((data[i] & (1 << j)) >> j);
    }

    for (int i = 0; i < 16; ++i) {
        uint32_t* v = (uint32_t*)(&counters[i]);
        for (int j = 0; j < 16; ++j)
            out_counters[i] += v[j];
    }
 
    for (int i = 0; i < 16; ++i) flags[i] = out_counters[i];

    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512_popcnt, pospopcnt_u16_avx512_popcnt)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512, pospopcnt_u16_avx512)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_avx512)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_popcnt32_mask)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_popcnt64_mask)
pospopcnt_u8_stub(pospopcnt_u8_avx512_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_avx512)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt32_mask)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt64_mask)
#endif

#if POSPOPCNT_SIMD_VERSION >= 5

// by D. Lemire
int pospopcnt_u16_avx2_lemire(const uint16_t* array, uint32_t len, uint32_t* flags) {
    uint16_t buffer[16];
    __m256i bits = _mm256_set_epi16(-32768, 16384, 8192, 4096, 2048, 1024, 512, 256,
                                       128,    64,   32,   16,    8,    4,   2,   1);
    // we do the first part
    if (len < 64) {
        // don't bother with handcrafted SIMD
        pospopcnt_u16_scalar_naive(array, len, flags);
        return 0;
    }
    // handle the start (naively)
    __m256i count16 = _mm256_setzero_si256();

    {
        uint16_t startbuffer[32];
        memset(startbuffer, 0, 32 * 2);
        memcpy(startbuffer + 16, array, 16 * 2);
        for (size_t i = 1; i < 16; ++i) {
            __m256i input = _mm256_loadu_si256((__m256i*)(startbuffer + i));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
    }

    {
        uint16_t startbuffer[32];
        memset(startbuffer, 0, 32 * 2);
        memcpy(startbuffer, array + len - 16, 16 * 2);
        for (size_t i = 1; i < 16; ++i) {
            __m256i input = _mm256_loadu_si256((__m256i*)(startbuffer + i));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
    }
    
    _mm256_storeu_si256((__m256i*)buffer, count16);
    for (size_t k = 0; k < 16; k++) {
        flags[k] += buffer[k];
    }

    // main loop starts here
    for (size_t i = 0; i + 16 <= len;) {
        count16 = _mm256_setzero_si256();
        size_t j = 0;
        size_t maxj = 65535;

        if (maxj + i + 16 >= len)
            maxj = len - i - 15;

        for (/**/; j < maxj; ++j) {
            __m256i input = _mm256_loadu_si256((__m256i*)(array + i + j));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
        i += j;
        _mm256_storeu_si256((__m256i*)buffer, count16);
        for (size_t k = 0; k < 16; k++) {
            flags[k] += buffer[k];
        }
    }
    return 0;
}

// By D. Lemire
int pospopcnt_u16_avx2_lemire2(const uint16_t* array, uint32_t len, uint32_t* flags) {
    uint16_t buffer[16];
    __m256i bits = _mm256_set_epi16(-32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 
                                       128,    64,   32,   16,    8,    4,   2,   1);
    // we do the first part
    if (len < 64) {
        // don't bother with handcrafted SIMD
        pospopcnt_u16_scalar_naive(array, len, flags);
        return 0;
    }
    // handle the start (naively)
    __m256i count16 = _mm256_setzero_si256();

    {
        uint16_t startbuffer[32];
        memset(startbuffer, 0, 32 * 2);
        memcpy(startbuffer + 16, array, 16 * 2);
        for (size_t i = 1; i < 16; ++i) {
            __m256i input = _mm256_loadu_si256((__m256i*)(startbuffer + i));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
    }

    {
        uint16_t startbuffer[32];
        memset(startbuffer, 0, 32 * 2);
        memcpy(startbuffer, array + len - 16, 16 * 2);
        for (size_t i = 1; i < 16; ++i) {
            __m256i input = _mm256_loadu_si256((__m256i*)(startbuffer + i));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, count16);
    for (size_t k = 0; k < 16; k++) {
        flags[k] += buffer[k];
    }

    // main loop starts here
    for (size_t i = 0; i + 16 <= len;) {
        count16 = _mm256_setzero_si256();
        size_t j = 0;
        size_t maxj = 65535;

        if (maxj + i + 16 >= len)
            maxj = len - i - 15;

        if (maxj > 8) {
            for (/**/; j < maxj - 7; j += 8) {
                __m256i input1 = _mm256_loadu_si256((__m256i*)(array + i + j));
                __m256i m1  = _mm256_and_si256(input1, bits);
                __m256i eq1 = _mm256_cmpeq_epi16(bits, m1);
                count16 = _mm256_sub_epi16(count16, eq1);
                __m256i input2 = _mm256_loadu_si256((__m256i*)(array + i + j + 1));
                __m256i m2  = _mm256_and_si256(input2, bits);
                __m256i eq2 = _mm256_cmpeq_epi16(bits, m2);
                count16 = _mm256_sub_epi16(count16, eq2);
                __m256i input3 = _mm256_loadu_si256((__m256i*)(array + i + j + 2));
                __m256i m3  = _mm256_and_si256(input3, bits);
                __m256i eq3 = _mm256_cmpeq_epi16(bits, m3);
                count16 = _mm256_sub_epi16(count16, eq3);
                __m256i input4 = _mm256_loadu_si256((__m256i*)(array + i + j + 3));
                __m256i m4  = _mm256_and_si256(input4, bits);
                __m256i eq4 = _mm256_cmpeq_epi16(bits, m4);
                count16 = _mm256_sub_epi16(count16, eq4);
                __m256i input5 = _mm256_loadu_si256((__m256i*)(array + i + j + 4));
                __m256i m5  = _mm256_and_si256(input5, bits);
                __m256i eq5 = _mm256_cmpeq_epi16(bits, m5);
                count16 = _mm256_sub_epi16(count16, eq5);
                __m256i input6 = _mm256_loadu_si256((__m256i*)(array + i + j + 5));
                __m256i m6  = _mm256_and_si256(input6, bits);
                __m256i eq6 = _mm256_cmpeq_epi16(bits, m6);
                count16 = _mm256_sub_epi16(count16, eq6);
                __m256i input7 = _mm256_loadu_si256((__m256i*)(array + i + j + 6));
                __m256i m7  = _mm256_and_si256(input7, bits);
                __m256i eq7 = _mm256_cmpeq_epi16(bits, m7);
                count16 = _mm256_sub_epi16(count16, eq7);
                __m256i input8 = _mm256_loadu_si256((__m256i*)(array + i + j + 7));
                __m256i m8  = _mm256_and_si256(input8, bits);
                __m256i eq8 = _mm256_cmpeq_epi16(bits, m8);
                count16 = _mm256_sub_epi16(count16, eq8);
            }
        }

        if (maxj > 4) {
            for (/**/; j + 3 < maxj; j += 4) {
                __m256i input1 = _mm256_loadu_si256((__m256i*)(array + i + j));
                __m256i m1  = _mm256_and_si256(input1, bits);
                __m256i eq1 = _mm256_cmpeq_epi16(bits, m1);
                count16 = _mm256_sub_epi16(count16, eq1);
                __m256i input2 = _mm256_loadu_si256((__m256i*)(array + i + j + 1));
                __m256i m2  = _mm256_and_si256(input2, bits);
                __m256i eq2 = _mm256_cmpeq_epi16(bits, m2);
                count16 = _mm256_sub_epi16(count16, eq2);
                __m256i input3 = _mm256_loadu_si256((__m256i*)(array + i + j + 2));
                __m256i m3  = _mm256_and_si256(input3, bits);
                __m256i eq3 = _mm256_cmpeq_epi16(bits, m3);
                count16 = _mm256_sub_epi16(count16, eq3);
                __m256i input4 = _mm256_loadu_si256((__m256i*)(array + i + j + 3));
                __m256i m4  = _mm256_and_si256(input4, bits);
                __m256i eq4 = _mm256_cmpeq_epi16(bits, m4);
                count16 = _mm256_sub_epi16(count16, eq4);
            }
        }
        for (/**/; j < maxj; ++j) {
            __m256i input = _mm256_loadu_si256((__m256i*)(array + i + j));
            __m256i m  = _mm256_and_si256(input, bits);
            __m256i eq = _mm256_cmpeq_epi16(bits, m);
            count16 = _mm256_sub_epi16(count16, eq);
        }
        i += j;

        _mm256_storeu_si256((__m256i*)buffer, count16);
        for (size_t k = 0; k < 16; k++) {
            flags[k] += buffer[k];
        }
    }
    return 0;
}

int pospopcnt_u16_avx2_mula2(const uint16_t* array, uint32_t len, uint32_t* flags) {
    __m256i counters[16];

    for (size_t i = 0; i < 16; ++i) {
        counters[i] = _mm256_setzero_si256();
    }

    size_t i = 0;
    for (/**/; i + 16 <= len; i += 16) {
        __m256i input = _mm256_loadu_si256((__m256i*)(array + i));

        for (int j = 0; j < 16; ++j) {
            __m256i bit = _mm256_and_si256(input, _mm256_set1_epi16(1));
            counters[j] = _mm256_add_epi16(counters[j], bit);
            input = _mm256_srli_epi16(input, 1);
        }
    }

     // residual
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    uint16_t tmp[16];
    for (size_t i = 0; i < 16; ++i) {
        _mm256_storeu_si256((__m256i*)tmp, counters[i]);
        for (int j=0; j < 16; ++j)
            flags[i] += tmp[j];
    }
    return 0;
}

// By Daniel Lemire
int pospopcnt_u16_avx2_blend_popcnt(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m256i* data_vectors = (const __m256i*)(array);
    const uint32_t n_cycles = len / 16;

    size_t i = 0;
    for (/**/; i + 2 <= n_cycles; i += 2) {
        __m256i v0 = _mm256_loadu_si256(data_vectors + i + 0);
        __m256i v1 = _mm256_loadu_si256(data_vectors + i + 1);
        __m256i input0 = _mm256_or_si256(_mm256_and_si256(v0, _mm256_set1_epi16(0x00FF)), _mm256_slli_epi16(v1, 8));
        __m256i input1 = _mm256_or_si256(_mm256_and_si256(v0, _mm256_set1_epi16((int16_t)0xFF00)), _mm256_srli_epi16(v1, 8));
        
        for (int i = 0; i < 8; ++i) {
            flags[ 7 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input0));
            flags[15 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input1));
            input0 = _mm256_add_epi8(input0, input0);
            input1 = _mm256_add_epi8(input1, input1);
        }
    }

    i *= 16;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }
   
    return 0;
}

int pospopcnt_u16_avx2_adder_forest(const uint16_t* array, uint32_t len, uint32_t* flags) {
    __m256i counters[16];

    for (size_t i = 0; i < 16; ++i) {
        counters[i] = _mm256_setzero_si256();
    }

    const __m256i mask1bit = _mm256_set1_epi16(0x5555); // 0101010101010101
    const __m256i mask2bit = _mm256_set1_epi16(0x3333); // 0011001100110011
    const __m256i mask4bit = _mm256_set1_epi16(0x0F0F); // 1111000011110000
    const __m256i mask8bit = _mm256_set1_epi16(0x00FF); // 0000000011111111
    
    const uint32_t n_cycles = len / (4096 * (16*16));
    const uint32_t n_total  = len / (16*16);
    uint16_t tmp[16];

/*------ Macros --------*/
#define LL(i,p,k)  const __m256i sum##p##k##_##i##bit_even = _mm256_add_epi8(input##p & mask##i##bit, input##k & mask##i##bit);
#define LO(i,p,k)  const __m256i sum##p##k##_##i##bit_odd  = _mm256_add_epi8(_mm256_srli_epi16(input##p, i) & mask##i##bit, _mm256_srli_epi16(input##k, i) & mask##i##bit);

#define LBLOCK(i)                                 \
    LL(i,0,1) LL(i,2,3)   LL(i,4,5)   LL(i,6,7)   \
    LL(i,8,9) LL(i,10,11) LL(i,12,13) LL(i,14,15) \
    LO(i,0,1) LO(i,2,3)   LO(i,4,5)   LO(i,6,7)   \
    LO(i,8,9) LO(i,10,11) LO(i,12,13) LO(i,14,15)

#define EVEN(b,i,k,p) input##i = sum##k##p##_##b##bit_even;
#define ODD(b,i,k,p)  input##i = sum##k##p##_##b##bit_odd;

#define UPDATE(i)                                                  \
    EVEN(i,0,0,1) EVEN(i,1,2,3)   EVEN(i,2,4,5)   EVEN(i,3,6,7)    \
    EVEN(i,4,8,9) EVEN(i,5,10,11) EVEN(i,6,12,13) EVEN(i,7,14,15)  \
     ODD(i,8,0,1)  ODD(i,9,2,3)    ODD(i,10,4,5)   ODD(i,11,6,7)   \
     ODD(i,12,8,9) ODD(i,13,10,11) ODD(i,14,12,13) ODD(i,15,14,15) \

#define UE(i,p,k) counters[i] = _mm256_add_epi16(counters[i], sum##p##k##_8bit_even);
#define UO(i,p,k) counters[i] = _mm256_add_epi16(counters[i], sum##p##k##_8bit_odd);

/*------ Start --------*/
#define L(p) __m256i input##p = _mm256_loadu_si256((__m256i*)(array + i*4096*256 + j*256 + p*16));
    size_t i = 0;
    for (/**/; i < n_cycles; ++i) {
        for (int j = 0; j < 4096; ++j) {
            // Load 16 registers.
            L(0)  L(1)  L(2)  L(3)  
            L(4)  L(5)  L(6)  L(7) 
            L(8)  L(9)  L(10) L(11) 
            L(12) L(13) L(14) L(15)

            // Perform updates for bits {1,2,4,8}.
            LBLOCK(1) UPDATE(1)
            LBLOCK(2) UPDATE(2)
            LBLOCK(4) UPDATE(4)
            LBLOCK(8) UPDATE(8)

            // Update accumulators.
            UE( 0,0,1) UE( 1, 2, 3) UE( 2, 4, 5) UE( 3, 6, 7)  
            UE( 4,8,9) UE( 5,10,11) UE( 6,12,13) UE( 7,14,15) 
            UO( 8,0,1) UO( 9, 2, 3) UO(10, 4, 5) UO(11, 6, 7) 
            UO(12,8,9) UO(13,10,11) UO(14,12,13) UO(15,14,15)
        }

        // Update.
        for (size_t i = 0; i < 16; ++i) {
            _mm256_storeu_si256((__m256i*)tmp, counters[i]);
            for (int j = 0; j < 16; ++j)
                flags[i] += tmp[j];
        }
        // Reset.
        for (size_t i = 0; i < 16; ++i) {
            counters[i] = _mm256_setzero_si256();
        }
    }
#undef L
#define L(p) __m256i input##p = _mm256_loadu_si256((__m256i*)(array + i*256 + p*16));
    i *= 4096;
    for (/**/; i < n_total; ++i) {
        // Load 16 registers.
        L(0)  L(1)  L(2)  L(3)  
        L(4)  L(5)  L(6)  L(7) 
        L(8)  L(9)  L(10) L(11) 
        L(12) L(13) L(14) L(15)

        // Perform updates for bits {1,2,4,8}.
        LBLOCK(1) UPDATE(1)
        LBLOCK(2) UPDATE(2)
        LBLOCK(4) UPDATE(4)
        LBLOCK(8) UPDATE(8)

        // Update accumulators.
        UE( 0,0,1) UE( 1, 2, 3) UE( 2, 4, 5) UE( 3, 6, 7)  
        UE( 4,8,9) UE( 5,10,11) UE( 6,12,13) UE( 7,14,15) 
        UO( 8,0,1) UO( 9, 2, 3) UO(10, 4, 5) UO(11, 6, 7) 
        UO(12,8,9) UO(13,10,11) UO(14,12,13) UO(15,14,15)
    }

    i *= 256;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef UPDATE
#undef ODD
#undef EVEN
#undef LBLOCK
#undef LL
#undef LO
#undef UO
#undef UE

    for (size_t i = 0; i < 16; ++i) {
        _mm256_storeu_si256((__m256i*)tmp, counters[i]);
        for (int j = 0; j < 16; ++j)
            flags[i] += tmp[j];
    }
    return 0;
}

int pospopcnt_u16_avx2_harley_seal(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (16 * 16)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m256i* data = (const __m256i*)array;
    size_t size = len / 16;
    __m256i v1  = _mm256_setzero_si256();
    __m256i v2  = _mm256_setzero_si256();
    __m256i v4  = _mm256_setzero_si256();
    __m256i v8  = _mm256_setzero_si256();
    __m256i v16 = _mm256_setzero_si256();
    __m256i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint16_t buffer[16];
    __m256i counter[16];
    const __m256i one = _mm256_set1_epi16(1);

    while (i < limit) {        
        for (size_t i = 0; i < 16; ++i) {
            counter[i] = _mm256_setzero_si256();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm256_add_epi16(counter[pos], _mm256_and_si256(v16, one)); \
    v16 = _mm256_srli_epi16(v16, 1); \
}
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  0), _mm256_loadu_si256(data + i +  1));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  2), _mm256_loadu_si256(data + i +  3));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  4), _mm256_loadu_si256(data + i +  5));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  6), _mm256_loadu_si256(data + i +  7));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  8),  _mm256_loadu_si256(data + i +  9));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 10),  _mm256_loadu_si256(data + i + 11));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i + 12),  _mm256_loadu_si256(data + i + 13));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 14),  _mm256_loadu_si256(data + i + 15));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsB,&v4, foursA, foursB);
            U(0) U(1) U(2) U(3) U(4) U(5) U(6) U(7) U(8) U(9) U(10) U(11) U(12) U(13) U(14) U(15) // Updates
            pospopcnt_csa_avx2(&v16,    &v8, eightsA, eightsB);
#undef U
        }

        // update the counters after the last iteration
        for (size_t i = 0; i < 16; ++i) {
            counter[i] = _mm256_add_epi16(counter[i], _mm256_and_si256(v16, one));
            v16 = _mm256_srli_epi16(v16, 1);
        }
        
        for (size_t i = 0; i < 16; ++i) {
            _mm256_storeu_si256((__m256i*)buffer, counter[i]);
            for (size_t z = 0; z < 16; z++) {
                flags[i] += 16 * (uint32_t)buffer[z];
            }
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v1);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v2);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v4);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v8);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}

int pospopcnt_u16_avx2_harley_seal_improved(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (16 * 16)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m256i* data = (const __m256i*)array;
    size_t size = len / 16;
    __m256i v1  = _mm256_setzero_si256();
    __m256i v2  = _mm256_setzero_si256();
    __m256i v4  = _mm256_setzero_si256();
    __m256i v8  = _mm256_setzero_si256();
    __m256i v16 = _mm256_setzero_si256();
    __m256i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint16_t buffer[16];
    __m256i counter[4];
    for (size_t i = 0; i < 4; ++i) {
        counter[i] = _mm256_setzero_si256();
    }

    while (i < limit) {

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define horizreduce(input, mask, shift, output) \
        __m256i output; \
        { \
            const __m256i tmp0 = _mm256_and_si256(input, mask); \
            const __m256i tmp1 = _mm256_and_si256(_mm256_srli_epi32(input, shift), mask); \
            output = _mm256_hadd_epi16(tmp0, tmp1); \
        }

            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  0), _mm256_loadu_si256(data + i +  1));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  2), _mm256_loadu_si256(data + i +  3));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  4), _mm256_loadu_si256(data + i +  5));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  6), _mm256_loadu_si256(data + i +  7));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  8),  _mm256_loadu_si256(data + i +  9));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 10),  _mm256_loadu_si256(data + i + 11));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i + 12),  _mm256_loadu_si256(data + i + 13));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 14),  _mm256_loadu_si256(data + i + 15));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsB,&v4, foursA, foursB);
            {
                horizreduce(v16, _mm256_set1_epi8(0x55), 1, t0); // t0 = 2-bit counters
                horizreduce(t0,  _mm256_set1_epi8(0x33), 2, t1); // t1 = 4-bit counters
                horizreduce(t1,  _mm256_set1_epi8(0x0f), 4, t2); // t2 = 8-bit counters

                const __m256i mask_byte0 = _mm256_set1_epi32(0x000000ff);
                counter[0] = _mm256_add_epi32(counter[0], t2 & mask_byte0);
                counter[1] = _mm256_add_epi32(counter[1], _mm256_srli_epi32(t2, 1*8) & mask_byte0);
                counter[2] = _mm256_add_epi32(counter[2], _mm256_srli_epi32(t2, 2*8) & mask_byte0);
                counter[3] = _mm256_add_epi32(counter[3], _mm256_srli_epi32(t2, 3*8) & mask_byte0);
            }
            pospopcnt_csa_avx2(&v16, &v8, eightsA, eightsB);
        } // for

        // update the counters after the last iteration
        {
            horizreduce(v16, _mm256_set1_epi8(0x55), 1, t0);
            horizreduce(t0,  _mm256_set1_epi8(0x33), 2, t1);
            horizreduce(t1,  _mm256_set1_epi8(0x0f), 4, t2);

            const __m256i mask_byte0 = _mm256_set1_epi32(0x000000ff);
            counter[0] = _mm256_add_epi32(counter[0], t2 & mask_byte0);
            counter[1] = _mm256_add_epi32(counter[1], _mm256_srli_epi32(t2, 1*8) & mask_byte0);
            counter[2] = _mm256_add_epi32(counter[2], _mm256_srli_epi32(t2, 2*8) & mask_byte0);
            counter[3] = _mm256_add_epi32(counter[3], _mm256_srli_epi32(t2, 3*8) & mask_byte0);
        }
#undef horizreduce
    }

    counter[0] = _mm256_slli_epi32(counter[0], 4); // * 16
    counter[1] = _mm256_slli_epi32(counter[1], 4); // * 16
    counter[2] = _mm256_slli_epi32(counter[2], 4); // * 16
    counter[3] = _mm256_slli_epi32(counter[3], 4); // * 16

    flags[0] += _mm256_extract_epi32(counter[0], 0) + _mm256_extract_epi32(counter[0], 4);
    flags[1] += _mm256_extract_epi32(counter[2], 0) + _mm256_extract_epi32(counter[2], 4);
    flags[2] += _mm256_extract_epi32(counter[0], 1) + _mm256_extract_epi32(counter[0], 5);
    flags[3] += _mm256_extract_epi32(counter[2], 1) + _mm256_extract_epi32(counter[2], 5);
    flags[4] += _mm256_extract_epi32(counter[0], 2) + _mm256_extract_epi32(counter[0], 6);
    flags[5] += _mm256_extract_epi32(counter[2], 2) + _mm256_extract_epi32(counter[2], 6);
    flags[6] += _mm256_extract_epi32(counter[0], 3) + _mm256_extract_epi32(counter[0], 7);
    flags[7] += _mm256_extract_epi32(counter[2], 3) + _mm256_extract_epi32(counter[2], 7);

    flags[8]  += _mm256_extract_epi32(counter[1], 0) + _mm256_extract_epi32(counter[1], 4);
    flags[9]  += _mm256_extract_epi32(counter[3], 0) + _mm256_extract_epi32(counter[3], 4);
    flags[10] += _mm256_extract_epi32(counter[1], 1) + _mm256_extract_epi32(counter[1], 5);
    flags[11] += _mm256_extract_epi32(counter[3], 1) + _mm256_extract_epi32(counter[3], 5);
    flags[12] += _mm256_extract_epi32(counter[1], 2) + _mm256_extract_epi32(counter[1], 6);
    flags[13] += _mm256_extract_epi32(counter[3], 2) + _mm256_extract_epi32(counter[3], 6);
    flags[14] += _mm256_extract_epi32(counter[1], 3) + _mm256_extract_epi32(counter[1], 7);
    flags[15] += _mm256_extract_epi32(counter[3], 3) + _mm256_extract_epi32(counter[3], 7);

    _mm256_storeu_si256((__m256i*)buffer, v1);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v2);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v4);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v8);
    for (size_t i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}


int pospopcnt_u16_avx2_blend_popcnt_unroll4(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m256i* data_vectors = (const __m256i*)(array);
    const uint32_t n_cycles = len / 16;

    size_t i = 0;
    for (/**/; i + 4 <= n_cycles; i += 4) {
#define L(p) __m256i v##p = _mm256_loadu_si256(data_vectors+i+p);
        L(0) L(1) L(2) L(3)
        
#define U0(p,k) __m256i input##p = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16(0x00FF)), _mm256_slli_epi16(v##k, 8));
#define U1(p,k) __m256i input##k = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16((int16_t)0xFF00)), _mm256_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)
       U(0,1) U(2, 3)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3)

#define P0(p) input##p = _mm256_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 16;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P
    
    return 0;
}

int pospopcnt_u16_avx2_blend_popcnt_unroll8(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m256i* data_vectors = (const __m256i*)(array);
    const uint32_t n_cycles = len / 16;

    size_t i = 0;
    for (/**/; i + 8 <= n_cycles; i += 8) {
#define L(p) __m256i v##p = _mm256_loadu_si256(data_vectors+i+p);
        L(0) L(1) L(2) L(3)
        L(4) L(5) L(6) L(7) 
        
#define U0(p,k) __m256i input##p = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16(0x00FF)), _mm256_slli_epi16(v##k, 8));
#define U1(p,k) __m256i input##k = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16((int16_t)0xFF00)), _mm256_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)
       U(0,1) U(2, 3) U(4, 5) U(6, 7)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3) A(4, 5) A(6, 7)

#define P0(p) input##p = _mm256_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3) P(4, 5) P(6, 7)
        }
    }

    for (/**/; i + 4 <= n_cycles; i += 4) {
        L(0) L(1) L(2) L(3)
        U(0,1) U(2, 3)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A( 2, 3)
            P(0,1) P( 2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 16;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P

    return 0;
}

int pospopcnt_u16_avx2_blend_popcnt_unroll16(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m256i* data_vectors = (const __m256i*)(array);
    const uint32_t n_cycles = len / 16;

    size_t i = 0;
    for (/**/; i + 16 <= n_cycles; i += 16) {
#define L(p) __m256i v##p = _mm256_loadu_si256(data_vectors+i+p);
        L(0)  L(1)  L(2)  L(3)  
        L(4)  L(5)  L(6)  L(7) 
        L(8)  L(9)  L(10) L(11) 
        L(12) L(13) L(14) L(15)

#define U0(p,k) __m256i input##p = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16(0x00FF)), _mm256_slli_epi16(v##k, 8));
#define U1(p,k) __m256i input##k = _mm256_or_si256(_mm256_and_si256(v##p, _mm256_set1_epi16((int16_t)0xFF00)), _mm256_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)
    U(0,1) U( 2, 3) U( 4, 5) U( 6, 7)
    U(8,9) U(10,11) U(12,13) U(14,15)

        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm256_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A( 2, 3) A( 4, 5) A( 6, 7)
            A(8,9) A(10,11) A(12,13) A(14,15)

#define P0(p) input##p = _mm256_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P( 2, 3) P( 4, 5) P( 6, 7)
            P(8,9) P(10,11) P(12,13) P(14,15)
        }
    }

    for (/**/; i + 8 <= n_cycles; i += 8) {
        L(0) L(1) L(2) L(3)
        L(4) L(5) L(6) L(7) 
        U(0,1) U(2, 3) U(4, 5) U(6, 7)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A( 2, 3) A( 4, 5) A( 6, 7)
            P(0,1) P( 2, 3) P( 4, 5) P( 6, 7)
        }
    }

    for (/**/; i + 4 <= n_cycles; i += 4) {
        L(0) L(1) L(2) L(3)
        U(0,1) U(2, 3)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A( 2, 3)
            P(0,1) P( 2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P

    i *= 16;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }
    
    return 0;
}

static
__m256i avx2_merge1_odd(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8((int8_t)0xaa);
    const __m256i t1 = b & _mm256_set1_epi8((int8_t)0xaa);

    return t0 | (_mm256_srli_epi32(t1, 1));
}

static
__m256i avx2_merge1_even(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8(0x55);
    const __m256i t1 = b & _mm256_set1_epi8(0x55);

    return t0 | (_mm256_add_epi8(t1, t1));
}

static
__m256i avx2_merge2_odd(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8((int8_t)0xcc);
    const __m256i t1 = b & _mm256_set1_epi8((int8_t)0xcc);

    return t0 | (_mm256_srli_epi32(t1, 2));
}

static
__m256i avx2_merge2_even(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8(0x33);
    const __m256i t1 = b & _mm256_set1_epi8(0x33);

    return t0 | (_mm256_slli_epi32(t1, 2));
}

static
uint64_t avx2_sum_epu64(__m256i x) {
    return (uint64_t)_mm256_extract_epi64(x, 0)
         + (uint64_t)_mm256_extract_epi64(x, 1)
         + (uint64_t)_mm256_extract_epi64(x, 2)
         + (uint64_t)_mm256_extract_epi64(x, 3);
}

void pospopcnt_u8_avx2_popcnt4bit(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m256i zero = _mm256_setzero_si256();

    __m256i counter_a = zero;
    __m256i counter_b = zero;
    __m256i counter_c = zero;
    __m256i counter_d = zero;
    __m256i counter_e = zero;
    __m256i counter_f = zero;
    __m256i counter_g = zero;
    __m256i counter_h = zero;

    const __m256i popcnt_4bit = _mm256_setr_epi8(
        /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
        /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
        /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
        /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4,

        /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
        /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
        /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
        /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4
    );

    const __m256i lo_nibble = _mm256_set1_epi8(0x0f);

    int local = 0;
    __m256i counter8bit_a = zero;
    __m256i counter8bit_b = zero;
    __m256i counter8bit_c = zero;
    __m256i counter8bit_d = zero;
    __m256i counter8bit_e = zero;
    __m256i counter8bit_f = zero;
    __m256i counter8bit_g = zero;
    __m256i counter8bit_h = zero;

    for (const uint8_t* end = &data[len & ~(4*32 - 1)]; data != end; data += 4*32) {
        // r0 = [a0|b0|c0|d0|e0|f0|g0|h0]
        // r1 = [a1|b1|c1|d1|e1|f1|g1|h1]
        // r2 = [a2|b2|c2|d2|e2|f2|g2|h2]
        // r3 = [a3|b3|c3|d3|e3|f3|g3|h3]
        const __m256i r0 = _mm256_loadu_si256((__m256i*)&data[0*32]);
        const __m256i r1 = _mm256_loadu_si256((__m256i*)&data[1*32]);
        const __m256i r2 = _mm256_loadu_si256((__m256i*)&data[2*32]);
        const __m256i r3 = _mm256_loadu_si256((__m256i*)&data[3*32]);

        // s0 = [a0|a1|c0|c1|e0|e1|g0|g1]
        // s1 = [b0|b1|d0|d1|f0|f1|h0|h1]
        // s2 = [a2|a3|c2|c3|e2|e3|g2|g3]
        // s3 = [b2|b3|d2|d3|f2|f3|h2|h3]
        const __m256i s0 = avx2_merge1_even(r0, r1);
        const __m256i s1 = avx2_merge1_odd (r0, r1);
        const __m256i s2 = avx2_merge1_even(r2, r3);
        const __m256i s3 = avx2_merge1_odd (r2, r3);

        // d0 = [a0|a1|a2|a3|e0|e1|e2|e3]
        // d1 = [b0|b1|b2|b3|f0|f1|f2|f3]
        // d2 = [c0|c1|c2|c3|g0|g1|g2|g3]
        // d3 = [d0|d1|d2|d3|h0|h1|h2|h3]
        const __m256i d0 = avx2_merge2_even(s0, s2);
        const __m256i d1 = avx2_merge2_even(s1, s3);
        const __m256i d2 = avx2_merge2_odd (s0, s2);
        const __m256i d3 = avx2_merge2_odd (s1, s3);

        // popcnt for 4-bit subwords in each registers
        const __m256i popcnt_a = _mm256_shuffle_epi8(popcnt_4bit, d0 & lo_nibble);
        const __m256i popcnt_e = _mm256_shuffle_epi8(popcnt_4bit, _mm256_srli_epi32(d0, 4) & lo_nibble);
        const __m256i popcnt_b = _mm256_shuffle_epi8(popcnt_4bit, d1 & lo_nibble);
        const __m256i popcnt_f = _mm256_shuffle_epi8(popcnt_4bit, _mm256_srli_epi32(d1, 4) & lo_nibble);
        const __m256i popcnt_c = _mm256_shuffle_epi8(popcnt_4bit, d2 & lo_nibble);
        const __m256i popcnt_g = _mm256_shuffle_epi8(popcnt_4bit, _mm256_srli_epi32(d2, 4) & lo_nibble);
        const __m256i popcnt_d = _mm256_shuffle_epi8(popcnt_4bit, d3 & lo_nibble);
        const __m256i popcnt_h = _mm256_shuffle_epi8(popcnt_4bit, _mm256_srli_epi32(d3, 4) & lo_nibble);

        counter8bit_a = _mm256_add_epi8(counter8bit_a, popcnt_a);
        counter8bit_b = _mm256_add_epi8(counter8bit_b, popcnt_b);
        counter8bit_c = _mm256_add_epi8(counter8bit_c, popcnt_c);
        counter8bit_d = _mm256_add_epi8(counter8bit_d, popcnt_d);
        counter8bit_e = _mm256_add_epi8(counter8bit_e, popcnt_e);
        counter8bit_f = _mm256_add_epi8(counter8bit_f, popcnt_f);
        counter8bit_g = _mm256_add_epi8(counter8bit_g, popcnt_g);
        counter8bit_h = _mm256_add_epi8(counter8bit_h, popcnt_h);

        local += 1;
        if (local == 63) {
            // avoid overflows in the 8-bit counters
#define U(n) \
            counter_##n = _mm256_add_epi64(counter_##n, _mm256_sad_epu8(counter8bit_##n, zero)); \
            counter8bit_##n = _mm256_setzero_si256();

            U(a) U(b) U(c) U(d)
            U(e) U(f) U(g) U(h)
#undef U
            local = 0;
        }
    }

    if (local != 0) {
#define U(n) counter_##n = _mm256_add_epi64(counter_##n, _mm256_sad_epu8(counter8bit_##n, zero));
        U(a) U(b) U(c) U(d)
        U(e) U(f) U(g) U(h)
#undef U
    }

    flag_counts[0] += avx2_sum_epu64(counter_a);
    flag_counts[1] += avx2_sum_epu64(counter_b);
    flag_counts[2] += avx2_sum_epu64(counter_c);
    flag_counts[3] += avx2_sum_epu64(counter_d);
    flag_counts[4] += avx2_sum_epu64(counter_e);
    flag_counts[5] += avx2_sum_epu64(counter_f);
    flag_counts[6] += avx2_sum_epu64(counter_g);
    flag_counts[7] += avx2_sum_epu64(counter_h);

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % (4*32), flag_counts);
}

static
__m256i avx2_add1_odd(__m256i a, __m256i b) {
    const __m256i mask = _mm256_set1_epi8(0x55);
    const __m256i t0 = _mm256_srli_epi32(a, 1) & mask;
    const __m256i t1 = _mm256_srli_epi32(b, 1) & mask;

    return _mm256_add_epi32(t0, t1);
}

static
__m256i avx2_add1_even(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8(0x55);
    const __m256i t1 = b & _mm256_set1_epi8(0x55);

    return _mm256_add_epi32(t0, t1);
}

static
__m256i avx2_add2_odd(__m256i a, __m256i b) {
    const __m256i mask = _mm256_set1_epi8(0x33);
    const __m256i t0 = _mm256_srli_epi32(a, 2) & mask;
    const __m256i t1 = _mm256_srli_epi32(b, 2) & mask;

    return _mm256_add_epi32(t0, t1);
}

static
__m256i avx2_add2_even(__m256i a, __m256i b) {
    const __m256i t0 = a & _mm256_set1_epi8(0x33);
    const __m256i t1 = b & _mm256_set1_epi8(0x33);

    return _mm256_add_epi32(t0, t1);
}

void pospopcnt_u8_avx2_horizreduce(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m256i zero = _mm256_setzero_si256();

    __m256i counter_a = zero;
    __m256i counter_b = zero;
    __m256i counter_c = zero;
    __m256i counter_d = zero;
    __m256i counter_e = zero;
    __m256i counter_f = zero;
    __m256i counter_g = zero;
    __m256i counter_h = zero;

    for (const uint8_t* end = &data[(len & ~127)]; data != end; data += 128) {
        // r0 = [a0|b0|c0|d0|e0|f0|g0|h0]
        // r1 = [a1|b1|c1|d1|e1|f1|g1|h1]
        // r2 = [a2|b2|c2|d2|e2|f2|g2|h2]
        // r3 = [a3|b3|c3|d3|e3|f3|g3|h3]
        const __m256i r0 = _mm256_loadu_si256((__m256i*)&data[0*32]);
        const __m256i r1 = _mm256_loadu_si256((__m256i*)&data[1*32]);
        const __m256i r2 = _mm256_loadu_si256((__m256i*)&data[2*32]);
        const __m256i r3 = _mm256_loadu_si256((__m256i*)&data[3*32]);

        // s0 = [a0+a1|c0+c1|e0+e1|g0+g1]
        // s1 = [b0+b1|d0+d1|f0+f1|h0+h1]
        // s2 = [a2+a3|c2+c3|e2+e3|g2+g3]
        // s3 = [b2+b3|d2+d3|f2+f3|h2+h3]
        const __m256i s0 = avx2_add1_even(r0, r1);
        const __m256i s1 = avx2_add1_odd (r0, r1);
        const __m256i s2 = avx2_add1_even(r2, r3);
        const __m256i s3 = avx2_add1_odd (r2, r3);

        // d0 = [a0+a1+a2+a3|e0+e1+e2+e3]
        // d1 = [b0+b1+b2+b3|f0+f1+f2+f3]
        // d2 = [c0+c1+c2+c3|g0+g1+g2+g3]
        // d3 = [d0+d1+d2+d3|h0+h1+h2+h3]
        const __m256i d0 = avx2_add2_even(s0, s2);
        const __m256i d1 = avx2_add2_even(s1, s3);
        const __m256i d2 = avx2_add2_odd (s0, s2);
        const __m256i d3 = avx2_add2_odd (s1, s3);

        // popcnt for 4-bit subwords in each registers
        const __m256i mask_lo = _mm256_set1_epi8(0x0f);
        const __m256i mask_hi = _mm256_set1_epi8(0xf0);
        // Note: counter for higher nibble have to be divided by 16
        const __m256i sum0 = _mm256_sad_epu8(d0 & mask_lo, zero);
        const __m256i sum1 = _mm256_sad_epu8(d0 & mask_hi, zero);
        const __m256i sum2 = _mm256_sad_epu8(d1 & mask_lo, zero);
        const __m256i sum3 = _mm256_sad_epu8(d1 & mask_hi, zero);
        const __m256i sum4 = _mm256_sad_epu8(d2 & mask_lo, zero);
        const __m256i sum5 = _mm256_sad_epu8(d2 & mask_hi, zero);
        const __m256i sum6 = _mm256_sad_epu8(d3 & mask_lo, zero);
        const __m256i sum7 = _mm256_sad_epu8(d3 & mask_hi, zero);

        counter_a = _mm256_add_epi64(counter_a, sum0);
        counter_b = _mm256_add_epi64(counter_b, sum1);
        counter_c = _mm256_add_epi64(counter_c, sum2);
        counter_d = _mm256_add_epi64(counter_d, sum3);
        counter_e = _mm256_add_epi64(counter_e, sum4);
        counter_f = _mm256_add_epi64(counter_f, sum5);
        counter_g = _mm256_add_epi64(counter_g, sum6);
        counter_h = _mm256_add_epi64(counter_h, sum7);
    }

    flag_counts[0] += avx2_sum_epu64(counter_a);
    flag_counts[1] += avx2_sum_epu64(counter_c);
    flag_counts[2] += avx2_sum_epu64(counter_e);
    flag_counts[3] += avx2_sum_epu64(counter_g);
    flag_counts[4] += avx2_sum_epu64(counter_b) >> 4;
    flag_counts[5] += avx2_sum_epu64(counter_d) >> 4;
    flag_counts[6] += avx2_sum_epu64(counter_f) >> 4;
    flag_counts[7] += avx2_sum_epu64(counter_h) >> 4;

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % 256, flag_counts);
}

void pospopcnt_u32_avx2_harley_seal(const uint32_t* array, size_t len, uint32_t* flags) {
    for (size_t i = len - (len % (16 * 8)); i < len; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m256i* data = (const __m256i*)array;
    size_t size = len / 8;
    __m256i v1  = _mm256_setzero_si256();
    __m256i v2  = _mm256_setzero_si256();
    __m256i v4  = _mm256_setzero_si256();
    __m256i v8  = _mm256_setzero_si256();
    __m256i v16 = _mm256_setzero_si256();
    __m256i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint32_t buffer[8];
    __m256i counter[32];
    const __m256i one = _mm256_set1_epi32(1);

    while (i < limit) {
        for (size_t i = 0; i < 32; ++i) {
            counter[i] = _mm256_setzero_si256();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm256_add_epi32(counter[pos], _mm256_and_si256(v16, one)); \
    v16 = _mm256_srli_epi32(v16, 1); \
}
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  0), _mm256_loadu_si256(data + i +  1));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  2), _mm256_loadu_si256(data + i +  3));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  4), _mm256_loadu_si256(data + i +  5));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  6), _mm256_loadu_si256(data + i +  7));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  8),  _mm256_loadu_si256(data + i +  9));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 10),  _mm256_loadu_si256(data + i + 11));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i + 12),  _mm256_loadu_si256(data + i + 13));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 14),  _mm256_loadu_si256(data + i + 15));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsB,&v4, foursA, foursB);
            U( 0) U( 1) U( 2) U( 3) U( 4) U( 5) U( 6) U( 7) U( 8) U( 9) U(10) U(11) U(12) U(13) U(14) U(15)
            U(16) U(17) U(18) U(19) U(20) U(21) U(22) U(23) U(24) U(25) U(26) U(27) U(28) U(29) U(30) U(31)
            pospopcnt_csa_avx2(&v16,    &v8, eightsA, eightsB);
#undef U
        }

        // update the counters after the last iteration
        for (size_t i = 0; i < 32; ++i) {
            counter[i] = _mm256_add_epi32(counter[i], _mm256_and_si256(v16, one));
            v16 = _mm256_srli_epi32(v16, 1);
        }

        for (size_t i = 0; i < 32; ++i) {
            _mm256_storeu_si256((__m256i*)buffer, counter[i]);
            for (size_t z = 0; z < 8; z++) {
                flags[i] += 16 * buffer[z];
            }
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v1);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v2);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v4);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
}

// Note: a translation of pospopcnt_u32_sse_harley_seal_improved
void pospopcnt_u32_avx2_harley_seal_improved(const uint32_t* array, size_t len, uint32_t* flags) {
    for (size_t i = len - (len % (16 * 8)); i < len; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m256i* data = (const __m256i*)array;
    size_t size = len / 8;
    __m256i v1  = _mm256_setzero_si256();
    __m256i v2  = _mm256_setzero_si256();
    __m256i v4  = _mm256_setzero_si256();
    __m256i v8  = _mm256_setzero_si256();
    __m256i v16 = _mm256_setzero_si256();
    __m256i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const __m256i lookup = _mm256_setr_epi8(
        0,  4,  8, 12, 1,  5,  9, 13, 2,  6, 10, 14, 3,  7, 11, 15,
        0,  4,  8, 12, 1,  5,  9, 13, 2,  6, 10, 14, 3,  7, 11, 15);

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint32_t buffer[8];
    __m256i counter[8];

    while (i < limit) {
        for (int i = 0; i < 8; ++i) {
            counter[i] = _mm256_setzero_si256();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  0), _mm256_loadu_si256(data + i +  1));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  2), _mm256_loadu_si256(data + i +  3));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  4), _mm256_loadu_si256(data + i +  5));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i +  6), _mm256_loadu_si256(data + i +  7));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i +  8),  _mm256_loadu_si256(data + i +  9));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 10),  _mm256_loadu_si256(data + i + 11));
            pospopcnt_csa_avx2(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&twosA,  &v1, _mm256_loadu_si256(data + i + 12),  _mm256_loadu_si256(data + i + 13));
            pospopcnt_csa_avx2(&twosB,  &v1, _mm256_loadu_si256(data + i + 14),  _mm256_loadu_si256(data + i + 15));
            pospopcnt_csa_avx2(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx2(&eightsB,&v4, foursA, foursB);

            {
                const __m256i t0 = _mm256_shuffle_epi8(v16, lookup);
                const __m256i m0 = _mm256_and_si256(t0, _mm256_set1_epi8(0x55));
                const __m256i m1 = _mm256_and_si256(_mm256_srli_epi32(t0, 1), _mm256_set1_epi8(0x55));

                const __m256i s0 = _mm256_maddubs_epi16(m0, _mm256_set1_epi16(0x0101));
                const __m256i s1 = _mm256_maddubs_epi16(m1, _mm256_set1_epi16(0x0101));

                const __m256i t1 = _mm256_or_si256(s0, _mm256_slli_epi32(s1, 8));

                const __m256i n0 = _mm256_and_si256(t1, _mm256_set1_epi8(0x33));
                const __m256i n1 = _mm256_and_si256(_mm256_srli_epi32(t1, 2), _mm256_set1_epi8(0x33));

                const __m256i S0 = _mm256_madd_epi16(n0, _mm256_set1_epi32(0x00010001));
                const __m256i S1 = _mm256_madd_epi16(n1, _mm256_set1_epi32(0x00010001));

                const __m256i mask = _mm256_set1_epi32(0x0000000f);
                counter[0] = _mm256_add_epi32(counter[0], _mm256_and_si256(S0, mask));
                counter[1] = _mm256_add_epi32(counter[1], _mm256_and_si256(_mm256_srli_epi32(S0,  4), mask));
                counter[2] = _mm256_add_epi32(counter[2], _mm256_and_si256(_mm256_srli_epi32(S0,  8), mask));
                counter[3] = _mm256_add_epi32(counter[3], _mm256_and_si256(_mm256_srli_epi32(S0, 12), mask));
                counter[4] = _mm256_add_epi32(counter[4], _mm256_and_si256(S1, mask));
                counter[5] = _mm256_add_epi32(counter[5], _mm256_and_si256(_mm256_srli_epi32(S1,  4), mask));
                counter[6] = _mm256_add_epi32(counter[6], _mm256_and_si256(_mm256_srli_epi32(S1,  8), mask));
                counter[7] = _mm256_add_epi32(counter[7], _mm256_and_si256(_mm256_srli_epi32(S1, 12), mask));
            }

            pospopcnt_csa_avx2(&v16,    &v8, eightsA, eightsB);
        }

        // update the counters after the last iteration
        {
            const __m256i t0 = _mm256_shuffle_epi8(v16, lookup);

            const __m256i m0 = _mm256_and_si256(t0, _mm256_set1_epi8(0x55));
            const __m256i m1 = _mm256_and_si256(_mm256_srli_epi32(t0, 1), _mm256_set1_epi8(0x55));

            const __m256i s0 = _mm256_maddubs_epi16(m0, _mm256_set1_epi16(0x0101));
            const __m256i s1 = _mm256_maddubs_epi16(m1, _mm256_set1_epi16(0x0101));

            const __m256i t1 = _mm256_or_si256(s0, _mm256_slli_epi32(s1, 8));

            const __m256i n0 = _mm256_and_si256(t1, _mm256_set1_epi8(0x33));
            const __m256i n1 = _mm256_and_si256(_mm256_srli_epi32(t1, 2), _mm256_set1_epi8(0x33));

            const __m256i S0 = _mm256_madd_epi16(n0, _mm256_set1_epi32(0x00010001));
            const __m256i S1 = _mm256_madd_epi16(n1, _mm256_set1_epi32(0x00010001));

            const __m256i mask = _mm256_set1_epi32(0x0000000f);
            counter[0] = _mm256_add_epi32(counter[0], _mm256_and_si256(S0, mask));
            counter[1] = _mm256_add_epi32(counter[1], _mm256_and_si256(_mm256_srli_epi32(S0,  4), mask));
            counter[2] = _mm256_add_epi32(counter[2], _mm256_and_si256(_mm256_srli_epi32(S0,  8), mask));
            counter[3] = _mm256_add_epi32(counter[3], _mm256_and_si256(_mm256_srli_epi32(S0, 12), mask));
            counter[4] = _mm256_add_epi32(counter[4], _mm256_and_si256(S1, mask));
            counter[5] = _mm256_add_epi32(counter[5], _mm256_and_si256(_mm256_srli_epi32(S1,  4), mask));
            counter[6] = _mm256_add_epi32(counter[6], _mm256_and_si256(_mm256_srli_epi32(S1,  8), mask));
            counter[7] = _mm256_add_epi32(counter[7], _mm256_and_si256(_mm256_srli_epi32(S1, 12), mask));

#define update_flags(counterN, index) \
            _mm256_storeu_si256((__m256i*)buffer, _mm256_slli_epi32(counterN, 4)); \
            flags[index + 8*0] += buffer[0]; \
            flags[index + 8*1] += buffer[1]; \
            flags[index + 8*2] += buffer[2]; \
            flags[index + 8*3] += buffer[3]; \
            flags[index + 8*0] += buffer[4]; \
            flags[index + 8*1] += buffer[5]; \
            flags[index + 8*2] += buffer[6]; \
            flags[index + 8*3] += buffer[7];

            update_flags(counter[0], 0);
            update_flags(counter[1], 4);
            update_flags(counter[2], 1);
            update_flags(counter[3], 5);
            update_flags(counter[4], 2);
            update_flags(counter[5], 6);
            update_flags(counter[6], 3);
            update_flags(counter[7], 7);
#undef update_flags
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v1);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm256_storeu_si256((__m256i*)buffer, v2);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v4);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm256_storeu_si256((__m256i*)buffer, v8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
}


make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_lemire, pospopcnt_u16_avx2_lemire)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_lemire2, pospopcnt_u16_avx2_lemire2)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_blend_popcnt, pospopcnt_u16_avx2_blend_popcnt)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_blend_popcnt_unroll4, pospopcnt_u16_avx2_blend_popcnt_unroll4)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_blend_popcnt_unroll8, pospopcnt_u16_avx2_blend_popcnt_unroll8)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_blend_popcnt_unroll16, pospopcnt_u16_avx2_blend_popcnt_unroll16)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_adder_forest, pospopcnt_u16_avx2_adder_forest)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx2_harley_seal, pospopcnt_u16_avx2_harley_seal)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx2_lemire)
pospopcnt_u16_stub(pospopcnt_u16_avx2_lemire2)
pospopcnt_u16_stub(pospopcnt_u16_avx2_blend_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_avx2_blend_popcnt_unroll4)
pospopcnt_u16_stub(pospopcnt_u16_avx2_blend_popcnt_unroll8)
pospopcnt_u16_stub(pospopcnt_u16_avx2_blend_popcnt_unroll16)
pospopcnt_u16_stub(pospopcnt_u16_avx2_adder_forest)
pospopcnt_u16_stub(pospopcnt_u16_avx2_harley_seal)
pospopcnt_u16_stub(pospopcnt_u16_avx2_harley_seal_improved)
pospopcnt_u8_stub(pospopcnt_u8_avx2_lemire)
pospopcnt_u8_stub(pospopcnt_u8_avx2_lemire2)
pospopcnt_u8_stub(pospopcnt_u8_avx2_blend_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_avx2_blend_popcnt_unroll4)
pospopcnt_u8_stub(pospopcnt_u8_avx2_blend_popcnt_unroll8)
pospopcnt_u8_stub(pospopcnt_u8_avx2_blend_popcnt_unroll16)
pospopcnt_u8_stub(pospopcnt_u8_avx2_adder_forest)
pospopcnt_u8_stub(pospopcnt_u8_avx2_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_avx2_popcnt4bit)
pospopcnt_u8_stub(pospopcnt_u8_avx2_horizreduce)
pospopcnt_u32_stub(pospopcnt_u32_avx2_harley_seal)
pospopcnt_u32_stub(pospopcnt_u32_avx2_harley_seal_improved)
#endif

#if POSPOPCNT_SIMD_VERSION >= 3
int pospopcnt_u16_sse_blend_popcnt(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m128i* data_vectors = (const __m128i*)(array);
    const uint32_t n_cycles = len / 8;

    size_t i = 0;
    for (/**/; i + 2 <= n_cycles; i += 2) {
        __m128i v0 = _mm_loadu_si128(data_vectors + i + 0);
        __m128i v1 = _mm_loadu_si128(data_vectors + i + 1);

        __m128i input0 = _mm_or_si128(_mm_and_si128(v0, _mm_set1_epi16(0x00FF)), _mm_slli_epi16(v1, 8));
        __m128i input1 = _mm_or_si128(_mm_and_si128(v0, _mm_set1_epi16((int16_t)0xFF00)), _mm_srli_epi16(v1, 8));
        
        for (int i = 0; i < 8; ++i) {
            flags[ 7 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input0));
            flags[15 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input1));
            input0 = _mm_add_epi8(input0, input0);
            input1 = _mm_add_epi8(input1, input1);
        }
    }

    i *= 8;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }
    
    return 0;
}

int pospopcnt_u16_sse_blend_popcnt_unroll4(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m128i* data_vectors = (const __m128i*)(array);
    const uint32_t n_cycles = len / 8;

    size_t i = 0;
    for (/**/; i + 4 <= n_cycles; i += 4) {
#define L(p) __m128i v##p =  _mm_loadu_si128(data_vectors+i+p);
        L(0) L(1) L(2) L(3)

#define U0(p,k) __m128i input##p = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16(0x00FF)), _mm_slli_epi16(v##k, 8));
#define U1(p,k) __m128i input##k = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16((int16_t)0xFF00)), _mm_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)

        U(0,1) U(2,3)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3)

#define P0(p) input##p = _mm_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 8;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P
    
    return 0;
}

int pospopcnt_u16_sse_blend_popcnt_unroll8(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m128i* data_vectors = (const __m128i*)(array);
    const uint32_t n_cycles = len / 8;

    size_t i = 0;
    for (/**/; i + 8 <= n_cycles; i += 8) {
#define L(p) __m128i v##p = _mm_loadu_si128(data_vectors+i+p);
        L(0) L(1) L(2) L(3)
        L(4) L(5) L(6) L(7)

#define U0(p,k) __m128i input##p = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16(0x00FF)), _mm_slli_epi16(v##k, 8));
#define U1(p,k) __m128i input##k = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16((int16_t)0xFF00)), _mm_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)

        U(0,1) U(2,3) U(4,5) U(6,7)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3) A(4,5) A(6, 7)

#define P0(p) input##p = _mm_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3) P(4,5) P(6, 7)
        }
    }

    for (/**/; i + 4 <= n_cycles; i += 4) {
        L(0) L(1) L(2) L(3)
        U(0,1) U(2,3)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A(2, 3)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 8;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P

    return 0;
}

int pospopcnt_u16_sse_blend_popcnt_unroll16(const uint16_t* array, uint32_t len, uint32_t* flags) {
    const __m128i* data_vectors = (const __m128i*)(array);
    const uint32_t n_cycles = len / 8;

    size_t i = 0;
    for (/**/; i + 16 <= n_cycles; i += 16) {
#define L(p) __m128i v##p = _mm_loadu_si128(data_vectors+i+p);
        L(0)  L(1)  L(2)  L(3)  
        L(4)  L(5)  L(6)  L(7) 
        L(8)  L(9)  L(10) L(11) 
        L(12) L(13) L(14) L(15)

#define U0(p,k) __m128i input##p = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16(0x00FF)), _mm_slli_epi16(v##k, 8));
#define U1(p,k) __m128i input##k = _mm_or_si128(_mm_and_si128(v##p, _mm_set1_epi16((int16_t)0xFF00)), _mm_srli_epi16(v##k, 8));
#define U(p, k)  U0(p,k) U1(p,k)

        U(0,1) U( 2, 3) U( 4, 5) U( 6, 7)
        U(8,9) U(10,11) U(12,13) U(14,15)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u32(_mm_movemask_epi8(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A( 2,  3) A( 4, 5) A( 6,  7)
            A(8,9) A(10, 11) A(12,13) A(14, 15)

#define P0(p) input##p = _mm_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P( 2,  3) P( 4, 5) P( 6,  7)
            P(8,9) P(10, 11) P(12,13) P(14, 15)
        }
    }

    for (/**/; i + 8 <= n_cycles; i += 8) {
        L(0) L(1) L(2) L(3)
        L(4) L(5) L(6) L(7)

        U(0,1) U(2,3) U(4,5) U(6,7)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A(2, 3) A(4,5) A(6, 7)
            P(0,1) P(2, 3) P(4,5) P(6, 7)
        }
    }

    for (/**/; i + 4 <= n_cycles; i += 4) {
        L(0) L(1) L(2) L(3)
        U(0,1) U(2,3)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A(2, 3)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 8;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P
    
    return 0;
}

int pospopcnt_u16_sse_harley_seal(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (16 * 8)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m128i* data = (const __m128i*)array;
    size_t size = len / 8;
    __m128i v1  = _mm_setzero_si128();
    __m128i v2  = _mm_setzero_si128();
    __m128i v4  = _mm_setzero_si128();
    __m128i v8  = _mm_setzero_si128();
    __m128i v16 = _mm_setzero_si128();
    __m128i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint16_t buffer[8];
    __m128i counter[16];

    while (i < limit) {        
        for (size_t i = 0; i < 16; ++i) {
            counter[i] = _mm_setzero_si128();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm_add_epi16(counter[pos], _mm_and_si128(v16, _mm_set1_epi16(1))); \
    v16 = _mm_srli_epi16(v16, 1); \
}
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  0), _mm_loadu_si128(data + i +  1));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  2), _mm_loadu_si128(data + i +  3));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  4), _mm_loadu_si128(data + i +  5));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  6), _mm_loadu_si128(data + i +  7));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  8),  _mm_loadu_si128(data + i +  9));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 10),  _mm_loadu_si128(data + i + 11));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i + 12),  _mm_loadu_si128(data + i + 13));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 14),  _mm_loadu_si128(data + i + 15));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsB,&v4, foursA, foursB);
            U(0) U(1) U(2) U(3) U(4) U(5) U(6) U(7) U(8) U(9) U(10) U(11) U(12) U(13) U(14) U(15) // Updates
            pospopcnt_csa_sse(&v16,    &v8, eightsA, eightsB);
#undef U
        }

        // update the counters after the last iteration
        for (size_t i = 0; i < 16; ++i) {
            counter[i] = _mm_add_epi16(counter[i], _mm_and_si128(v16, _mm_set1_epi16(1)));
            v16 = _mm_srli_epi16(v16, 1);
        }
        
        for (size_t i = 0; i < 16; ++i) {
            _mm_storeu_si128((__m128i*)buffer, counter[i]);
            for (size_t z = 0; z < 8; z++) {
                flags[i] += 16 * (uint32_t)buffer[z];
            }
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v1);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v2);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v4);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v8);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}

/*
   This version differs from pospopcnt_u16_sse_harley_seal with the step of
   updating v16 vector (the vector of MSBs). We horizontally reduce all bits
   from input and end up with sixteen 8-bit sums.  The sums are then used to
   update the auxiliary counters.

   In the original algorithm update is done in a simple loop. The loop's body
   consists 3 instructions and there're 16 iterations, total 48 instructions.
   In the improved version we have 23 instructions (~2 fold).

   Additionally, the number of auxiliary counters was reduced from 16 to 4 (4 fold).
*/
int pospopcnt_u16_sse_harley_seal_improved(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (16 * 8)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m128i* data = (const __m128i*)array;
    size_t size = len / 8;
    __m128i v1  = _mm_setzero_si128();
    __m128i v2  = _mm_setzero_si128();
    __m128i v4  = _mm_setzero_si128();
    __m128i v8  = _mm_setzero_si128();
    __m128i v16 = _mm_setzero_si128();
    __m128i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint16_t buffer[8];
    __m128i counter[4];
    for (size_t i = 0; i < 4; ++i) {
        counter[i] = _mm_setzero_si128();
    }

    while (i < limit) {

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define horizreduce(input, mask, shift, output) \
        __m128i output; \
        { \
            const __m128i tmp0 = _mm_and_si128(input, mask); \
            const __m128i tmp1 = _mm_and_si128(_mm_srli_epi32(input, shift), mask); \
            output = _mm_hadd_epi16(tmp0, tmp1); \
        }

            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  0), _mm_loadu_si128(data + i +  1));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  2), _mm_loadu_si128(data + i +  3));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  4), _mm_loadu_si128(data + i +  5));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  6), _mm_loadu_si128(data + i +  7));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  8),  _mm_loadu_si128(data + i +  9));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 10),  _mm_loadu_si128(data + i + 11));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i + 12),  _mm_loadu_si128(data + i + 13));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 14),  _mm_loadu_si128(data + i + 15));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsB,&v4, foursA, foursB);
            {
                horizreduce(v16, _mm_set1_epi8(0x55), 1, t0); // t0 = 2-bit counters
                horizreduce(t0,  _mm_set1_epi8(0x33), 2, t1); // t1 = 4-bit counters
                horizreduce(t1,  _mm_set1_epi8(0x0f), 4, t2); // t2 = 8-bit counters

                const __m128i mask_byte0 = _mm_set1_epi32(0x000000ff);
                counter[0] = _mm_add_epi32(counter[0], t2 & mask_byte0);
                counter[1] = _mm_add_epi32(counter[1], _mm_srli_epi32(t2, 1*8) & mask_byte0);
                counter[2] = _mm_add_epi32(counter[2], _mm_srli_epi32(t2, 2*8) & mask_byte0);
                counter[3] = _mm_add_epi32(counter[3], _mm_srli_epi32(t2, 3*8) & mask_byte0);
            }
            pospopcnt_csa_sse(&v16, &v8, eightsA, eightsB);
        } // for

        // update the counters after the last iteration
        {
            horizreduce(v16, _mm_set1_epi8(0x55), 1, t0);
            horizreduce(t0,  _mm_set1_epi8(0x33), 2, t1);
            horizreduce(t1,  _mm_set1_epi8(0x0f), 4, t2);

            const __m128i mask_byte0 = _mm_set1_epi32(0x000000ff);
            counter[0] = _mm_add_epi32(counter[0], t2 & mask_byte0);
            counter[1] = _mm_add_epi32(counter[1], _mm_srli_epi32(t2, 1*8) & mask_byte0);
            counter[2] = _mm_add_epi32(counter[2], _mm_srli_epi32(t2, 2*8) & mask_byte0);
            counter[3] = _mm_add_epi32(counter[3], _mm_srli_epi32(t2, 3*8) & mask_byte0);
        }
#undef horizreduce
    }

    counter[0] = _mm_slli_epi32(counter[0], 4); // * 16
    counter[1] = _mm_slli_epi32(counter[1], 4); // * 16
    counter[2] = _mm_slli_epi32(counter[2], 4); // * 16
    counter[3] = _mm_slli_epi32(counter[3], 4); // * 16

#if 0 // Left to illustrate the mapping between auxiliary counters and flags.
    flags[0] += _mm_extract_epi32(counter[0], 0);
    flags[1] += _mm_extract_epi32(counter[2], 0);
    flags[2] += _mm_extract_epi32(counter[0], 1);
    flags[3] += _mm_extract_epi32(counter[2], 1);
    flags[4] += _mm_extract_epi32(counter[0], 2);
    flags[5] += _mm_extract_epi32(counter[2], 2);
    flags[6] += _mm_extract_epi32(counter[0], 3);
    flags[7] += _mm_extract_epi32(counter[2], 3);

    flags[8]  += _mm_extract_epi32(counter[1], 0);
    flags[9]  += _mm_extract_epi32(counter[3], 0);
    flags[10] += _mm_extract_epi32(counter[1], 1);
    flags[11] += _mm_extract_epi32(counter[3], 1);
    flags[12] += _mm_extract_epi32(counter[1], 2);
    flags[13] += _mm_extract_epi32(counter[3], 2);
    flags[14] += _mm_extract_epi32(counter[1], 3);
    flags[15] += _mm_extract_epi32(counter[3], 3);
#else
#define update_flags(offset, counters)                  \
    {                                                   \
        __m128i* ptr = (__m128i*)(flags + offset);      \
        const __m128i f = _mm_loadu_si128(ptr);         \
        const __m128i s = _mm_add_epi32(f, counters);   \
        _mm_storeu_si128(ptr, s);                       \
    }
    update_flags(0  /*  0 ..  3 */, _mm_unpacklo_epi32(counter[0], counter[2]));
    update_flags(4  /*  4 ..  7 */, _mm_unpackhi_epi32(counter[0], counter[2]));
    update_flags(8  /*  8 .. 11 */, _mm_unpacklo_epi32(counter[1], counter[3]));
    update_flags(12 /* 12 .. 15 */, _mm_unpackhi_epi32(counter[1], counter[3]));
#undef update_flags
#endif

    _mm_storeu_si128((__m128i*)buffer, v1);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v2);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v4);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v8);
    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}

static
__m128i sse4_merge1_odd(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8((int8_t)0xaa);
    const __m128i t1 = b & _mm_set1_epi8((int8_t)0xaa);

    return t0 | (_mm_srli_epi32(t1, 1));
}

static
__m128i sse4_merge1_even(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8(0x55);
    const __m128i t1 = b & _mm_set1_epi8(0x55);

    return t0 | (_mm_add_epi8(t1, t1));
}

static
__m128i sse4_merge2_odd(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8((int8_t)0xcc);
    const __m128i t1 = b & _mm_set1_epi8((int8_t)0xcc);

    return t0 | (_mm_srli_epi32(t1, 2));
}

static
__m128i sse4_merge2_even(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8(0x33);
    const __m128i t1 = b & _mm_set1_epi8(0x33);

    return t0 | (_mm_slli_epi32(t1, 2));
}

static
uint64_t sse4_sum_epu64(__m128i x) {
    return (uint64_t)_mm_extract_epi64(x, 0)
         + (uint64_t)_mm_extract_epi64(x, 1);
}

void pospopcnt_u8_sse_popcnt4bit(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m128i zero = _mm_setzero_si128();

    __m128i counter_a = zero;
    __m128i counter_b = zero;
    __m128i counter_c = zero;
    __m128i counter_d = zero;
    __m128i counter_e = zero;
    __m128i counter_f = zero;
    __m128i counter_g = zero;
    __m128i counter_h = zero;

    const __m128i popcnt_4bit = _mm_setr_epi8(
        /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
        /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
        /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
        /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4
    );

    const __m128i lo_nibble = _mm_set1_epi8(0x0f);

    int local = 0;
    __m128i counter8bit_a = zero;
    __m128i counter8bit_b = zero;
    __m128i counter8bit_c = zero;
    __m128i counter8bit_d = zero;
    __m128i counter8bit_e = zero;
    __m128i counter8bit_f = zero;
    __m128i counter8bit_g = zero;
    __m128i counter8bit_h = zero;

    for (const uint8_t* end = &data[(len & ~63)]; data != end; data += 64) {
        // r0 = [a0|b0|c0|d0|e0|f0|g0|h0]
        // r1 = [a1|b1|c1|d1|e1|f1|g1|h1]
        // r2 = [a2|b2|c2|d2|e2|f2|g2|h2]
        // r3 = [a3|b3|c3|d3|e3|f3|g3|h3]
        const __m128i r0 = _mm_loadu_si128((__m128i*)&data[0*16]);
        const __m128i r1 = _mm_loadu_si128((__m128i*)&data[1*16]);
        const __m128i r2 = _mm_loadu_si128((__m128i*)&data[2*16]);
        const __m128i r3 = _mm_loadu_si128((__m128i*)&data[3*16]);

        // s0 = [a0|a1|c0|c1|e0|e1|g0|g1]
        // s1 = [b0|b1|d0|d1|f0|f1|h0|h1]
        // s2 = [a2|a3|c2|c3|e2|e3|g2|g3]
        // s3 = [b2|b3|d2|d3|f2|f3|h2|h3]
        const __m128i s0 = sse4_merge1_even(r0, r1);
        const __m128i s1 = sse4_merge1_odd (r0, r1);
        const __m128i s2 = sse4_merge1_even(r2, r3);
        const __m128i s3 = sse4_merge1_odd (r2, r3);

        // d0 = [a0|a1|a2|a3|e0|e1|e2|e3]
        // d1 = [b0|b1|b2|b3|f0|f1|f2|f3]
        // d2 = [c0|c1|c2|c3|g0|g1|g2|g3]
        // d3 = [d0|d1|d2|d3|h0|h1|h2|h3]
        const __m128i d0 = sse4_merge2_even(s0, s2);
        const __m128i d1 = sse4_merge2_even(s1, s3);
        const __m128i d2 = sse4_merge2_odd (s0, s2);
        const __m128i d3 = sse4_merge2_odd (s1, s3);

        // popcnt for 4-bit subwords in each registers
        const __m128i popcnt_a = _mm_shuffle_epi8(popcnt_4bit, d0 & lo_nibble);
        const __m128i popcnt_e = _mm_shuffle_epi8(popcnt_4bit, _mm_srli_epi32(d0, 4) & lo_nibble);
        const __m128i popcnt_b = _mm_shuffle_epi8(popcnt_4bit, d1 & lo_nibble);
        const __m128i popcnt_f = _mm_shuffle_epi8(popcnt_4bit, _mm_srli_epi32(d1, 4) & lo_nibble);
        const __m128i popcnt_c = _mm_shuffle_epi8(popcnt_4bit, d2 & lo_nibble);
        const __m128i popcnt_g = _mm_shuffle_epi8(popcnt_4bit, _mm_srli_epi32(d2, 4) & lo_nibble);
        const __m128i popcnt_d = _mm_shuffle_epi8(popcnt_4bit, d3 & lo_nibble);
        const __m128i popcnt_h = _mm_shuffle_epi8(popcnt_4bit, _mm_srli_epi32(d3, 4) & lo_nibble);

        counter8bit_a = _mm_add_epi8(counter8bit_a, popcnt_a);
        counter8bit_b = _mm_add_epi8(counter8bit_b, popcnt_b);
        counter8bit_c = _mm_add_epi8(counter8bit_c, popcnt_c);
        counter8bit_d = _mm_add_epi8(counter8bit_d, popcnt_d);
        counter8bit_e = _mm_add_epi8(counter8bit_e, popcnt_e);
        counter8bit_f = _mm_add_epi8(counter8bit_f, popcnt_f);
        counter8bit_g = _mm_add_epi8(counter8bit_g, popcnt_g);
        counter8bit_h = _mm_add_epi8(counter8bit_h, popcnt_h);

        local += 1;
        if (local == 63) {
            // avoid overflows in the 8-bit counters
#define U(n) \
            counter_##n = _mm_add_epi64(counter_##n, _mm_sad_epu8(counter8bit_##n, zero)); \
            counter8bit_##n = _mm_setzero_si128();

            U(a) U(b) U(c) U(d)
            U(e) U(f) U(g) U(h)
#undef U
            local = 0;
        }
    }

    if (local != 0) {
#define U(n) counter_##n = _mm_add_epi64(counter_##n, _mm_sad_epu8(counter8bit_##n, zero));
        U(a) U(b) U(c) U(d)
        U(e) U(f) U(g) U(h)
#undef U
    }

    flag_counts[0] += sse4_sum_epu64(counter_a);
    flag_counts[1] += sse4_sum_epu64(counter_b);
    flag_counts[2] += sse4_sum_epu64(counter_c);
    flag_counts[3] += sse4_sum_epu64(counter_d);
    flag_counts[4] += sse4_sum_epu64(counter_e);
    flag_counts[5] += sse4_sum_epu64(counter_f);
    flag_counts[6] += sse4_sum_epu64(counter_g);
    flag_counts[7] += sse4_sum_epu64(counter_h);

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % 64, flag_counts);
}

static
__m128i sse4_add1_odd(__m128i a, __m128i b) {
    const __m128i mask = _mm_set1_epi8(0x55);
    const __m128i t0 = _mm_srli_epi32(a, 1) & mask;
    const __m128i t1 = _mm_srli_epi32(b, 1) & mask;

    return _mm_add_epi32(t0, t1);
}

static
__m128i sse4_add1_even(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8(0x55);
    const __m128i t1 = b & _mm_set1_epi8(0x55);

    return _mm_add_epi32(t0, t1);
}

static
__m128i sse4_add2_odd(__m128i a, __m128i b) {
    const __m128i mask = _mm_set1_epi8(0x33);
    const __m128i t0 = _mm_srli_epi32(a, 2) & mask;
    const __m128i t1 = _mm_srli_epi32(b, 2) & mask;

    return _mm_add_epi32(t0, t1);
}

static
__m128i sse4_add2_even(__m128i a, __m128i b) {
    const __m128i t0 = a & _mm_set1_epi8(0x33);
    const __m128i t1 = b & _mm_set1_epi8(0x33);

    return _mm_add_epi32(t0, t1);
}

/*
    This variant follow is a modification of pospopcnt_u8_sse_popcnt4bit.
    Instead of merging bits into subwords we add them and then add these
    subwords using PSDABW instructions. As a result we're getting wide
    64-bit counters, no bookeeping needed.

    The pospopcnt_u8_sse_popcnt4bit approach is still suitable for
    AVX512 vectorized popcounts.
*/
void pospopcnt_u8_sse_horizreduce(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m128i zero = _mm_setzero_si128();

    __m128i counter_a = zero;
    __m128i counter_b = zero;
    __m128i counter_c = zero;
    __m128i counter_d = zero;
    __m128i counter_e = zero;
    __m128i counter_f = zero;
    __m128i counter_g = zero;
    __m128i counter_h = zero;

    for (const uint8_t* end = &data[(len & ~63)]; data != end; data += 64) {
        // r0 = [a0|b0|c0|d0|e0|f0|g0|h0]
        // r1 = [a1|b1|c1|d1|e1|f1|g1|h1]
        // r2 = [a2|b2|c2|d2|e2|f2|g2|h2]
        // r3 = [a3|b3|c3|d3|e3|f3|g3|h3]
        const __m128i r0 = _mm_loadu_si128((__m128i*)&data[0*16]);
        const __m128i r1 = _mm_loadu_si128((__m128i*)&data[1*16]);
        const __m128i r2 = _mm_loadu_si128((__m128i*)&data[2*16]);
        const __m128i r3 = _mm_loadu_si128((__m128i*)&data[3*16]);

        // s0 = [a0+a1|c0+c1|e0+e1|g0+g1]
        // s1 = [b0+b1|d0+d1|f0+f1|h0+h1]
        // s2 = [a2+a3|c2+c3|e2+e3|g2+g3]
        // s3 = [b2+b3|d2+d3|f2+f3|h2+h3]
        const __m128i s0 = sse4_add1_even(r0, r1);
        const __m128i s1 = sse4_add1_odd (r0, r1);
        const __m128i s2 = sse4_add1_even(r2, r3);
        const __m128i s3 = sse4_add1_odd (r2, r3);

        // d0 = [a0+a1+a2+a3|e0+e1+e2+e3]
        // d1 = [b0+b1+b2+b3|f0+f1+f2+f3]
        // d2 = [c0+c1+c2+c3|g0+g1+g2+g3]
        // d3 = [d0+d1+d2+d3|h0+h1+h2+h3]
        const __m128i d0 = sse4_add2_even(s0, s2);
        const __m128i d1 = sse4_add2_even(s1, s3);
        const __m128i d2 = sse4_add2_odd (s0, s2);
        const __m128i d3 = sse4_add2_odd (s1, s3);

        // popcnt for 4-bit subwords in each registers
        const __m128i mask_lo = _mm_set1_epi8(0x0f);
        const __m128i mask_hi = _mm_set1_epi8(0xf0);
        // Note: counter for higher nibble have to be divided by 16
        const __m128i sum0 = _mm_sad_epu8(d0 & mask_lo, zero);
        const __m128i sum1 = _mm_sad_epu8(d0 & mask_hi, zero);
        const __m128i sum2 = _mm_sad_epu8(d1 & mask_lo, zero);
        const __m128i sum3 = _mm_sad_epu8(d1 & mask_hi, zero);
        const __m128i sum4 = _mm_sad_epu8(d2 & mask_lo, zero);
        const __m128i sum5 = _mm_sad_epu8(d2 & mask_hi, zero);
        const __m128i sum6 = _mm_sad_epu8(d3 & mask_lo, zero);
        const __m128i sum7 = _mm_sad_epu8(d3 & mask_hi, zero);

        counter_a = _mm_add_epi64(counter_a, sum0);
        counter_b = _mm_add_epi64(counter_b, sum1);
        counter_c = _mm_add_epi64(counter_c, sum2);
        counter_d = _mm_add_epi64(counter_d, sum3);
        counter_e = _mm_add_epi64(counter_e, sum4);
        counter_f = _mm_add_epi64(counter_f, sum5);
        counter_g = _mm_add_epi64(counter_g, sum6);
        counter_h = _mm_add_epi64(counter_h, sum7);
    }

    flag_counts[0] += sse4_sum_epu64(counter_a);
    flag_counts[1] += sse4_sum_epu64(counter_c);
    flag_counts[2] += sse4_sum_epu64(counter_e);
    flag_counts[3] += sse4_sum_epu64(counter_g);
    flag_counts[4] += sse4_sum_epu64(counter_b) >> 4;
    flag_counts[5] += sse4_sum_epu64(counter_d) >> 4;
    flag_counts[6] += sse4_sum_epu64(counter_f) >> 4;
    flag_counts[7] += sse4_sum_epu64(counter_h) >> 4;

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % 64, flag_counts);
}

void pospopcnt_u32_sse_harley_seal(const uint32_t* array, size_t len, uint32_t* flags) {
    for (size_t i = len - (len % (16 * 4)); i < len; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m128i* data = (const __m128i*)array;
    size_t size = len / 4;
    __m128i v1  = _mm_setzero_si128();
    __m128i v2  = _mm_setzero_si128();
    __m128i v4  = _mm_setzero_si128();
    __m128i v8  = _mm_setzero_si128();
    __m128i v16 = _mm_setzero_si128();
    __m128i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint32_t buffer[4];
    __m128i counter[32];

    while (i < limit) {
        for (size_t i = 0; i < 32; ++i) {
            counter[i] = _mm_setzero_si128();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm_add_epi32(counter[pos], _mm_and_si128(v16, _mm_set1_epi32(1))); \
    v16 = _mm_srli_epi32(v16, 1); \
}
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  0), _mm_loadu_si128(data + i +  1));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  2), _mm_loadu_si128(data + i +  3));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  4), _mm_loadu_si128(data + i +  5));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  6), _mm_loadu_si128(data + i +  7));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  8),  _mm_loadu_si128(data + i +  9));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 10),  _mm_loadu_si128(data + i + 11));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i + 12),  _mm_loadu_si128(data + i + 13));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 14),  _mm_loadu_si128(data + i + 15));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsB,&v4, foursA, foursB);
            U( 0) U( 1) U( 2) U( 3) U( 4) U( 5) U( 6) U( 7) U( 8) U( 9) U(10) U(11) U(12) U(13) U(14) U(15)
            U(16) U(17) U(18) U(19) U(20) U(21) U(22) U(23) U(24) U(25) U(26) U(27) U(28) U(29) U(30) U(31)
            pospopcnt_csa_sse(&v16,    &v8, eightsA, eightsB);
#undef U
        }

        // update the counters after the last iteration
        for (size_t i = 0; i < 32; ++i) {
            counter[i] = _mm_add_epi32(counter[i], _mm_and_si128(v16, _mm_set1_epi32(1)));
            v16 = _mm_srli_epi32(v16, 1);
        }

        for (size_t i = 0; i < 32; ++i) {
            _mm_storeu_si128((__m128i*)buffer, counter[i]);
            for (size_t z = 0; z < 4; z++) {
                flags[i] += 16 * buffer[z];
            }
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v1);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v2);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v4);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v8);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
}


/*
    Improved Harley-Seal implementation
    --------------------------------------------------

    The most time consuming step in HS for 32-bit pospopcnt is accumulating
    the v16 vector (which holdsthe MSBs of results). In the original procedure
    we have a simple 32-iteration loop which requires 32 extra registers
    ('counters'). Each iteration executes three instructions: bit-and, add
    and shift right --- in total 96 instructions.

    This step can be simplified and fewer counters are needed. In case of SSE
    code we start with 4 x 32-bit words. Let's call them a, b, c and d.

    in = [d31|d30|...| d2| d1|d0|c31|c30|...| c2| c1|c0|b31|b30|...| b2| b1|b0|a31|a30|...| a3| a2|a1]
         |      dword d         |       dword c        |    dword b        |        dword a          |
    
    Our goal is to calculate sum of four bits a[i] + b[i] + c[i] + d[i] for i = 0..31.


    1. First we gather in separate words bytes holding bits 0..7, 8..15, 16..23 and
       24..31 (below is the layout of the 0th dword):

    t0 = [d7|d6|d5|d4|d3|d2|d1|d0|c7|c6|c5|c4|c3|c2|c1|c0|b7|b6|b5|b4|b3|b2|b1|b0|a7|a6|a5|a4|a3|a2|a1|a0] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |
         |                                            dword 0                                            |
    
    t0 = _mm_shuffle_epi8(in, lookup);

    2. Group together even and odd bits ('..' = zero, for better readability in ASCII).

    m0 = [..|d6|..|d4|..|d2|..|d0|..|c6|..|c4|..|c2|..|c0|..|b6|..|b4|..|b2|..|b0|..|a6|..|a4|..|a2|..|a0] x 4
    m1 = [..|d7|..|d5|..|d3|..|d1|..|c7|..|c5|..|c3|..|c1|..|b7|..|b5|..|b3|..|b1|..|a7|..|a5|..|a3|..|a1] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |

    m0 = _mm_and_si128(t0, _mm_set1_epi8(0x55));
    m1 = _mm_and_si128(_mm_srli_epi32(t0, 1), _mm_set1_epi8(0x55));

    3. Then we add bits from **adjacent** bytes. These adjacent bytes have got corresponding
       bits from input pairs a-b and c-d. The results are 2-bit sums.

    s0 = [..|..|..|..|..|..|..|..|d6+c6|d4+c4|d2+c2|d0+c0|..|..|..|..|..|..|..|..|b6+a6|b4+a4|b0+a2|b0+a0] x 4
    s1 = [..|..|..|..|..|..|..|..|d7+c7|d5+c5|d3+c3|d1+c1|..|..|..|..|..|..|..|..|b7+a7|b5+a5|b3+a3|b1+a1] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |

    s0 = _mm_maddubs_epi16(m0, _mm_set1_epi16(0x0101));
    s1 = _mm_maddubs_epi16(m1, _mm_set1_epi16(0x0101));

    4. Please note that the odd bytes of s0 and s1 are zero. We marge these vectors:

    t1 = [d7+c7|d5+c5|d3+c3|d1+c1|d6+c6|d4+c4|d2+c2|d0+c0|b7+a7|b5+a5|b3+a3|b1+a1|b6+a6|b4+a4|b0+a2|b0+a0] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |

    __m128i t1 = _mm_or_si128(s0, _mm_slli_epi32(s1, 8));

    5. Similarly to summing 1-bit inputs, now we're going to add 2-bit sums creating
       a vector of 4-bit sums. Again we split & mask the vector to make room for 
       4-bit results.

    n0 = [..|..|d5+c5|..|..|d1+c1|..|..|d4+c4|..|..|d0+c0|..|..|b5+a5|..|..|b1+a1|..|..|b4+a4|..|..|b0+a0] x 4
    n1 = [..|..|d7+c7|..|..|d3+c3|..|..|d6+c6|..|..|d2+c2|..|..|b7+a7|..|..|b3+a3|..|..|b6+a6|..|..|b0+a2] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |

    n0 = _mm_and_si128(t1, _mm_set1_epi8(0x33));
    n1 = _mm_and_si128(_mm_srli_epi32(t1, 2), _mm_set1_epi8(0x33));

    6. Now we can add the partial sums using the 16-bit multiply-add instruction.

    S0 = [..|..|..|..|..|..|..|..|..|..|..|..|..|..|..|..|d5+c5|b5+a5|d1+c1|b1+a1|d4+c4|b4+a4|d0+c0+b0+a0] x 4
    S1 = [..|..|..|..|..|..|..|..|..|..|..|..|..|..|..|..|d7+c7|b7+a7|d3+c3|b3+a3|d6+c6|b6+a6|d2+c2+b0+a2] x 4
         |        byte 3         |         byte 2        |         byte 1        |         byte 0        |

    S0 = _mm_madd_epi16(n0, _mm_set1_epi32(0x00010001));
    S1 = _mm_madd_epi16(n1, _mm_set1_epi32(0x00010001));

    7. At this point we have four 4-bit sums in lower 16-bits of each 32-bit word.
       The order of fields in not linear, but this an issue when updating in-memory
       histogram. No more vectorization opportunities at this point, just bare
       bit-ands and shifts. 

    mask = _mm_set1_epi32(0x0000000f);
    counter[0] = _mm_add_epi32(counter[0], _mm_and_si128(S0, mask));
    counter[1] = _mm_add_epi32(counter[1], _mm_and_si128(_mm_srli_epi32(S0,  4), mask));
    counter[2] = _mm_add_epi32(counter[2], _mm_and_si128(_mm_srli_epi32(S0,  8), mask));
    counter[3] = _mm_add_epi32(counter[3], _mm_and_si128(_mm_srli_epi32(S0, 12), mask));
    counter[4] = _mm_add_epi32(counter[4], _mm_and_si128(S1, mask));
    counter[5] = _mm_add_epi32(counter[5], _mm_and_si128(_mm_srli_epi32(S1,  4), mask));
    counter[6] = _mm_add_epi32(counter[6], _mm_and_si128(_mm_srli_epi32(S1,  8), mask));
    counter[7] = _mm_add_epi32(counter[7], _mm_and_si128(_mm_srli_epi32(S1, 12), mask));

    Summary:
    - decrease the number of auxiliary registers from 32 to 8 (4 fold)
    - decrease the number of instructions from 96 to 35 (~2.5 fold)
*/
void pospopcnt_u32_sse_harley_seal_improved(const uint32_t* array, size_t len, uint32_t* flags) {
    for (size_t i = len - (len % (16 * 4)); i < len; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m128i* data = (const __m128i*)array;
    size_t size = len / 4;
    __m128i v1  = _mm_setzero_si128();
    __m128i v2  = _mm_setzero_si128();
    __m128i v4  = _mm_setzero_si128();
    __m128i v8  = _mm_setzero_si128();
    __m128i v16 = _mm_setzero_si128();
    __m128i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const uint64_t limit = size - size % 16;
    uint64_t i = 0;
    uint32_t buffer[4];
    __m128i counter[8];

    const __m128i lookup = _mm_setr_epi8( 0,  4,  8, 12,
                                          1,  5,  9, 13,
                                          2,  6, 10, 14,
                                          3,  7, 11, 15);

    while (i < limit) {
        for (size_t i = 0; i < 8; ++i) {
            counter[i] = _mm_setzero_si128();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  0), _mm_loadu_si128(data + i +  1));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  2), _mm_loadu_si128(data + i +  3));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  4), _mm_loadu_si128(data + i +  5));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i +  6), _mm_loadu_si128(data + i +  7));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i +  8),  _mm_loadu_si128(data + i +  9));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 10),  _mm_loadu_si128(data + i + 11));
            pospopcnt_csa_sse(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_sse(&twosA,  &v1, _mm_loadu_si128(data + i + 12),  _mm_loadu_si128(data + i + 13));
            pospopcnt_csa_sse(&twosB,  &v1, _mm_loadu_si128(data + i + 14),  _mm_loadu_si128(data + i + 15));
            pospopcnt_csa_sse(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_sse(&eightsB,&v4, foursA, foursB);

            {
                const __m128i t0 = _mm_shuffle_epi8(v16, lookup);
                const __m128i m0 = _mm_and_si128(t0, _mm_set1_epi8(0x55));
                const __m128i m1 = _mm_and_si128(_mm_srli_epi32(t0, 1), _mm_set1_epi8(0x55));

                const __m128i s0 = _mm_maddubs_epi16(m0, _mm_set1_epi16(0x0101));
                const __m128i s1 = _mm_maddubs_epi16(m1, _mm_set1_epi16(0x0101));

                const __m128i t1 = _mm_or_si128(s0, _mm_slli_epi32(s1, 8));

                const __m128i n0 = _mm_and_si128(t1, _mm_set1_epi8(0x33));
                const __m128i n1 = _mm_and_si128(_mm_srli_epi32(t1, 2), _mm_set1_epi8(0x33));

                const __m128i S0 = _mm_madd_epi16(n0, _mm_set1_epi32(0x00010001));
                const __m128i S1 = _mm_madd_epi16(n1, _mm_set1_epi32(0x00010001));

                const __m128i mask = _mm_set1_epi32(0x0000000f);
                counter[0] = _mm_add_epi32(counter[0], _mm_and_si128(S0, mask));
                counter[1] = _mm_add_epi32(counter[1], _mm_and_si128(_mm_srli_epi32(S0,  4), mask));
                counter[2] = _mm_add_epi32(counter[2], _mm_and_si128(_mm_srli_epi32(S0,  8), mask));
                counter[3] = _mm_add_epi32(counter[3], _mm_and_si128(_mm_srli_epi32(S0, 12), mask));
                counter[4] = _mm_add_epi32(counter[4], _mm_and_si128(S1, mask));
                counter[5] = _mm_add_epi32(counter[5], _mm_and_si128(_mm_srli_epi32(S1,  4), mask));
                counter[6] = _mm_add_epi32(counter[6], _mm_and_si128(_mm_srli_epi32(S1,  8), mask));
                counter[7] = _mm_add_epi32(counter[7], _mm_and_si128(_mm_srli_epi32(S1, 12), mask));
            }

            pospopcnt_csa_sse(&v16,    &v8, eightsA, eightsB);
        }

        // update the counters after the last iteration
        {
            const __m128i t0 = _mm_shuffle_epi8(v16, lookup);

            const __m128i m0 = _mm_and_si128(t0, _mm_set1_epi8(0x55));
            const __m128i m1 = _mm_and_si128(_mm_srli_epi32(t0, 1), _mm_set1_epi8(0x55));

            const __m128i s0 = _mm_maddubs_epi16(m0, _mm_set1_epi16(0x0101));
            const __m128i s1 = _mm_maddubs_epi16(m1, _mm_set1_epi16(0x0101));

            const __m128i t1 = _mm_or_si128(s0, _mm_slli_epi32(s1, 8));

            const __m128i n0 = _mm_and_si128(t1, _mm_set1_epi8(0x33));
            const __m128i n1 = _mm_and_si128(_mm_srli_epi32(t1, 2), _mm_set1_epi8(0x33));

            const __m128i S0 = _mm_madd_epi16(n0, _mm_set1_epi32(0x00010001));
            const __m128i S1 = _mm_madd_epi16(n1, _mm_set1_epi32(0x00010001));

            const __m128i mask = _mm_set1_epi32(0x0000000f);
            counter[0] = _mm_add_epi32(counter[0], _mm_and_si128(S0, mask));
            counter[1] = _mm_add_epi32(counter[1], _mm_and_si128(_mm_srli_epi32(S0,  4), mask));
            counter[2] = _mm_add_epi32(counter[2], _mm_and_si128(_mm_srli_epi32(S0,  8), mask));
            counter[3] = _mm_add_epi32(counter[3], _mm_and_si128(_mm_srli_epi32(S0, 12), mask));
            counter[4] = _mm_add_epi32(counter[4], _mm_and_si128(S1, mask));
            counter[5] = _mm_add_epi32(counter[5], _mm_and_si128(_mm_srli_epi32(S1,  4), mask));
            counter[6] = _mm_add_epi32(counter[6], _mm_and_si128(_mm_srli_epi32(S1,  8), mask));
            counter[7] = _mm_add_epi32(counter[7], _mm_and_si128(_mm_srli_epi32(S1, 12), mask));

#define update_flags(counterN, index0, index1, index2, index3)      \
            _mm_storeu_si128((__m128i*)buffer, _mm_slli_epi32(counterN, 4));           \
            flags[index0] += buffer[0]; flags[index1] += buffer[1]; \
            flags[index2] += buffer[2]; flags[index3] += buffer[3];

            update_flags(counter[0], 0,  8, 16, 24);
            update_flags(counter[1], 4, 12, 20, 28);
            update_flags(counter[2], 1,  9, 17, 25);
            update_flags(counter[3], 5, 13, 21, 29);
            update_flags(counter[4], 2, 10, 18, 26);
            update_flags(counter[5], 6, 14, 22, 30);
            update_flags(counter[6], 3, 11, 19, 27);
            update_flags(counter[7], 7, 15, 23, 31);
#undef update_flags
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v1);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm_storeu_si128((__m128i*)buffer, v2);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v4);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    _mm_storeu_si128((__m128i*)buffer, v8);
    for (size_t i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_blend_popcnt, pospopcnt_u16_sse_blend_popcnt)
make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_blend_popcnt_unroll4, pospopcnt_u16_sse_blend_popcnt_unroll4)
make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_blend_popcnt_unroll8, pospopcnt_u16_sse_blend_popcnt_unroll8)
make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_blend_popcnt_unroll16, pospopcnt_u16_sse_blend_popcnt_unroll16)
make_pospopcnt_u8_from_u16(pospopcnt_u8_sse_harley_seal, pospopcnt_u16_sse_harley_seal)
#else
pospopcnt_u16_stub(pospopcnt_u16_sse_blend_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_sse_blend_popcnt_unroll4)
pospopcnt_u16_stub(pospopcnt_u16_sse_blend_popcnt_unroll8)
pospopcnt_u16_stub(pospopcnt_u16_sse_blend_popcnt_unroll16)
pospopcnt_u16_stub(pospopcnt_u16_sse_harley_seal)
pospopcnt_u16_stub(pospopcnt_u16_sse_harley_seal_improved)
pospopcnt_u8_stub(pospopcnt_u8_sse_blend_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_sse_blend_popcnt_unroll4)
pospopcnt_u8_stub(pospopcnt_u8_sse_blend_popcnt_unroll8)
pospopcnt_u8_stub(pospopcnt_u8_sse_blend_popcnt_unroll16)
pospopcnt_u8_stub(pospopcnt_u8_sse_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_sse_popcnt4bit)
pospopcnt_u8_stub(pospopcnt_u8_sse_horizreduce)
pospopcnt_u32_stub(pospopcnt_u32_sse_harley_seal)
pospopcnt_u32_stub(pospopcnt_u32_sse_harley_seal_improved)
#endif

#if POSPOPCNT_SIMD_VERSION >= 6
#define AND_OR 0xea // ternary function: (a & b) | c
#if defined(__AVX512BW__) && __AVX512BW__ == 1
int pospopcnt_u16_avx512bw_blend_popcnt(const uint16_t* data, uint32_t len, uint32_t* flags) { 
    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

    size_t i = 0;
    for (/**/; i + 2 <= n_cycles; i += 2) {
        __m512i v0 = _mm512_loadu_si512(data_vectors + i + 0);
        __m512i v1 = _mm512_loadu_si512(data_vectors + i + 1);

        __m512i input0 = _mm512_ternarylogic_epi32(v0, _mm512_set1_epi16(0x00FF), _mm512_slli_epi16(v1, 8), AND_OR);
        __m512i input1 = _mm512_ternarylogic_epi32(v0, _mm512_set1_epi16((int16_t)0xFF00), _mm512_srli_epi16(v1, 8), AND_OR);
        
        for (int i = 0; i < 8; ++i) {
            flags[ 7 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input0));
            flags[15 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input1));
            input0 = _mm512_add_epi8(input0, input0);
            input1 = _mm512_add_epi8(input1, input1);
        }
    }

    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

    return 0;
}

int pospopcnt_u16_avx512bw_blend_popcnt_unroll4(const uint16_t* data, uint32_t len, uint32_t* flags) { 
    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

    size_t i = 0;
    for (/**/; i + 4 <= n_cycles; i += 4) {
#define L(p) __m512i v##p = _mm512_loadu_si512(data_vectors+i+p);
        L(0) L(1) L(2) L(3)

#define U0(p,k) __m512i input##p = _mm512_ternarylogic_epi32(v##p, _mm512_set1_epi16(0x00FF), _mm512_slli_epi16(v##k, 8), AND_OR);
#define U1(p,k) __m512i input##k = _mm512_ternarylogic_epi32(v##p, _mm512_set1_epi16((int16_t)0xFF00), _mm512_srli_epi16(v##k, 8), AND_OR);
#define U(p, k)  U0(p,k) U1(p,k)

        U(0,1) U(2,3)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3)

#define P0(p) input##p = _mm512_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P
    
    return 0;
}

int pospopcnt_u16_avx512bw_blend_popcnt_unroll8(const uint16_t* data, uint32_t len, uint32_t* flags) { 
    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

    size_t i = 0;
    for (/**/; i + 8 <= n_cycles; i += 8) {
#define L(p) __m512i v##p = _mm512_loadu_si512(data_vectors+i+p);
        L(0)  L(1)  L(2)  L(3)  
        L(4)  L(5)  L(6)  L(7) 

#define U0(p,k) __m512i input##p = _mm512_ternarylogic_epi32(v##p, _mm512_set1_epi16(0x00FF), _mm512_slli_epi16(v##k, 8), AND_OR);
#define U1(p,k) __m512i input##k = _mm512_ternarylogic_epi32(v##p, _mm512_set1_epi16((int16_t)0xFF00), _mm512_srli_epi16(v##k, 8), AND_OR);
#define U(p, k)  U0(p,k) U1(p,k)

        U(0,1) U( 2, 3) U( 4, 5) U( 6, 7)
        
        for (int i = 0; i < 8; ++i) {
#define A0(p) flags[ 7 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input##p));
#define A1(k) flags[15 - i] += _mm_popcnt_u64(_mm512_movepi8_mask(input##k));
#define A(p, k) A0(p) A1(k)
            A(0,1) A(2, 3) A(4,5) A(6, 7)

#define P0(p) input##p = _mm512_add_epi8(input##p, input##p);
#define P(p, k) P0(p) P0(k)
            P(0,1) P(2, 3) P(4,5) P(6, 7)
        }
    }

    for (/**/; i + 4 <= n_cycles; i += 4) {
        L(0) L(1) L(2) L(3)
        U(0,1) U(2,3)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1) A(2, 3)
            P(0,1) P(2, 3)
        }
    }

    for (/**/; i + 2 <= n_cycles; i += 2) {
        L(0) L(1)
        U(0,1)
        
        for (int i = 0; i < 8; ++i) {
            A(0,1)
            P(0,1)
        }
    }

    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef U0
#undef U1
#undef U
#undef A0
#undef A1
#undef A
#undef P0
#undef P
    
    return 0;
}

static
__m512i avx512_merge1_odd(__m512i a, __m512i b) {
    const __m512i t0 = a & _mm512_set1_epi8((int8_t)0xaa);
    const __m512i t1 = b & _mm512_set1_epi8((int8_t)0xaa);

    return t0 | (_mm512_srli_epi32(t1, 1));
}

static
__m512i avx512_merge1_even(__m512i a, __m512i b) {
    const __m512i t0 = a & _mm512_set1_epi8(0x55);
    const __m512i t1 = b & _mm512_set1_epi8(0x55);

    return t0 | (_mm512_add_epi8(t1, t1));
}

static
__m512i avx512_merge2_odd(__m512i a, __m512i b) {
    const __m512i t0 = a & _mm512_set1_epi8((int8_t)0xcc);
    const __m512i t1 = b & _mm512_set1_epi8((int8_t)0xcc);

    return t0 | (_mm512_srli_epi32(t1, 2));
}

static
__m512i avx512_merge2_even(__m512i a, __m512i b) {
    const __m512i t0 = a & _mm512_set1_epi8(0x33);
    const __m512i t1 = b & _mm512_set1_epi8(0x33);

    return t0 | (_mm512_slli_epi32(t1, 2));
}

static
uint64_t avx512_sum_epu64(__m512i x) {
    return avx2_sum_epu64(_mm512_extracti64x4_epi64(x, 0))
         + avx2_sum_epu64(_mm512_extracti64x4_epi64(x, 1));
}

void pospopcnt_u8_avx512bw_popcnt4bit(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m512i zero = _mm512_setzero_si512();

    __m512i counter_a = zero;
    __m512i counter_b = zero;
    __m512i counter_c = zero;
    __m512i counter_d = zero;
    __m512i counter_e = zero;
    __m512i counter_f = zero;
    __m512i counter_g = zero;
    __m512i counter_h = zero;

    const __m512i popcnt_4bit = _mm512_setr_epi64(
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu
    );

    const __m512i lo_nibble = _mm512_set1_epi8(0x0f);

    int local = 0;
    __m512i counter8bit_a = zero;
    __m512i counter8bit_b = zero;
    __m512i counter8bit_c = zero;
    __m512i counter8bit_d = zero;
    __m512i counter8bit_e = zero;
    __m512i counter8bit_f = zero;
    __m512i counter8bit_g = zero;
    __m512i counter8bit_h = zero;

    for (const uint8_t* end = &data[len & ~(4*64 - 1)]; data != end; data += 4*64) {
        // r0 = [a0|b0|c0|d0|e0|f0|g0|h0]
        // r1 = [a1|b1|c1|d1|e1|f1|g1|h1]
        // r2 = [a2|b2|c2|d2|e2|f2|g2|h2]
        // r3 = [a3|b3|c3|d3|e3|f3|g3|h3]
        const __m512i r0 = _mm512_loadu_si512((__m512i*)&data[0*64]);
        const __m512i r1 = _mm512_loadu_si512((__m512i*)&data[1*64]);
        const __m512i r2 = _mm512_loadu_si512((__m512i*)&data[2*64]);
        const __m512i r3 = _mm512_loadu_si512((__m512i*)&data[3*64]);

        // s0 = [a0|a1|c0|c1|e0|e1|g0|g1]
        // s1 = [b0|b1|d0|d1|f0|f1|h0|h1]
        // s2 = [a2|a3|c2|c3|e2|e3|g2|g3]
        // s3 = [b2|b3|d2|d3|f2|f3|h2|h3]
        const __m512i s0 = avx512_merge1_even(r0, r1);
        const __m512i s1 = avx512_merge1_odd (r0, r1);
        const __m512i s2 = avx512_merge1_even(r2, r3);
        const __m512i s3 = avx512_merge1_odd (r2, r3);

        // d0 = [a0|a1|a2|a3|e0|e1|e2|e3]
        // d1 = [b0|b1|b2|b3|f0|f1|f2|f3]
        // d2 = [c0|c1|c2|c3|g0|g1|g2|g3]
        // d3 = [d0|d1|d2|d3|h0|h1|h2|h3]
        const __m512i d0 = avx512_merge2_even(s0, s2);
        const __m512i d1 = avx512_merge2_even(s1, s3);
        const __m512i d2 = avx512_merge2_odd (s0, s2);
        const __m512i d3 = avx512_merge2_odd (s1, s3);

        // popcnt for 4-bit subwords in each registers
        const __m512i popcnt_a = _mm512_shuffle_epi8(popcnt_4bit, d0 & lo_nibble);
        const __m512i popcnt_e = _mm512_shuffle_epi8(popcnt_4bit, _mm512_srli_epi32(d0, 4) & lo_nibble);
        const __m512i popcnt_b = _mm512_shuffle_epi8(popcnt_4bit, d1 & lo_nibble);
        const __m512i popcnt_f = _mm512_shuffle_epi8(popcnt_4bit, _mm512_srli_epi32(d1, 4) & lo_nibble);
        const __m512i popcnt_c = _mm512_shuffle_epi8(popcnt_4bit, d2 & lo_nibble);
        const __m512i popcnt_g = _mm512_shuffle_epi8(popcnt_4bit, _mm512_srli_epi32(d2, 4) & lo_nibble);
        const __m512i popcnt_d = _mm512_shuffle_epi8(popcnt_4bit, d3 & lo_nibble);
        const __m512i popcnt_h = _mm512_shuffle_epi8(popcnt_4bit, _mm512_srli_epi32(d3, 4) & lo_nibble);

        counter8bit_a = _mm512_add_epi8(counter8bit_a, popcnt_a);
        counter8bit_b = _mm512_add_epi8(counter8bit_b, popcnt_b);
        counter8bit_c = _mm512_add_epi8(counter8bit_c, popcnt_c);
        counter8bit_d = _mm512_add_epi8(counter8bit_d, popcnt_d);
        counter8bit_e = _mm512_add_epi8(counter8bit_e, popcnt_e);
        counter8bit_f = _mm512_add_epi8(counter8bit_f, popcnt_f);
        counter8bit_g = _mm512_add_epi8(counter8bit_g, popcnt_g);
        counter8bit_h = _mm512_add_epi8(counter8bit_h, popcnt_h);

        local += 1;
        if (local == 63) {
            // avoid overflows in the 8-bit counters
#define U(n) \
            counter_##n = _mm512_add_epi64(counter_##n, _mm512_sad_epu8(counter8bit_##n, zero)); \
            counter8bit_##n = _mm512_setzero_si512();

            U(a) U(b) U(c) U(d)
            U(e) U(f) U(g) U(h)
#undef U
            local = 0;
        }
    }

    if (local != 0) {
#define U(n) counter_##n = _mm512_add_epi64(counter_##n, _mm512_sad_epu8(counter8bit_##n, zero));
        U(a) U(b) U(c) U(d)
        U(e) U(f) U(g) U(h)
#undef U
    }

    flag_counts[0] += avx512_sum_epu64(counter_a);
    flag_counts[1] += avx512_sum_epu64(counter_b);
    flag_counts[2] += avx512_sum_epu64(counter_c);
    flag_counts[3] += avx512_sum_epu64(counter_d);
    flag_counts[4] += avx512_sum_epu64(counter_e);
    flag_counts[5] += avx512_sum_epu64(counter_f);
    flag_counts[6] += avx512_sum_epu64(counter_g);
    flag_counts[7] += avx512_sum_epu64(counter_h);

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % (4*64), flag_counts);
}

#define qword(x0, x1, x2, x3, x4, x5, x6, x7) \
   (((uint64_t)(x0) << (0*8)) | \
    ((uint64_t)(x1) << (1*8)) | \
    ((uint64_t)(x2) << (2*8)) | \
    ((uint64_t)(x3) << (3*8)) | \
    ((uint64_t)(x4) << (4*8)) | \
    ((uint64_t)(x5) << (5*8)) | \
    ((uint64_t)(x6) << (6*8)) | \
    ((uint64_t)(x7) << (7*8)))

#define _mm512_setr_epi8( \
    a0, a1, a2, a3, a4, a5, a6, a7, \
    b0, b1, b2, b3, b4, b5, b6, b7, \
    c0, c1, c2, c3, c4, c5, c6, c7, \
    d0, d1, d2, d3, d4, d5, d6, d7, \
    e0, e1, e2, e3, e4, e5, e6, e7, \
    f0, f1, f2, f3, f4, f5, f6, f7, \
    g0, g1, g2, g3, g4, g5, g6, g7, \
    h0, h1, h2, h3, h4, h5, h6, h7) \
    _mm512_setr_epi64( \
        qword(a0, a1, a2, a3, a4, a5, a6, a7),\
        qword(b0, b1, b2, b3, b4, b5, b6, b7),\
        qword(c0, c1, c2, c3, c4, c5, c6, c7),\
        qword(d0, d1, d2, d3, d4, d5, d6, d7),\
        qword(e0, e1, e2, e3, e4, e5, e6, e7),\
        qword(f0, f1, f2, f3, f4, f5, f6, f7),\
        qword(g0, g1, g2, g3, g4, g5, g6, g7),\
        qword(h0, h1, h2, h3, h4, h5, h6, h7))

void pospopcnt_u8_avx512bw_sadbw(const uint8_t* data, size_t len, uint32_t* flag_counts) {
    const __m512i zero = _mm512_setzero_si512();

    __m512i counts = zero;

    for (const uint8_t* end = &data[len & ~(64 - 1)]; data != end; data += 64) {
        const __m512i in = _mm512_loadu_si512((__m512i*)data);

        // in = [a0|a1|a2|a3|a4|a5|a6|a7|.............|n0|n1|n2|n3|n4|n5|n6|n7]
        //      |       byte 63         | bytes 1..62 |        byte 0         ]

        // 1. Isolate pairs of bits (0, 4), (1, 5), (2, 6), (3, 7) and store
        //    them in separate nibbles.
        const __m512i mask = _mm512_set1_epi8(0x11);
        const __m512i t04  = _mm512_and_si512(in, mask);
        const __m512i t15  = _mm512_and_si512(_mm512_srli_epi32(in, 1), mask);
        const __m512i t26  = _mm512_and_si512(_mm512_srli_epi32(in, 2), mask);
        const __m512i t37  = _mm512_and_si512(_mm512_srli_epi32(in, 3), mask);

        // 2. Count these bits in parallel. Each qword in the result vector
        //    has in the lowest byte the counts of 8 bits. The counts are
        //    stored in byte's nibbles.
        const __m512i sum04 = _mm512_sad_epu8(t04, zero);
        const __m512i sum15 = _mm512_sad_epu8(t15, zero);
        const __m512i sum26 = _mm512_sad_epu8(t26, zero);
        const __m512i sum37 = _mm512_sad_epu8(t37, zero);

        // 3. Group these non-zero bytes in separate qwords. Each qword will hold
        //    counters for given group
#define lo(idx) idx
#define hi(idx) (idx + 64)
#define any     0xff
        const __m512i group0_idx = _mm512_setr_epi8(
            /* qword 0 */   lo(0), lo(8), lo(16), lo(24), lo(32), lo(40), lo(48), lo(56), // sum04
            /* qword 1 */   hi(0), hi(8), hi(16), hi(24), hi(32), hi(40), hi(48), hi(56), // sum15
            /* qword 2 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 3 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 4 */   lo(0), lo(8), lo(16), lo(24), lo(32), lo(40), lo(48), lo(56), // sum04
            /* qword 5 */   hi(0), hi(8), hi(16), hi(24), hi(32), hi(40), hi(48), hi(56), // sum15
            /* qword 6 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 7 */   any,   any,   any,     any,   any,    any,    any,    any);
        const __m512i g0 = _mm512_permutex2var_epi8(sum04, group0_idx, sum15);

        const __m512i group1_idx = _mm512_setr_epi8(
            /* qword 0 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 1 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 2 */   lo(0), lo(8), lo(16), lo(24), lo(32), lo(40), lo(48), lo(56),  // sum26
            /* qword 3 */   hi(0), hi(8), hi(16), hi(24), hi(32), hi(40), hi(48), hi(56),  // sum37
            /* qword 4 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 5 */   any,   any,   any,     any,   any,    any,    any,    any,
            /* qword 6 */   lo(0), lo(8), lo(16), lo(24), lo(32), lo(40), lo(48), lo(56),  // sum26
            /* qword 7 */   hi(0), hi(8), hi(16), hi(24), hi(32), hi(40), hi(48), hi(56)); // sum37
        const __m512i g1 = _mm512_permutex2var_epi8(sum26, group1_idx, sum37);
#undef any
#undef lo
#undef hi
        // 4. Merge the counters
        const __m512i merge_mask = _mm512_setr_epi64(-1, -1, 0, 0, -1, -1, 0, 0);
        const __m512i t0  = _mm512_ternarylogic_epi32(merge_mask, g0, g1, 0xca);
        
        // 5. Mask proper nibbles: lower in qwords 0..3, higher in qword 4..7
        const __m512i nibble_mask = _mm512_setr_epi64(
            0x0f0f0f0f0f0f0f0flu, 0x0f0f0f0f0f0f0f0flu, 0x0f0f0f0f0f0f0f0flu, 0x0f0f0f0f0f0f0f0flu,
            0xf0f0f0f0f0f0f0f0lu, 0xf0f0f0f0f0f0f0f0lu, 0xf0f0f0f0f0f0f0f0lu, 0xf0f0f0f0f0f0f0f0lu);

        const __m512i t1 = _mm512_and_si512(t0, nibble_mask);
        
        // 6. Sum counters for each bit. Please note that values in qword 4..7
        //    are multiplied by 16 (shifted right by 4 bits).
        const __m512i t2 = _mm512_sad_epu8(t1, zero);

        // 7. Update the vector of 64-bit counters
        counts = _mm512_add_epi64(counts, t2);
    }

    // save the counters
    uint64_t tmp[8];
    _mm512_storeu_si512(tmp, counts);
    for (int i=0; i < 4; i++) {
        flag_counts[i + 0] += (uint32_t)tmp[i + 0];
        flag_counts[i + 4] += (uint32_t)(tmp[i + 4] >> 4);
    }

    // scalar tail loop
    pospopcnt_u8_scalar_naive(data, len % (64), flag_counts);
}

#undef qword
#undef _mm512_setr_epi8

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_blend_popcnt, pospopcnt_u16_avx512bw_blend_popcnt)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_blend_popcnt_unroll4, pospopcnt_u16_avx512bw_blend_popcnt_unroll4)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_blend_popcnt_unroll8, pospopcnt_u16_avx512bw_blend_popcnt_unroll8)
#else 
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt_unroll4)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt_unroll8)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt_unroll4)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt_unroll8)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt4bit)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_sadbw)
#endif

int pospopcnt_u16_avx512_mula2(const uint16_t* data, uint32_t len, uint32_t* flags) {
    const __m512i* data_vectors = (const __m512i*)(data);
    const uint32_t n_cycles = len / 32;

    __m512i sum[16];
    for (int i=0; i < 16; ++i)
        sum[i] = _mm512_setzero_si512();

    size_t i = 0;
    for (/**/; i + 2 <= n_cycles; i += 2) {
        __m512i v0 = _mm512_loadu_si512(data_vectors + i);
        __m512i v1 = _mm512_loadu_si512(data_vectors + i + 1);

        __m512i input0 = _mm512_or_si512(_mm512_and_si512(v0, _mm512_set1_epi16(0x00FF)), _mm512_slli_epi16(v1, 8));
        __m512i input1 = _mm512_or_si512(_mm512_and_si512(v0, _mm512_set1_epi16((int16_t)0xFF00)), _mm512_srli_epi16(v1, 8));
        
        const __m512i bits_04 = _mm512_and_si512(input0, _mm512_set1_epi16(0x1111));
        const __m512i bits_15 = _mm512_and_si512(_mm512_srli_epi32(input0, 1), _mm512_set1_epi16(0x1111)); // 00010001 00010001
        const __m512i bits_26 = _mm512_and_si512(_mm512_srli_epi32(input0, 2), _mm512_set1_epi16(0x1111));
        const __m512i bits_37 = _mm512_and_si512(_mm512_srli_epi32(input0, 3), _mm512_set1_epi16(0x1111));

        const __m512i bits_8c = _mm512_and_si512(input1, _mm512_set1_epi16(0x1111));
        const __m512i bits_9d = _mm512_and_si512(_mm512_srli_epi32(input1, 1), _mm512_set1_epi16(0x1111));
        const __m512i bits_ae = _mm512_and_si512(_mm512_srli_epi32(input1, 2), _mm512_set1_epi16(0x1111));
        const __m512i bits_bf = _mm512_and_si512(_mm512_srli_epi32(input1, 3), _mm512_set1_epi16(0x1111));

        const __m512i sum_04 = _mm512_sad_epu8(bits_04, _mm512_setzero_si512());
        const __m512i sum_15 = _mm512_sad_epu8(bits_15, _mm512_setzero_si512());
        const __m512i sum_26 = _mm512_sad_epu8(bits_26, _mm512_setzero_si512());
        const __m512i sum_37 = _mm512_sad_epu8(bits_37, _mm512_setzero_si512());

        const __m512i sum_8c = _mm512_sad_epu8(bits_8c, _mm512_setzero_si512());
        const __m512i sum_9d = _mm512_sad_epu8(bits_9d, _mm512_setzero_si512());
        const __m512i sum_ae = _mm512_sad_epu8(bits_ae, _mm512_setzero_si512());
        const __m512i sum_bf = _mm512_sad_epu8(bits_bf, _mm512_setzero_si512());

        sum[0x0] = _mm512_add_epi64(sum[0x0], _mm512_and_si512 (sum_04, _mm512_set1_epi64(0xF))); // 00001111
        sum[0x4] = _mm512_add_epi64(sum[0x4], _mm512_srli_epi32(sum_04, 4));
        sum[0x1] = _mm512_add_epi64(sum[0x1], _mm512_and_si512 (sum_15, _mm512_set1_epi64(0xF)));
        sum[0x5] = _mm512_add_epi64(sum[0x5], _mm512_srli_epi32(sum_15, 4));
        sum[0x2] = _mm512_add_epi64(sum[0x2], _mm512_and_si512 (sum_26, _mm512_set1_epi64(0xF)));
        sum[0x6] = _mm512_add_epi64(sum[0x6], _mm512_srli_epi32(sum_26, 4));
        sum[0x3] = _mm512_add_epi64(sum[0x3], _mm512_and_si512 (sum_37, _mm512_set1_epi64(0xF)));
        sum[0x7] = _mm512_add_epi64(sum[0x7], _mm512_srli_epi32(sum_37, 4));
        sum[0x8] = _mm512_add_epi64(sum[0x8], _mm512_and_si512 (sum_8c, _mm512_set1_epi64(0xF)));
        sum[0xc] = _mm512_add_epi64(sum[0xc], _mm512_srli_epi32(sum_8c, 4));
        sum[0x9] = _mm512_add_epi64(sum[0x9], _mm512_and_si512 (sum_9d, _mm512_set1_epi64(0xF)));
        sum[0xd] = _mm512_add_epi64(sum[0xd], _mm512_srli_epi32(sum_9d, 4));
        sum[0xa] = _mm512_add_epi64(sum[0xa], _mm512_and_si512 (sum_ae, _mm512_set1_epi64(0xF)));
        sum[0xe] = _mm512_add_epi64(sum[0xe], _mm512_srli_epi32(sum_ae, 4));
        sum[0xb] = _mm512_add_epi64(sum[0xb], _mm512_and_si512 (sum_bf, _mm512_set1_epi64(0xF)));
        sum[0xf] = _mm512_add_epi64(sum[0xf], _mm512_srli_epi32(sum_bf, 4));
    }

    for (int j = 0; j < 16; ++j) {
        const __m128i dq0 = _mm512_extracti32x4_epi32(sum[j], 0);
        const __m128i dq1 = _mm512_extracti32x4_epi32(sum[j], 1);
        const __m128i dq2 = _mm512_extracti32x4_epi32(sum[j], 2);
        const __m128i dq3 = _mm512_extracti32x4_epi32(sum[j], 3);
        flags[j] = _mm_extract_epi64(dq0, 0)
                    + _mm_extract_epi64(dq0, 1)
                    + _mm_extract_epi64(dq1, 0)
                    + _mm_extract_epi64(dq1, 1)
                    + _mm_extract_epi64(dq2, 0)
                    + _mm_extract_epi64(dq2, 1)
                    + _mm_extract_epi64(dq3, 0)
                    + _mm_extract_epi64(dq3, 1);
    }

    i *= 32;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((data[i] & (1 << j)) >> j);
        }
    }

    return 0;
}

#if defined(__AVX512BW__) && __AVX512BW__ == 1
int pospopcnt_u16_avx512bw_adder_forest(const uint16_t* array, uint32_t len, uint32_t* flags) {
    __m512i counters[16];

    for (size_t i = 0; i < 16; ++i) {
        counters[i] = _mm512_setzero_si512();
    }

    const __m512i mask1bit = _mm512_set1_epi16(0x5555); // 0101010101010101 Pattern: 01
    const __m512i mask2bit = _mm512_set1_epi16(0x3333); // 0011001100110011 Pattern: 0011
    const __m512i mask4bit = _mm512_set1_epi16(0x0F0F); // 0000111100001111 Pattern: 00001111
    const __m512i mask8bit = _mm512_set1_epi16(0x00FF); // 0000000011111111 Pattern: 0000000011111111
    
    const uint32_t n_cycles = len / (2048 * (16*32));
    const uint32_t n_total  = len / (16*32);
    uint16_t tmp[32];

/*------ Macros --------*/
#define LE(i,p,k)  const __m512i sum##p##k##_##i##bit_even = _mm512_add_epi8(input##p & mask##i##bit, input##k & mask##i##bit);
#define LO(i,p,k)  const __m512i sum##p##k##_##i##bit_odd  = _mm512_add_epi8(_mm512_srli_epi16(input##p, i) & mask##i##bit, _mm512_srli_epi16(input##k, i) & mask##i##bit);

#define LBLOCK(i)           \
    LE(i,0,1)   LO(i,0,1)   \
    LE(i,2,3)   LO(i,2,3)   \
    LE(i,4,5)   LO(i,4,5)   \
    LE(i,6,7)   LO(i,6,7)   \
    LE(i,8,9)   LO(i,8,9)   \
    LE(i,10,11) LO(i,10,11) \
    LE(i,12,13) LO(i,12,13) \
    LE(i,14,15) LO(i,14,15) \

#define EVEN(b,i,k,p) input##i = sum##k##p##_##b##bit_even;
#define ODD(b,i,k,p)  input##i = sum##k##p##_##b##bit_odd;

#define UPDATE(i)                                                  \
    EVEN(i,0,0,1) EVEN(i,1,2,3)   EVEN(i,2,4,5)   EVEN(i,3,6,7)    \
    EVEN(i,4,8,9) EVEN(i,5,10,11) EVEN(i,6,12,13) EVEN(i,7,14,15)  \
     ODD(i,8,0,1)  ODD(i,9,2,3)    ODD(i,10,4,5)   ODD(i,11,6,7)   \
     ODD(i,12,8,9) ODD(i,13,10,11) ODD(i,14,12,13) ODD(i,15,14,15) \

#define UE(i,p,k) counters[i] = _mm512_add_epi16(counters[i], sum##p##k##_8bit_even);
#define UO(i,p,k) counters[i] = _mm512_add_epi16(counters[i], sum##p##k##_8bit_odd);

/*------ Start --------*/
#define L(p) __m512i input##p = _mm512_loadu_si512((__m512i*)(array + i*2048*512 + j*512 + p*32));
    size_t i = 0;
    for (/**/; i < n_cycles; ++i) {
        for (int j = 0; j < 2048; ++j) {
            // Load 16 registers.
            L(0)  L(1)  L(2)  L(3)  
            L(4)  L(5)  L(6)  L(7) 
            L(8)  L(9)  L(10) L(11) 
            L(12) L(13) L(14) L(15)

            // Perform updates for bits {1,2,4,8}.
            LBLOCK(1) UPDATE(1)
            LBLOCK(2) UPDATE(2)
            LBLOCK(4) UPDATE(4)
            LBLOCK(8) UPDATE(8)

            // Update accumulators.
            UE( 0,0,1) UE( 1, 2, 3) UE( 2, 4, 5) UE( 3, 6, 7)  
            UE( 4,8,9) UE( 5,10,11) UE( 6,12,13) UE( 7,14,15) 
            UO( 8,0,1) UO( 9, 2, 3) UO(10, 4, 5) UO(11, 6, 7) 
            UO(12,8,9) UO(13,10,11) UO(14,12,13) UO(15,14,15)
        }

        // Update.
        for (size_t i = 0; i < 16; ++i) {
            _mm512_storeu_si512((__m512i*)tmp, counters[i]);
            for (int j = 0; j < 32; ++j) flags[i] += tmp[j];
        }
        // Reset.
        for (size_t i = 0; i < 16; ++i) {
            counters[i] = _mm512_setzero_si512();
        }
    }
#undef L
#define L(p) __m512i input##p = _mm512_loadu_si512((__m512i*)(array + i*512 + p*32));
    i *= 2048;
    for (/**/; i < n_total; ++i) {
        // Load 16 registers.
        L(0)  L(1)  L(2)  L(3)  
        L(4)  L(5)  L(6)  L(7) 
        L(8)  L(9)  L(10) L(11) 
        L(12) L(13) L(14) L(15)

        // Perform updates for bits {1,2,4,8}.
        LBLOCK(1) UPDATE(1)
        LBLOCK(2) UPDATE(2)
        LBLOCK(4) UPDATE(4)
        LBLOCK(8) UPDATE(8)

        // Update accumulators.
        UE( 0,0,1) UE( 1, 2, 3) UE( 2, 4, 5) UE( 3, 6, 7)  
        UE( 4,8,9) UE( 5,10,11) UE( 6,12,13) UE( 7,14,15) 
        UO( 8,0,1) UO( 9, 2, 3) UO(10, 4, 5) UO(11, 6, 7) 
        UO(12,8,9) UO(13,10,11) UO(14,12,13) UO(15,14,15)
    }

    i *= 512;
    for (/**/; i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

#undef L
#undef UPDATE
#undef ODD
#undef EVEN
#undef LBLOCK
#undef LE
#undef LO
#undef UO
#undef UE

    for (size_t i = 0; i < 16; ++i) {
        _mm512_storeu_si512((__m512i*)tmp, counters[i]);
        for (int j = 0; j < 32; ++j) flags[i] += tmp[j];
    }
    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_adder_forest, pospopcnt_u16_avx512bw_adder_forest)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_adder_forest)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_adder_forest)
#endif

#if defined(__AVX512BW__) && __AVX512BW__ == 1
int pospopcnt_u16_avx512bw_harley_seal(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (32 * 16)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m512i* data = (const __m512i*)array;
    __m512i v1  = _mm512_setzero_si512();
    __m512i v2  = _mm512_setzero_si512();
    __m512i v4  = _mm512_setzero_si512();
    __m512i v8  = _mm512_setzero_si512();
    __m512i v16 = _mm512_setzero_si512();
    __m512i twosA, twosB, foursA, foursB, eightsA, eightsB;
    __m512i counter[16];

    const size_t size = len / 32;
    const uint64_t limit = size - size % 16;

    uint16_t buffer[32];

    uint64_t i = 0;
    while (i < limit) {
        for (size_t i = 0; i < 16; ++i)
            counter[i] = _mm512_setzero_si512();

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm512_add_epi16(counter[pos], _mm512_and_si512(v16, _mm512_set1_epi16(1))); \
    v16 = _mm512_srli_epi16(v16, 1); \
}
            pospopcnt_csa_avx512(&twosA,   &v1, _mm512_loadu_si512(data + i + 0), _mm512_loadu_si512(data + i + 1));
            pospopcnt_csa_avx512(&twosB,   &v1, _mm512_loadu_si512(data + i + 2), _mm512_loadu_si512(data + i + 3));
            pospopcnt_csa_avx512(&foursA,  &v2, twosA, twosB);
            pospopcnt_csa_avx512(&twosA,   &v1, _mm512_loadu_si512(data + i + 4), _mm512_loadu_si512(data + i + 5));
            pospopcnt_csa_avx512(&twosB,   &v1, _mm512_loadu_si512(data + i + 6), _mm512_loadu_si512(data + i + 7));
            pospopcnt_csa_avx512(&foursB,  &v2, twosA, twosB);
            pospopcnt_csa_avx512(&eightsA, &v4, foursA, foursB);
            pospopcnt_csa_avx512(&twosA,   &v1, _mm512_loadu_si512(data + i + 8),  _mm512_loadu_si512(data + i + 9));
            pospopcnt_csa_avx512(&twosB,   &v1, _mm512_loadu_si512(data + i + 10), _mm512_loadu_si512(data + i + 11));
            pospopcnt_csa_avx512(&foursA,  &v2, twosA, twosB);
            pospopcnt_csa_avx512(&twosA,   &v1, _mm512_loadu_si512(data + i + 12), _mm512_loadu_si512(data + i + 13));
            pospopcnt_csa_avx512(&twosB,   &v1, _mm512_loadu_si512(data + i + 14), _mm512_loadu_si512(data + i + 15));
            pospopcnt_csa_avx512(&foursB,  &v2, twosA, twosB);
            pospopcnt_csa_avx512(&eightsB, &v4, foursA, foursB);
            U(0) U(1) U(2) U(3) U(4) U(5) U(6) U(7) U(8) U(9) U(10) U(11) U(12) U(13) U(14) U(15) // Updates
            pospopcnt_csa_avx512(&v16,     &v8, eightsA, eightsB);
        }
        // Update the counters after the last iteration.
        for (size_t i = 0; i < 16; ++i) U(i)
#undef U
        
        for (size_t i = 0; i < 16; ++i) {
            _mm512_storeu_si512((__m512i*)buffer, counter[i]);
            for (size_t z = 0; z < 32; z++) {
                flags[i] += 16 * (uint32_t)buffer[z];
            }
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v1);
    for (size_t i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            flags[j] += 1 * ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v2);
    for (size_t i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    
    _mm512_storeu_si512((__m512i*)buffer, v4);
    for (size_t i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v8);
    for (size_t i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512bw_harley_seal, pospopcnt_u16_avx512bw_harley_seal)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_harley_seal)
#endif

#if defined(__AVX512VBMI__) && __AVX512VBMI__ == 1
int pospopcnt_u16_avx512vbmi_harley_seal(const uint16_t* array, uint32_t len, uint32_t* flags) {
    for (uint32_t i = len - (len % (32 * 16)); i < len; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += ((array[i] & (1 << j)) >> j);
        }
    }

    const __m512i* data = (const __m512i*)array;
    __m512i v1  = _mm512_setzero_si512();
    __m512i v2  = _mm512_setzero_si512();
    __m512i v4  = _mm512_setzero_si512();
    __m512i v8  = _mm512_setzero_si512();
    __m512i v16 = _mm512_setzero_si512();
    __m512i twosA, twosB, foursA, foursB, eightsA, eightsB;

    const size_t size = len / 32;
    const uint64_t limit = size - size % 16;

    uint16_t buffer[32];

    uint64_t i = 0;
    while (i < limit) {
        __m512i counter[16];
        for (size_t i = 0; i < 16; ++i) {
            counter[i] = _mm512_setzero_si512();
        }

        size_t thislimit = limit;
        if (thislimit - i >= (1 << 16))
            thislimit = i + (1 << 16) - 1;

        for (/**/; i < thislimit; i += 16) {
#define U(pos) {                     \
    counter[pos] = _mm512_add_epi16(counter[pos], _mm512_and_si512(v16, _mm512_set1_epi16(1))); \
    v16 = _mm512_srli_epi16(v16, 1); \
}
            pospopcnt_csa_avx512(&twosA,  &v1, _mm512_loadu_si512(data + i + 0), _mm512_loadu_si512(data + i + 1));
            pospopcnt_csa_avx512(&twosB,  &v1, _mm512_loadu_si512(data + i + 2), _mm512_loadu_si512(data + i + 3));
            pospopcnt_csa_avx512(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx512(&twosA,  &v1, _mm512_loadu_si512(data + i + 4), _mm512_loadu_si512(data + i + 5));
            pospopcnt_csa_avx512(&twosB,  &v1, _mm512_loadu_si512(data + i + 6), _mm512_loadu_si512(data + i + 7));
            pospopcnt_csa_avx512(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx512(&eightsA,&v4, foursA, foursB);
            pospopcnt_csa_avx512(&twosA,  &v1, _mm512_loadu_si512(data + i + 8),  _mm512_loadu_si512(data + i + 9));
            pospopcnt_csa_avx512(&twosB,  &v1, _mm512_loadu_si512(data + i + 10), _mm512_loadu_si512(data + i + 11));
            pospopcnt_csa_avx512(&foursA, &v2, twosA, twosB);
            pospopcnt_csa_avx512(&twosA,  &v1, _mm512_loadu_si512(data + i + 12), _mm512_loadu_si512(data + i + 13));
            pospopcnt_csa_avx512(&twosB,  &v1, _mm512_loadu_si512(data + i + 14), _mm512_loadu_si512(data + i + 15));
            pospopcnt_csa_avx512(&foursB, &v2, twosA, twosB);
            pospopcnt_csa_avx512(&eightsB,&v4, foursA, foursB);
            U(0) U(1) U(2) U(3) U(4) U(5) U(6) U(7) U(8) U(9) U(10) U(11) U(12) U(13) U(14) U(15) // Updates
            pospopcnt_csa_avx512(&v16,    &v8, eightsA, eightsB);
        }

        // Update the counters after the last iteration
        for (size_t i = 0; i < 16; ++i) U(i)
#undef U
        
        for (size_t i = 0; i < 16; i += 2) {
// 00000110 00000100 00000010 00000000, 00001110 00001100 00001010 00001000, 00010110 00010100 00010010 00010000, 00011110 00011100 00011010 00011000
// {6, 4, 2, 0},                        {14, 12, 10, 8},                     {22, 20, 18, 16},                    {30, 28, 26, 24}
// 00100110 00100100 00100010 00100000, 00101110 00101100 00101010 00101000, 00110110 00110100 00110010 00110000, 00111110 00111100 00111010 00111000
// {38, 36, 34, 32},                    {46, 44, 42, 40},                    {54, 52, 50, 48},                    {62, 60, 58, 56}
// 01000110 01000100 01000010 01000000, 01001110 01001100 01001010 01001000, 01010110 01010100 01010010 01010000, 01011110 01011100 01011010 01011000
// {70, 68, 66, 64},                    {78, 76, 74, 72},                    {86, 84, 82, 80},                    {94, 92, 90, 88}
// 01100110 01100100 01100010 01100000, 01101110 01101100 01101010 01101000, 01110110 01110100 01110010 01110000, 01111110 01111100 01111010 01111000
// {102, 100, 98, 96},                  {110, 108, 106, 104},                {118, 116, 114, 112},                {126, 124, 122, 120}
            __m512i shuffle_lo = _mm512_setr_epi32(0x06040200, 0x0e0c0a08, 0x16141210, 0x1e1c1a18,
                                                   0x26242220, 0x2e2c2a28, 0x36343230, 0x3e3c3a38,
                                                   0x46444240, 0x4e4c4a48, 0x56545250, 0x5e5c5a58,
                                                   0x66646260, 0x6e6c6a68, 0x76747270, 0x7e7c7a78);

// 00000111 00000101 00000011 00000001, 00001111 00001101 00001011 00001001, 00010111 00010101 00010011 00010001, 00011111 00011101 00011011 00011001
// {7, 5, 3, 1},                        {15, 13, 11, 9},                     {23, 21, 19, 17},                    {31, 29, 27, 25}
// 00100111 00100101 00100011 00100001, 00101111 00101101 00101011 00101001, 00110111 00110101 00110011 00110001, 00111111 00111101 00111011 00111001
// {39, 37, 35, 33},                    {47, 45, 43, 41},                    {55, 53, 51, 49},                    {63, 61, 59, 57}
// 01000111 01000101 01000011 01000001, 01001111 01001101 01001011 01001001, 01010111 01010101 01010011 01010001, 01011111 01011101 01011011 01011001
// {71, 69, 67, 65},                    {79, 77, 75, 73},                    {87, 85, 83, 81},                    {95, 93, 91, 89}
// 01100111 01100101 01100011 01100001, 01101111 01101101 01101011 01101001, 01110111 01110101 01110011 01110001, 01111111 01111101 01111011 01111001
 // {103, 101, 99, 97},                 {111, 109, 107, 105},                {119, 117, 115, 113},                {127, 125, 123, 121}
            __m512i shuffle_hi = _mm512_setr_epi32(0x07050301, 0x0f0d0b09, 0x17151311, 0x1f1d1b19,
                                                   0x27252321, 0x2f2d2b29, 0x37353331, 0x3f3d3b39,
                                                   0x47454341, 0x4f4d4b49, 0x57555351, 0x5f5d5b59,
                                                   0x67656361, 0x6f6d6b69, 0x77757371, 0x7f7d7b79);
            
            // Move **lower** bytes from 16-bit counters, so bytes 0..31 of
            // results are from counter[i] and 32..63 from counter[i+1].
            __m512i lo_bytes = _mm512_permutex2var_epi8(counter[i], shuffle_lo, counter[i + 1]);

            // Likewise, move **higher** bytes.
            __m512i hi_bytes = _mm512_permutex2var_epi8(counter[i], shuffle_hi, counter[i + 1]);

            // Sum the lower bytes: now each 64-bit word holds sum of 8 bytes.
            __m512i sum_lo = _mm512_sad_epu8(lo_bytes, _mm512_setzero_si512());

            // Likewise sum the higher bytes.
            __m512i sum_hi = _mm512_sad_epu8(hi_bytes, _mm512_setzero_si512());

            // Calculate final sums --- the sum of higher bytes has to be multiplied by 256 (1 << 8)
            __m512i sum = _mm512_add_epi64(sum_lo, _mm512_slli_epi64(sum_hi, 8));

            // Since _mm512_extract* are slow. Instead we use a local buffer that is most likely cached.
            uint64_t buf64[8];
            _mm512_storeu_si512((__m512i*)buf64, _mm512_slli_epi32(sum, 4));

            flags[i + 0] += buf64[0] + buf64[1] + buf64[2] + buf64[3];
            flags[i + 1] += buf64[4] + buf64[5] + buf64[6] + buf64[7];
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v1);
    for (size_t i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 1 * ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v2);
    for (size_t i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 2 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    
    _mm512_storeu_si512((__m512i*)buffer, v4);
    for (size_t i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 4 * ((buffer[i] & (1 << j)) >> j);
        }
    }

    _mm512_storeu_si512((__m512i*)buffer, v8);
    for (size_t i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            flags[j] += 8 * ((buffer[i] & (1 << j)) >> j);
        }
    }
    return 0;
}

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512vbmi_harley_seal, pospopcnt_u16_avx512vbmi_harley_seal)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512vbmi_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_avx512vbmi_harley_seal)
#endif

int pospopcnt_u16_avx512_masked_ops(const uint16_t* data, uint32_t len, uint32_t* flags) {
    __m512i counter = _mm512_setzero_si512();
    __m512i ones = _mm512_set1_epi32(1);

    for (uint32_t i = 0; i < len; ++i) {
        counter = _mm512_mask_add_epi32(counter, data[i], counter, ones);
    }

    _mm512_storeu_si512((__m512i*)flags, counter);
    return 0;
}

// Wrapper
int pospopcnt_u16_avx512_harley_seal(const uint16_t* data, uint32_t len, uint32_t* flags) { 
    #if defined(__AVX512BW__) && __AVX512BW__ == 1
    return pospopcnt_u16_avx512bw_harley_seal(data, len, flags);
    #elif defined(__AVX512VBMI__) && __AVX512VBMI__ == 1
    return pospopcnt_u16_avx512vbmi_harley_seal(data, len, flags);
    #else
    return(0);
    #endif
}
#undef AND_OR

make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512_mula2, pospopcnt_u16_avx512_mula2)
make_pospopcnt_u8_from_u16(pospopcnt_u8_avx512_masked_ops, pospopcnt_u16_avx512_masked_ops)
#else
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_adder_forest)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt_unroll4)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_blend_popcnt_unroll8)
pospopcnt_u16_stub(pospopcnt_u16_avx512bw_harley_seal)
pospopcnt_u16_stub(pospopcnt_u16_avx512_masked_ops)
pospopcnt_u16_stub(pospopcnt_u16_avx512_mula2)
pospopcnt_u16_stub(pospopcnt_u16_avx512vbmi_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_adder_forest)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt_unroll4)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_blend_popcnt_unroll8)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_harley_seal)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_popcnt4bit)
pospopcnt_u8_stub(pospopcnt_u8_avx512bw_sadbw)
pospopcnt_u8_stub(pospopcnt_u8_avx512_masked_ops)
pospopcnt_u8_stub(pospopcnt_u8_avx512_mula2)
pospopcnt_u8_stub(pospopcnt_u8_avx512vbmi_harley_seal)
#endif

#if __clang__ == 1 || __llvm__ == 1
#pragma clang diagnostic pop
#endif
