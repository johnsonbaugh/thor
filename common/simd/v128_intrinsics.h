/*
Copyright (c) 2015, Cisco Systems
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2; -*- */

#ifndef _V128_INTRINSICS_H
#define _V128_INTRINSICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v64_intrinsics.h"
#include "v128_intrinsics_c.h"

/* Fallback to plain, unoptimised C. */

typedef c_v128 v128;

SIMD_INLINE uint32_t v128_low_u32(v128 a) { return c_v128_low_u32(a); }
SIMD_INLINE v64 v128_low_v64(v128 a) { return c_v128_low_v64(a); }
SIMD_INLINE v64 v128_high_v64(v128 a) { return c_v128_high_v64(a); }
SIMD_INLINE v128 v128_from_64(uint64_t hi, uint64_t lo) { return c_v128_from_64(hi, lo); }
SIMD_INLINE v128 v128_from_v64(v64 hi, v64 lo) { return c_v128_from_v64(hi, lo); }
SIMD_INLINE v128 v128_from_32(uint32_t a, uint32_t b, uint32_t c, uint32_t d) { return c_v128_from_32(a, b, c, d); }

SIMD_INLINE v128 v128_load_unaligned(const void *p) { return c_v128_load_unaligned(p); }
SIMD_INLINE v128 v128_load_aligned(const void *p) { return c_v128_load_aligned(p); }

SIMD_INLINE void v128_store_unaligned(void *p, v128 a) { c_v128_store_unaligned(p, a); }
SIMD_INLINE void v128_store_aligned(void *p, v128 a) { c_v128_store_aligned(p, a); }

SIMD_INLINE v128 v128_align(v128 a, v128 b, const unsigned int c) { return c_v128_align(a, b, c); }

SIMD_INLINE v128 v128_zero() { return c_v128_zero(); }
SIMD_INLINE v128 v128_dup_8(uint8_t x) { return c_v128_dup_8(x); }
SIMD_INLINE v128 v128_dup_16(uint16_t x) { return c_v128_dup_16(x); }
SIMD_INLINE v128 v128_dup_32(uint32_t x) { return c_v128_dup_32(x); }


typedef uint32_t sad128_internal;
SIMD_INLINE sad128_internal v128_sad_u8_init() { return c_v128_sad_u8_init(); }
SIMD_INLINE sad128_internal v128_sad_u8(sad128_internal s, v128 a, v128 b) { return c_v128_sad_u8(s, a, b); }
SIMD_INLINE uint32_t v128_sad_u8_sum(sad128_internal s) { return c_v128_sad_u8_sum(s); }
typedef uint32_t ssd128_internal;
SIMD_INLINE ssd128_internal v128_ssd_u8_init() { return c_v128_ssd_u8_init(); }
SIMD_INLINE ssd128_internal v128_ssd_u8(ssd128_internal s, v128 a, v128 b) { return c_v128_ssd_u8(s, a, b); }
SIMD_INLINE uint32_t v128_ssd_u8_sum(ssd128_internal s) { return c_v128_ssd_u8_sum(s); }
SIMD_INLINE int64_t v128_dotp_s16(v128 a, v128 b) { return c_v128_dotp_s16(a, b); }
SIMD_INLINE uint64_t v128_hadd_u8(v128 a) { return c_v128_hadd_u8(a); }


SIMD_INLINE v128 v128_or(v128 a, v128 b) { return c_v128_or(a, b); }
SIMD_INLINE v128 v128_xor(v128 a, v128 b) { return c_v128_xor(a, b); }
SIMD_INLINE v128 v128_and(v128 a, v128 b) { return c_v128_and(a, b); }
SIMD_INLINE v128 v128_andn(v128 a, v128 b) { return c_v128_andn(a, b); }


SIMD_INLINE v128 v128_add_8(v128 a, v128 b) { return c_v128_add_8(a, b); }
SIMD_INLINE v128 v128_add_16(v128 a, v128 b) { return c_v128_add_16(a, b); }
SIMD_INLINE v128 v128_sadd_s16(v128 a, v128 b) { return c_v128_sadd_s16(a, b); }
SIMD_INLINE v128 v128_add_32(v128 a, v128 b) { return c_v128_add_32(a, b); }
SIMD_INLINE v128 v128_padd_s16(v128 a) { return c_v128_padd_s16(a); }
SIMD_INLINE v128 v128_sub_8(v128 a, v128 b) { return c_v128_sub_8(a, b); }
SIMD_INLINE v128 v128_ssub_u8(v128 a, v128 b) { return c_v128_ssub_u8(a, b); }
SIMD_INLINE v128 v128_sub_16(v128 a, v128 b) { return c_v128_sub_16(a, b); }
SIMD_INLINE v128 v128_ssub_s16(v128 a, v128 b) { return c_v128_ssub_s16(a, b); }
SIMD_INLINE v128 v128_sub_32(v128 a, v128 b) { return c_v128_sub_32(a, b); }
SIMD_INLINE v128 v128_abs_s16(v128 a) { return c_v128_abs_s16(a); }


