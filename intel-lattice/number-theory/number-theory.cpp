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

#include "number-theory/number-theory.hpp"

#include <bitset>
#include <random>

#include "logging/logging.hpp"
#include "util/check.hpp"

namespace intel {
namespace lattice {

uint64_t InverseUIntMod(uint64_t input, uint64_t modulus) {
  uint64_t a = input % modulus;
  LATTICE_CHECK(a != 0,
                input << " does not have a InverseMod with modulus " << modulus)

  if (modulus == 1) {
    return 0;
  }

  int64_t m0 = modulus;
  int64_t y = 0;
  int64_t x = 1;
  while (a > 1) {
    // q is quotient
    int64_t q = a / modulus;

    int64_t t = modulus;
    modulus = a % modulus;
    a = t;

    // Update y and x
    t = y;
    y = x - q * y;
    x = t;
  }

  // Make x positive
  if (x < 0) x += m0;

  return uint64_t(x);
}

uint64_t BarrettReduce128(const uint128_t input, const uint64_t modulus) {
  LATTICE_CHECK(modulus != 0, "modulus == 0")
  return input % modulus;
  // TODO(fboemer): actually use barrett reduction if performance-critical
}

uint64_t MultiplyUIntMod(uint64_t x, uint64_t y, const uint64_t modulus) {
  LATTICE_CHECK(modulus != 0, "modulus == 0");
  LATTICE_CHECK(x < modulus, "x " << x << " > modulus " << modulus);
  LATTICE_CHECK(y < modulus, "y " << y << " > modulus " << modulus);
  uint128_t z = MultiplyUInt64(x, y);

  return BarrettReduce128(z, modulus);
}

// Returns base^exp mod modulus
uint64_t PowMod(uint64_t base, uint64_t exp, uint64_t modulus) {
  base %= modulus;
  uint64_t result = 1;
  while (exp > 0) {
    if (exp & 1) {
      result = MultiplyUIntMod(result, base, modulus);
    }
    base = MultiplyUIntMod(base, base, modulus);
    exp >>= 1;
  }
  return result;
}

// Returns true whether root is a degree-th root of unity
// degree must be a power of two.
bool IsPrimitiveRoot(uint64_t root, uint64_t degree, uint64_t modulus) {
  if (root == 0) {
    return false;
  }
  LATTICE_CHECK(IsPowerOfTwo(degree), degree << " not a power of 2");

  IVLOG(4, "IsPrimitiveRoot root " << root << ", degree " << degree
                                   << ", modulus " << modulus);

  // Check if root^(degree/2) == -1 mod modulus
  return PowMod(root, degree / 2, modulus) == (modulus - 1);
}

// Tries to return a primtiive degree-th root of unity
// throw std::invalid_argumentif no root is found
uint64_t GeneratePrimitiveRoot(uint64_t degree, uint64_t modulus) {
  std::default_random_engine generator;
  std::uniform_int_distribution<uint64_t> distribution(0, modulus - 1);

  // We need to divide modulus-1 by degree to get the size of the quotient group
  uint64_t size_entire_group = modulus - 1;

  // Compute size of quotient group
  uint64_t size_quotient_group = size_entire_group / degree;

  for (int trial = 0; trial < 1000; ++trial) {
    uint64_t root = distribution(generator);

    root = PowMod(root, size_quotient_group, modulus);

    if (IsPrimitiveRoot(root, degree, modulus)) {
      return root;
    }
  }
  LATTICE_CHECK(false, "no primitive root found for degree "
                           << degree << " modulus " << modulus);
  return 0;
}

// Returns true whether root is a degree-th root of unity
// degree must be a power of two.
uint64_t MinimalPrimitiveRoot(uint64_t degree, uint64_t modulus) {
  LATTICE_CHECK(IsPowerOfTwo(degree),
                "Degere " << degree << " is not a power of 2");

  uint64_t root = GeneratePrimitiveRoot(degree, modulus);

  uint64_t generator_sq = MultiplyUIntMod(root, root, modulus);
  uint64_t current_generator = root;

  uint64_t min_root = root;

  // Check if root^(degree/2) == -1 mod modulus
  for (size_t i = 0; i < degree; ++i) {
    if (current_generator < min_root) {
      min_root = current_generator;
    }
    current_generator =
        MultiplyUIntMod(current_generator, generator_sq, modulus);
  }

  return min_root;
}

uint64_t ReverseBitsUInt(uint64_t x, uint64_t bit_width) {
  if (bit_width == 0) {
    return 0;
  }
  uint64_t rev = 0;
  for (int i = bit_width; i > 0; i--) {
    rev |= ((x & 1) << (i - 1));
    x >>= 1;
  }
  return rev;
}

bool IsPrime(const uint64_t n) {
  static const std::vector<uint64_t> as{2,  3,  5,  7,  11, 13,
                                        17, 19, 23, 29, 31, 37};

  for (const uint64_t a : as) {
    if (n == a) return true;
    if (n % a == 0) return false;
  }

  // Miller-Rabin primality test
  // n < 2^64, so it is enough to test a=2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
  // and 37. See
  // https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test#Testing_against_small_sets_of_bases

  // Write n == 2**r * d + 1 with d odd.
  uint64_t r = 63;
  while (r > 0) {
    uint64_t two_pow_r = (1UL << r);
    if ((n - 1) % two_pow_r == 0) {
      break;
    }
    --r;
  }
  LATTICE_CHECK(r != 0, "Error factoring n " << n);
  uint64_t d = (n - 1) / (1UL << r);

  LATTICE_CHECK(n == (1UL << r) * d + 1, "Error factoring n " << n);
  LATTICE_CHECK(d % 2 == 1, "d is even");

  for (const uint64_t a : as) {
    uint64_t x = PowMod(a, d, n);
    if ((x == 1) || (x == n - 1)) {
      continue;
    }

    bool prime = false;
    for (uint64_t i = 1; i < r; ++i) {
      x = PowMod(x, 2, n);
      if (x == n - 1) {
        prime = true;
      }
    }
    if (!prime) {
      return false;
    }
  }
  return true;
}

std::vector<uint64_t> GeneratePrimes(size_t num_primes, size_t bit_size,
                                     size_t ntt_size) {
  LATTICE_CHECK(num_primes > 0, "num_primes == 0");
  LATTICE_CHECK(IsPowerOfTwo(ntt_size),
                "ntt_size " << ntt_size << " is not a power of two");
  LATTICE_CHECK(Log2(ntt_size) < bit_size,
                "log2(ntt_size) " << Log2(ntt_size)
                                  << " should be less than bit_size "
                                  << bit_size);

  uint64_t value = (1UL << bit_size) + 1;

  std::vector<uint64_t> ret;

  while (value < (1UL << (bit_size + 1))) {
    if (IsPrime(value)) {
      ret.emplace_back(value);
      if (ret.size() == num_primes) {
        return ret;
      }
    }
    value += 2 * ntt_size;
  }

  LATTICE_CHECK(false, "Failed to find enough primes");
  return ret;
}

}  // namespace lattice
}  // namespace intel