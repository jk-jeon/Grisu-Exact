// Copyright 2020 Junekey Jeon
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

#include "../grisu_exact.h"

#include <iostream>
#include <iomanip>

void verify_correct_rounding_bound()
{
	std::cout << "[Verifying correct rounding bound...]\n";

	using namespace jkj::grisu_exact_detail;

	auto verify_single_type = [](auto type_tag) {
		using float_type = typename decltype(type_tag)::float_type;
		using extended_significand_type = typename common_info<float_type>::extended_significand_type;

		bool success = true;

		constexpr auto max_exponent_shifted =
			((int)(1) << common_info<float_type>::exponent_bits) - 1;
		for (int e_shifted = 1; e_shifted < max_exponent_shifted; ++e_shifted)
		{
			// Compose bits
			auto const bit_rep = extended_significand_type(e_shifted) << common_info<float_type>::precision;
			float_type x;
			std::memcpy(&x, &bit_rep, sizeof(bit_rep));

			// Compute e, k, and beta
			auto const e = e_shifted + common_info<float_type>::exponent_bias
				- int(common_info<float_type>::extended_precision) + 1;
			auto const k = -floor_log10_pow2(e + 1 - common_info<float_type>::alpha);
			int const beta = e + floor_log2_pow10(k) + 1;

			// Run Grisu-Exact without correct rounding search to inspect the possible range of kappa
			// Since the significand is always even, nearest-to-odd is the most harsh condition, and
			// nearest-to-even is the most generous condition
			auto const grisu_exact_result_harsh = jkj::grisu_exact(x,
				jkj::grisu_exact_rounding_modes::nearest_to_odd{},
				jkj::grisu_exact_correct_rounding::do_not_care{});
			auto const grisu_exact_result_generous = jkj::grisu_exact(x,
				jkj::grisu_exact_rounding_modes::nearest_to_even{},
				jkj::grisu_exact_correct_rounding::do_not_care{});

			auto const kappa_min = grisu_exact_result_harsh.exponent + k;
			auto const kappa_max = grisu_exact_result_generous.exponent + k;

			if (kappa_min != kappa_max) {
				std::cout << "Detected mismatch between kappa's for different rounding modes!\n";
				return false;
			}
			auto const kappa = kappa_min;
			
			// Note that the generous case always include the boundary, thus
			// this value does not have the issue of kappa'
			auto const right_bdy = grisu_exact_result_generous.significand;

			// Correctly compute ceil(y/10^kappa - 1/2)
			// Since y = 2^(e+q-1) * 10^k, let
			// 2^(e+q) * 10^k = a * 10^kappa + b for some b in [0,10^kappa)
			// Then 2^(e+q) * 10^(k-kappa) = a + b * 10^-kappa, so
			// 2y/10^kappa - 1 = (a-1) + b * 10^-kappa.
			// If a-1 is odd, we get floor((a-1)/2) + 1
			// If a-1 is even and b != 0, we get floor((a-1)/2) + 1
			// If a-1 is even and b == 0, we get floor((a-1)/2).
			// 2^(e+q) * 10^k <= 2^(e+ek+q) * phi_k < 2^(e+ek+Q+q) <= 2^(gamma+q) <= 2^q,
			// so extended_significand_type should suffice to hold these numbers
			// Also, the integer part of 2^(e+q) * 10^k is nothing but the
			// first q + beta bits of tilde{phi_k}.

			extended_significand_type round_down;
			auto divisor = extended_significand_type(1);
			for (int i = 0; i < kappa; ++i) {
				divisor *= 10;
			}

			auto const& cache = get_cache<float_type>(k);
			assert(-beta < common_info<float_type>::extended_precision);
			extended_significand_type integer_part;
			if constexpr (sizeof(float_type) == 4) {
				integer_part = extended_significand_type(cache >> common_info<float_type>::extended_precision);
				integer_part >>= -beta;
			}
			else {
				static_assert(sizeof(float_type) == 8);
				integer_part = cache.high() >> -beta;
			}

			auto const a = integer_part / divisor;
			auto const bi = integer_part % divisor;
			assert(a > 0);

			// a-1 is even
			if (a % 2 == 1) {
				// Check if the remainder is zero
				if (bi == 0) {
					// Fractional part is zero if and only if 2^(e+q) * 10^k is an integer
					if (e + common_info<float_type>::extended_precision + k >= 0 && k >= 0) {
						round_down = (a - 1) / 2;
					}
					else {
						round_down = (a - 1) / 2 + 1;
					}
				}
				else {
					round_down = (a - 1) / 2 + 1;
				}
			}
			// a-1 is odd
			else {
				round_down = (a - 1) / 2 + 1;
			}

			// Measure the distance
			auto const distance = right_bdy - round_down;
			if (distance >= 6 && kappa != 0) {
				std::cout << "distance = " << distance
					<< " (e = " << e << ", x = ";
				std::cout << std::hex << std::setfill('0');				

				if constexpr (sizeof(float_type) == 4) {
					std::cout << std::setprecision(9) << x << " [0x" << std::setw(8);
					success = false;
				}
				else {
					static_assert(sizeof(float_type) == 8);
					std::cout << std::setprecision(17) << x << " [0x" << std::setw(16);
					if (distance >= 7) {
						success = false;
					}
				}

				std::cout << bit_rep << "])\n" << std::dec;
			}
		}

		return success;
	};

	if (verify_single_type(common_info<float>{}))
		std::cout << "correct rounding bound computation for binary32: verified." << std::endl;
	else
		std::cout << "correct rounding bound computation for binary32: failed." << std::endl;

	if (verify_single_type(common_info<double>{}))
		std::cout << "correct rounding bound computation for binary64: verified." << std::endl;
	else
		std::cout << "correct rounding bound computation for binary64: failed." << std::endl;

	std::cout << "Done.\n\n\n";
}