SIMD_INLINE v128 v128_mul_s16(v64 a, v64 b) { return c_v128_mul_s16(a, b); }
SIMD_INLINE v128 v128_mullo_s16(v128 a, v128 b) { return c_v128_mullo_s16(a, b); }
SIMD_INLINE v128 v128_mulhi_s16(v128 a, v128 b) { return c_v128_mulhi_s16(a, b); }
SIMD_INLINE v128 v128_mullo_s32(v128 a, v128 b) { return c_v128_mullo_s32(a, b); }
SIMD_INLINE v128 v128_madd_s16(v128 a, v128 b) { return c_v128_madd_s16(a, b); }
SIMD_INLINE v128 v128_madd_us8(v128 a, v128 b) { return c_v128_madd_us8(a, b); }


SIMD_INLINE v128 v128_avg_u8(v128 a, v128 b) { return c_v128_avg_u8(a, b); }
SIMD_INLINE v128 v128_rdavg_u8(v128 a, v128 b) { return c_v128_rdavg_u8(a, b); }
SIMD_INLINE v128 v128_avg_u16(v128 a, v128 b) { return c_v128_avg_u16(a, b); }
SIMD_INLINE v128 v128_min_u8(v128 a, v128 b) { return c_v128_min_u8(a, b); }
SIMD_INLINE v128 v128_max_u8(v128 a, v128 b) { return c_v128_max_u8(a, b); }
SIMD_INLINE v128 v128_min_s16(v128 a, v128 b) { return c_v128_min_s16(a, b); }
SIMD_INLINE v128 v128_max_s16(v128 a, v128 b) { return c_v128_max_s16(a, b); }


SIMD_INLINE v128 v128_ziplo_8(v128 a, v128 b) { return c_v128_ziplo_8(a, b); }
SIMD_INLINE v128 v128_ziphi_8(v128 a, v128 b) { return c_v128_ziphi_8(a, b); }
SIMD_INLINE v128 v128_ziplo_16(v128 a, v128 b) { return c_v128_ziplo_16(a, b); }
SIMD_INLINE v128 v128_ziphi_16(v128 a, v128 b) { return c_v128_ziphi_16(a, b); }
SIMD_INLINE v128 v128_ziplo_32(v128 a, v128 b) { return c_v128_ziplo_32(a, b); }
SIMD_INLINE v128 v128_ziphi_32(v128 a, v128 b) { return c_v128_ziphi_32(a, b); }
SIMD_INLINE v128 v128_ziplo_64(v128 a, v128 b) { return c_v128_ziplo_64(a, b); }
SIMD_INLINE v128 v128_ziphi_64(v128 a, v128 b) { return c_v128_ziphi_64(a, b); }
SIMD_INLINE v128 v128_zip_8(v64 a, v64 b) { return c_v128_zip_8(a, b); }
SIMD_INLINE v128 v128_zip_16(v64 a, v64 b) { return c_v128_zip_16(a, b); }
SIMD_INLINE v128 v128_zip_32(v64 a, v64 b) { return c_v128_zip_32(a, b); }
SIMD_INLINE v128 v128_unziplo_8(v128 a, v128 b) { return c_v128_unziplo_8(a, b); }
SIMD_INLINE v128 v128_unziphi_8(v128 a, v128 b) { return c_v128_unziphi_8(a, b); }
SIMD_INLINE v128 v128_unziplo_16(v128 a, v128 b) { return c_v128_unziplo_16(a, b); }
SIMD_INLINE v128 v128_unziphi_16(v128 a, v128 b) { return c_v128_unziphi_16(a, b); }
SIMD_INLINE v128 v128_unziplo_32(v128 a, v128 b) { return c_v128_unziplo_32(a, b); }
SIMD_INLINE v128 v128_unziphi_32(v128 a, v128 b) { return c_v128_unziphi_32(a, b); }
SIMD_INLINE v128 v128_unpack_u8_s16(v64 a) { return c_v128_unpack_u8_s16(a); }
SIMD_INLINE v128 v128_unpacklo_u8_s16(v128 a) { return c_v128_unpacklo_u8_s16(a); }
SIMD_INLINE v128 v128_unpackhi_u8_s16(v128 a) { return c_v128_unpackhi_u8_s16(a); }
SIMD_INLINE v128 v128_pack_s32_s16(v128 a, v128 b) { return c_v128_pack_s32_s16(a, b); }
SIMD_INLINE v128 v128_pack_s16_u8(v128 a, v128 b) { return c_v128_pack_s16_u8(a, b); }
SIMD_INLINE v128 v128_pack_s16_s8(v128 a, v128 b) { return c_v128_pack_s16_s8(a, b); }
SIMD_INLINE v128 v128_unpack_u16_s32(v64 a) { return c_v128_unpack_u16_s32(a); }
SIMD_INLINE v128 v128_unpack_s16_s32(v64 a) { return c_v128_unpack_s16_s32(a); }
SIMD_INLINE v128 v128_unpacklo_u16_s32(v128 a) { return c_v128_unpacklo_u16_s32(a); }
SIMD_INLINE v128 v128_unpacklo_s16_s32(v128 a) { return c_v128_unpacklo_s16_s32(a); }
SIMD_INLINE v128 v128_unpackhi_u16_s32(v128 a) { return c_v128_unpackhi_u16_s32(a); }
SIMD_INLINE v128 v128_unpackhi_s16_s32(v128 a) { return c_v128_unpackhi_s16_s32(a); }
SIMD_INLINE v128 v128_shuffle_8(v128 a, v128 pattern) { return c_v128_shuffle_8(a, pattern); }


