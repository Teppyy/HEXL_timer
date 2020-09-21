//*****************************************************************************
// Copyright 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#pragma once

#include <immintrin.h>
#include <stdint.h>

#include "number-theory/number-theory.hpp"
#include "poly/poly-fma-internal.hpp"
#include "poly/poly-fma.hpp"
#include "util/avx512-util.hpp"
#include "util/check.hpp"

namespace intel {
namespace lattice {

template <int BitShift>
void FMAModScalarAVX512(const uint64_t* arg1, const uint64_t arg2,
                        const uint64_t* arg3, uint64_t* out,
                        const uint64_t arg2_barr, uint64_t n,
                        const uint64_t modulus) {
  uint64_t n_mod_8 = n % 8;
  if (n_mod_8 != 0) {
    FMAModScalarNative(arg1, arg2, arg3, out, arg2_barr, n_mod_8, modulus);
    arg1 += n_mod_8;
    arg3 += n_mod_8;
    // out += n -= n_mod_8;
    n -= n_mod_8;
  }
  LATTICE_CHECK((modulus) < MaximumValue(BitShift),
                "Modulus " << (modulus) << " exceeds bit shift bound "
                           << MaximumValue(BitShift));

  LATTICE_CHECK_BOUNDS(arg1, n, modulus,
                       "pre-mult value in arg1 exceeds bound " << modulus);
  LATTICE_CHECK_BOUNDS(&arg2, 1, modulus, "arg2 exceeds bound " << modulus);
  LATTICE_CHECK(BitShift == 52 || BitShift == 64,
                "Invalid bitshift " << BitShift << "; need 52 or 64");

  __m512i varg2_barr = _mm512_set1_epi64(arg2_barr);

  __m512i vmodulus = _mm512_set1_epi64(modulus);
  const __m512i* vp_arg1 = reinterpret_cast<const __m512i*>(arg1);
  __m512i varg2 = _mm512_set1_epi64(arg2);
  __m512i* vp_out = reinterpret_cast<__m512i*>(out);

  if (arg3) {
    const __m512i* vp_arg3 = reinterpret_cast<const __m512i*>(arg3);
#pragma GCC unroll 4
#pragma clang loop unroll_count(4)
    for (size_t i = n / 8; i > 0; --i) {
      __m512i varg1 = _mm512_loadu_si512(vp_arg1);
      __m512i varg3 = _mm512_loadu_si512(vp_arg3);

      // uint64_t q = MultiplyUInt64Hi<64>(arg1, arg2_precon);
      __m512i vq = _mm512_il_mulhi_epi<BitShift>(varg1, varg2_barr);
      __m512i vq_times_mod = _mm512_mullo_epi64(vq, vmodulus);
      __m512i va_times_b = _mm512_il_mullo_epi<64>(varg1, varg2);
      // q = arg1 * arg2 - q * modulus;
      vq = _mm512_sub_epi64(va_times_b, vq_times_mod);
      // Conditional Barrett subtraction
      vq = _mm512_il_small_mod_epi64(vq, vmodulus);

      // result = AddUIntMod(*arg1, *arg3, modulus);
      vq = _mm512_add_epi64(vq, varg3);
      vq = _mm512_il_small_mod_epi64(vq, vmodulus);

      _mm512_storeu_si512(vp_out, vq);

      ++vp_arg1;
      ++vp_out;
      ++vp_arg3;
    }
    return;
  }

  // arg3 == nullptr
#pragma GCC unroll 4
#pragma clang loop unroll_count(4)
  for (size_t i = n / 8; i > 0; --i) {
    __m512i varg1 = _mm512_loadu_si512(vp_arg1);
    // uint64_t q = MultiplyUInt64Hi<64>(arg1, arg2_precon);
    __m512i vq = _mm512_il_mulhi_epi<BitShift>(varg1, varg2_barr);
    __m512i vq_times_mod = _mm512_mullo_epi64(vq, vmodulus);
    __m512i va_times_b = _mm512_il_mullo_epi<64>(varg1, varg2);
    // q = arg1 * arg2 - q * modulus;
    vq = _mm512_sub_epi64(va_times_b, vq_times_mod);
    // Conditional Barrett subtraction
    vq = _mm512_il_small_mod_epi64(vq, vmodulus);
    _mm512_storeu_si512(vp_out, vq);

    ++vp_arg1;
    ++vp_out;
  }
  return;
}

template <int BitShift>
inline void FMAModScalarAVX512(const uint64_t* arg1, uint64_t arg2,
                               const uint64_t* arg3, uint64_t* out, uint64_t n,
                               uint64_t modulus) {
  MultiplyFactor mf(arg2, BitShift, modulus);

  FMAModScalarAVX512<BitShift>(arg1, arg2, arg3, out, mf.BarrettFactor(), n,
                               modulus);
}

}  // namespace lattice
}  // namespace intel