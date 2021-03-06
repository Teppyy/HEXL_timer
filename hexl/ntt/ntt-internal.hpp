// Copyright (C) 2020-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdint.h>

#include <utility>

#include "hexl/ntt/ntt.hpp"
#include "hexl/number-theory/number-theory.hpp"
#include "hexl/util/aligned-allocator.hpp"
#include "hexl/util/check.hpp"
#include "hexl/util/util.hpp"
#include "util/util-internal.hpp"

namespace intel {
namespace hexl {

void ForwardTransformToBitReverse64(uint64_t* operand, uint64_t n,
                                    uint64_t modulus,
                                    const uint64_t* root_of_unity_powers,
                                    const uint64_t* precon_root_of_unity_powers,
                                    uint64_t input_mod_factor = 1,
                                    uint64_t output_mod_factor = 1);

/// @brief Reference NTT which is written for clarity rather than performance
/// @param[in, out] operand Input data. Overwritten with NTT output
/// @param[in] n Size of the transfrom, i.e. the polynomial degree. Must be a
/// power of two.
/// @param[in] modulus Prime modulus. Must satisfy q == 1 mod 2n
/// @param[in] root_of_unity_powers Powers of 2n'th root of unity in F_q. In
/// bit-reversed order
void ReferenceForwardTransformToBitReverse(
    uint64_t* operand, uint64_t n, uint64_t modulus,
    const uint64_t* root_of_unity_powers);

void InverseTransformFromBitReverse64(
    uint64_t* operand, uint64_t n, uint64_t modulus,
    const uint64_t* inv_root_of_unity_powers,
    const uint64_t* precon_inv_root_of_unity_powers,
    uint64_t input_mod_factor = 1, uint64_t output_mod_factor = 1);

// Returns true if arguments satisfy constraints for negacyclic NTT
bool CheckNTTArguments(uint64_t degree, uint64_t modulus);

}  // namespace hexl
}  // namespace intel