SIMD_INLINE v128 v128_cmpgt_s8(v128 a, v128 b) { return c_v128_cmpgt_s8(a, b); }
SIMD_INLINE v128 v128_cmplt_s8(v128 a, v128 b) { return c_v128_cmplt_s8(a, b); }
SIMD_INLINE v128 v128_cmpeq_8(v128 a, v128 b) { return c_v128_cmpeq_8(a, b); }
SIMD_INLINE v128 v128_cmpgt_s16(v128 a, v128 b) { return c_v128_cmpgt_s16(a, b); }
SIMD_INLINE v128 v128_cmplt_s16(v128 a, v128 b) { return c_v128_cmplt_s16(a, b); }
SIMD_INLINE v128 v128_cmpeq_16(v128 a, v128 b) { return c_v128_cmpeq_16(a, b); }


SIMD_INLINE v128 v128_shl_8(v128 a, unsigned int c) { return c_v128_shl_8(a, c); }
SIMD_INLINE v128 v128_shr_u8(v128 a, unsigned int c) { return c_v128_shr_u8(a, c); }
SIMD_INLINE v128 v128_shr_s8(v128 a, unsigned int c) { return c_v128_shr_s8(a, c); }
SIMD_INLINE v128 v128_shl_16(v128 a, unsigned int c) { return c_v128_shl_16(a, c); }
SIMD_INLINE v128 v128_shr_u16(v128 a, unsigned int c) { return c_v128_shr_u16(a, c); }
SIMD_INLINE v128 v128_shr_s16(v128 a, unsigned int c) { return c_v128_shr_s16(a, c); }
SIMD_INLINE v128 v128_shl_32(v128 a, unsigned int c) { return c_v128_shl_32(a, c); }
SIMD_INLINE v128 v128_shr_u32(v128 a, unsigned int c) { return c_v128_shr_u32(a, c); }
SIMD_INLINE v128 v128_shr_s32(v128 a, unsigned int c) { return c_v128_shr_s32(a, c); }

SIMD_INLINE v128 v128_shr_n_byte(v128 a, const unsigned int n) { return c_v128_shr_n_byte(a, n); }
SIMD_INLINE v128 v128_shl_n_byte(v128 a, const unsigned int n) { return c_v128_shl_n_byte(a, n); }
SIMD_INLINE v128 v128_shl_n_8(v128 a, const unsigned int n) { return c_v128_shl_n_8(a, n); }
SIMD_INLINE v128 v128_shl_n_16(v128 a, const unsigned int n) { return c_v128_shl_n_16(a, n); }
SIMD_INLINE v128 v128_shl_n_32(v128 a, const unsigned int n) { return c_v128_shl_n_32(a, n); }
SIMD_INLINE v128 v128_shr_n_u8(v128 a, const unsigned int n) { return c_v128_shr_n_u8(a, n); }
SIMD_INLINE v128 v128_shr_n_u16(v128 a, const unsigned int n) { return c_v128_shr_n_u16(a, n); }
SIMD_INLINE v128 v128_shr_n_u32(v128 a, const unsigned int n) { return c_v128_shr_n_u32(a, n); }
SIMD_INLINE v128 v128_shr_n_s8(v128 a, const unsigned int n) { return c_v128_shr_n_s8(a, n); }
SIMD_INLINE v128 v128_shr_n_s16(v128 a, const unsigned int n) { return c_v128_shr_n_s16(a, n); }
SIMD_INLINE v128 v128_shr_n_s32(v128 a, const unsigned int n) { return c_v128_shr_n_s32(a, n); }

#endif /* _V128_INTRINSICS_H */
