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

#include <memory>
#include <random>
#include <vector>

#include "gtest/gtest.h"
#include "logging/logging.hpp"
#include "number-theory/number-theory.hpp"
#include "poly/poly-mult-internal.hpp"
#include "poly/poly-mult.hpp"
#include "test/test-util.hpp"

#ifdef LATTICE_HAS_AVX512DQ
#include "poly/poly-mult-avx512.hpp"
#endif

namespace intel {
namespace lattice {

TEST(PolyMult, native_small) {
  std::vector<uint64_t> op1{1, 2, 3, 1, 1, 1, 0, 1};
  std::vector<uint64_t> op2{1, 1, 1, 1, 2, 3, 1, 0};
  std::vector<uint64_t> exp_out{1, 2, 3, 1, 2, 3, 0, 0};

  uint64_t modulus = 769;
  BarrettFactor<64> bf(modulus);

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), bf.Hi(), bf.Lo(),
                           modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, native_mult2) {
  std::vector<uint64_t> op1{1, 2,  3,  4,  5,  6,  7,  8,
                            9, 10, 11, 12, 13, 14, 15, 16};
  std::vector<uint64_t> op2{17, 18, 19, 20, 21, 22, 23, 24,
                            25, 26, 27, 28, 29, 30, 31, 32};
  std::vector<uint64_t> exp_out{17, 36, 57, 80, 4,  31, 60, 91,
                                23, 58, 95, 33, 74, 16, 61, 7};
  uint64_t modulus = 101;

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

#ifdef LATTICE_HAS_AVX512DQ
TEST(PolyMult, avx512_small) {
  std::vector<uint64_t> op1{1, 2, 3, 1, 1, 1, 0, 1, 0};
  std::vector<uint64_t> op2{1, 1, 1, 1, 2, 3, 1, 0, 0};
  std::vector<uint64_t> exp_out{1, 2, 3, 1, 2, 3, 0, 0, 0};

  uint64_t modulus = 769;
  BarrettFactor<64> bf(modulus);

  MultiplyModInPlaceAVX512<64>(op1.data(), op2.data(), op1.size(), bf.Hi(),
                               bf.Lo(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512_mult2) {
  std::vector<uint64_t> op1{1, 2,  3,  4,  5,  6,  7,  8,
                            9, 10, 11, 12, 13, 14, 15, 16};
  std::vector<uint64_t> op2{17, 18, 19, 20, 21, 22, 23, 24,
                            25, 26, 27, 28, 29, 30, 31, 32};
  std::vector<uint64_t> exp_out{17, 36, 57, 80, 4,  31, 60, 91,
                                23, 58, 95, 33, 74, 16, 61, 7};

  uint64_t modulus = 101;
  BarrettFactor<64> bf(modulus);

  MultiplyModInPlaceAVX512<64>(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

#endif

#ifdef LATTICE_HAS_AVX512IFMA
TEST(PolyMult, avx512ifma_small) {
  std::vector<uint64_t> op1{1, 2, 3, 1, 1, 1, 0, 1};
  std::vector<uint64_t> op2{1, 1, 1, 1, 2, 3, 1, 0};
  std::vector<uint64_t> exp_out{1, 2, 3, 1, 2, 3, 0, 0};

  uint64_t modulus = 769;
  BarrettFactor<52> bf(modulus);

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), bf.Hi(),
                               bf.Lo(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big) {
  uint64_t modulus = GeneratePrimes(1, 48, 1024)[0];

  std::vector<uint64_t> op1{modulus - 1, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{modulus - 1, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{1, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big2) {
  uint64_t modulus = GeneratePrimes(1, 48, 1024)[0];

  std::vector<uint64_t> op1{modulus - 1, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{2, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{modulus - 2, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big3) {
  uint64_t modulus = GeneratePrimes(1, 48, 1024)[0];

  std::vector<uint64_t> op1{modulus - 3, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{modulus - 4, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{12, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big4) {
  uint64_t p = 281474976749569;

  std::vector<uint64_t> op1{(p - 1) / 2, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{(p + 1) / 2, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{70368744187392, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), p);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big5) {
  uint64_t p = (1UL << 52) - 2;
  // uint64_t p = GeneratePrimes(10, 28, 1024)[9];

  std::vector<uint64_t> op1{(p - 1), 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{(p - 1), 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{1, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), p);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_big6) {
  uint64_t p = 1125891450734593;

  std::vector<uint64_t> op1{1078888294739028, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{1114802337613200, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{13344071208410, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), p);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, avx512ifma_mult2) {
  std::vector<uint64_t> op1{1, 2,  3,  4,  5,  6,  7,  8,
                            9, 10, 11, 12, 13, 14, 15, 16};
  std::vector<uint64_t> op2{17, 18, 19, 20, 21, 22, 23, 24,
                            25, 26, 27, 28, 29, 30, 31, 32};
  std::vector<uint64_t> exp_out{17, 36, 57, 80, 4,  31, 60, 91,
                                23, 58, 95, 33, 74, 16, 61, 7};

  uint64_t modulus = 101;
  BarrettFactor<52> bf(modulus);

  MultiplyModInPlaceAVX512<52>(op1.data(), op2.data(), op1.size(), bf.Hi(),
                               bf.Lo(), modulus);

  CheckEqual(op1, exp_out);
}
#endif

TEST(PolyMult, 4) {
  std::vector<uint64_t> op1{2, 4, 3, 2};
  std::vector<uint64_t> op2{2, 1, 2, 0};
  std::vector<uint64_t> exp_out{4, 4, 6, 0};

  uint64_t modulus = 769;

  MultiplyModInPlace(op1.data(), op2.data(), op1.size(), modulus);
  CheckEqual(op1, exp_out);
}

TEST(PolyMult, 6) {
  std::vector<uint64_t> op1{0, 1, 2, 3, 4, 5};
  std::vector<uint64_t> op2{2, 4, 6, 8, 10, 12};
  std::vector<uint64_t> exp_out{0, 4, 12, 24, 40, 60};

  uint64_t modulus = 769;

  MultiplyModInPlace(op1.data(), op2.data(), op1.size(), modulus);
  CheckEqual(op1, exp_out);
}

TEST(PolyMult, 8big) {
  uint64_t modulus = GeneratePrimes(1, 48, 1024)[0];

  std::vector<uint64_t> op1{modulus - 1, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{modulus - 1, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{1, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, 8big3) {
  uint64_t modulus = GeneratePrimes(1, 48, 1024)[0];

  std::vector<uint64_t> op1{modulus - 3, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{modulus - 4, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{12, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), modulus);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, 8big4) {
  uint64_t p = 281474976749569;

  std::vector<uint64_t> op1{(p - 1) / 2, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{(p + 1) / 2, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{70368744187392, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), p);

  CheckEqual(op1, exp_out);
}

TEST(PolyMult, 8big6) {
  uint64_t p = 1125891450734593;

  std::vector<uint64_t> op1{1078888294739028, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> op2{1114802337613200, 1, 1, 1, 1, 1, 1, 1};
  std::vector<uint64_t> exp_out{13344071208410, 1, 1, 1, 1, 1, 1, 1};

  MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), p);

  CheckEqual(op1, exp_out);
}

#ifdef LATTICE_DEBUG
TEST(PolyMult, 8_bounds) {
  std::vector<uint64_t> op1{0, 1, 2, 3, 4, 5, 6, 7};
  std::vector<uint64_t> op2{0, 1, 2, 3, 4, 5, 6, 770};

  uint64_t modulus = 769;

  EXPECT_ANY_THROW(
      MultiplyModInPlace(op1.data(), op2.data(), op1.size(), modulus));
}
#endif

// Checks AVX512 and native poly mult implementations match
#ifdef LATTICE_HAS_AVX512IFMA
TEST(PolyMult, AVX512) {
  uint64_t length = 1024;
  std::random_device rd;
  std::mt19937 gen(rd());

  for (size_t bits = 48; bits <= 51; ++bits) {
    uint64_t prime = GeneratePrimes(1, bits, 1024)[0];
    std::uniform_int_distribution<> distrib(0, prime - 1);

    for (size_t trial = 0; trial < 1000; ++trial) {
      std::vector<std::uint64_t> op1(length, 0);
      std::vector<std::uint64_t> op2(length, 0);
      for (size_t i = 0; i < length; ++i) {
        op1[i] = distrib(gen);
        op2[i] = distrib(gen);
      }
      std::vector<std::uint64_t> op1a = op1;

      MultiplyModInPlaceNative(op1.data(), op2.data(), op1.size(), prime);
      MultiplyModInPlaceAVX512<52>(op1a.data(), op2.data(), op1.size(), prime);

      ASSERT_EQ(op1, op1a);
    }
  }
}
#endif

}  // namespace lattice
}  // namespace intel