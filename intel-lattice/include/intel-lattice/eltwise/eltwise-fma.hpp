// *****************************************************************************
// INTEL CONFIDENTIAL
// Copyright 2020-2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
// *****************************************************************************

#pragma once

#include <stdint.h>

namespace intel {
namespace lattice {

/// @brief Computes fused multiply-add (\p arg1 * \p arg2 + \p arg3) mod \p
/// modulus element-wise, broadcasting scalars to vectors.
/// @param[in] arg1 Vector to multiply
/// @param[in] arg2 Scalar to multiply
/// @param[in] arg3 Vector to add. Will not add if \p arg3 == nullptr
/// @param[out] out Stores the output
/// @param[in] n Number of elements in each vector
/// @param[in] modulus Modulus with which to perform modular reduction
void EltwiseFMAMod(const uint64_t* arg1, uint64_t arg2, const uint64_t* arg3,
                   uint64_t* out, uint64_t n, uint64_t modulus);

}  // namespace lattice
}  // namespace intel
