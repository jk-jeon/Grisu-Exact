// Copyright 2020 Junekey Jeon
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the MIT License:
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this softwareand associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//  
//  The above copyright noticeand this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "bigint.h"
#include <algorithm>
#include <vector>

namespace jkj {
	namespace grisu_exact_detail {
		// Min-Max Euclid
		// Precondition: a and b are coprime; otherwise, the result is incorrect
		template <std::size_t array_size>
		std::pair<bigint_impl<array_size>, bigint_impl<array_size>>
			minmax_euclid(bigint_impl<array_size> const& a, bigint_impl<array_size> const& b, std::uint64_t N)
		{
			using bigint_t = bigint_impl<array_size>;

			std::pair<bigint_t, bigint_t> ret;
			auto& ret_min = ret.first;
			auto& ret_max = ret.second;
			ret_max = b;

			// a * si - b * ti = ai
			// b * vi - a * ui = bi
			std::vector<bigint_t> ai{ a };
			std::vector<bigint_t> bi{ b };
			std::vector<std::uint64_t> si{ 1 };
			std::vector<std::uint64_t> ui{ 0 };
			bigint_t ti{ 0 };
			bigint_t vi{ 1 };

			while (true) {
				if (bi.back() >= ai.back()) {
					auto new_bi = bi.back();
					auto q = new_bi.long_division(ai.back());
					auto new_ui = q * si.back();
					new_ui += ui.back();

					if (new_ui >= N) {
						ret_min = ai.back();
						ret_max -= bi.back();

						auto remaining = N - ui.back();
						auto j = ai.size();
						while (remaining > 0) {
							assert(j != 0);
							--j;

							auto qr = remaining / si[j];
							remaining %= si[j];

							auto new_max_candidate = ai[j];
							new_max_candidate *= qr;
							new_max_candidate += ret_max;

							if (new_max_candidate < b)
								ret_max = new_max_candidate;
							else {
								auto margin = b;
								margin -= ret_max;
								q = margin.long_division(ai[j]);
								ret_max += q * ai[j];

								return ret;
							}
						}
						return ret;
					}

					bi.push_back(new_bi);
					assert(new_ui.leading_one_pos.element_pos == 0);
					ui.push_back(new_ui.elements[0]);
					vi += q * ti;
				}

				if (bi.back() == 0) {
					ret_min = 0;
					ret_max -= 1;
					return ret;
				}

				if (ai.back() >= bi.back()) {
					auto new_ai = ai.back();
					auto q = new_ai.long_division(bi.back());
					auto new_si = q * ui.back();
					new_si += si.back();

					if (new_si >= N) {
						ret_min = ai.back();
						ret_max -= bi.back();

						auto remaining = N - si.back();
						auto j = bi.size();
						while (remaining > 0) {
							assert(j != 0);
							--j;

							if (ui[j] != 0) {
								auto qr = remaining / ui[j];
								remaining %= ui[j];

								auto new_min_diff = bi[j];
								new_min_diff *= qr;

								if (new_min_diff < ret_min)
									ret_min -= new_min_diff;
								else {
									ret_min.long_division(bi[j]);
									return ret;
								}
							}
							else {
								ret_min.long_division(bi[j]);
								return ret;
							}
						}
						return ret;
					}

					ai.push_back(new_ai);
					assert(new_si.leading_one_pos.element_pos == 0);
					si.push_back(new_si.elements[0]);
					ti += q * vi;
				}

				if (ai.back() == 0) {
					ret_min = 0;
					ret_max -= 1;
					return ret;
				}
			}
		}

		template <class Float>
		struct further_info {
			// When k < 0,
			// we should be able to hold 5^-k and 2^(q + e + k + 1).
			// For the former, the necessary number of bits are
			// floor(-k * log2(5)) + 1 = floor(-k * log2(10)) + k + 1,
			// and for the latter, the necessary number of bits are q + e + k + 2.
			// Since k = ceil((alpha-e-1) * log10(2)), we can show that
			// floor(-k * log2(10)) <= e + 1 - alpha, so
			// the necessary bits for the former is at most e + k + 2 - alpha.
			// On the other hand, e + k is an increasing function of e, so
			// the following is an upper bound:
			static constexpr std::size_t negative_k_max_bits =
				std::max(int(common_info<Float>::extended_precision + 2),
					int(2 - common_info<Float>::alpha)) +
				common_info<Float>::max_exponent + common_info<Float>::min_k;


			// When k >= 0,
			// we should be able to hold 5^k * 2^(p+2) and 2^(-e - k - (q-p-3)).
			// For the former, the necessary number of bits are
			// floor(k * log2(5)) + p + 3 = floor(k * log2(10)) - k + p + 3,
			// and for the latter, the necessary number of bits are
			// -e - k - (q-p-3) + 1 = -e - k + p + 4 - q.
			// Since k = ceil((alpha-e-1) * log10(2)), we can show that
			// floor(k * log2(10)) <= (alpha-e-1) + log2(10) < alpha - e + 3, so
			// the necessary bits for the former is at most -e - k + p + 5 + alpha.
			// On the other hand, -e - k is a decreasing function on e, so
			// the following is an upper bound:
			static constexpr std::size_t positive_k_max_bits =
				common_info<Float>::precision +
				std::max(-int(common_info<Float>::extended_precision - 4),
					int(5 + common_info<Float>::alpha))
				- common_info<Float>::min_exponent - common_info<Float>::max_k;


			// Useful constants
			static constexpr auto range =
				(std::uint64_t(1) << (common_info<Float>::precision + 2));
			static constexpr auto q_mp_m2 =
				common_info<Float>::extended_precision - common_info<Float>::precision - 2;
		};

		template <class Float, class F1, class F2>
		void verify_cache_length_single_type_negative_k(F1&& on_each, F2&& on_max)
		{
			using bigint_type = bigint<further_info<Float>::negative_k_max_bits>;

			std::size_t max_required_bits = 0;

			bigint_type power_of_5 = 1;
			int prev_k = 0;
			for (int e = common_info<Float>::alpha + 3; e <= common_info<Float>::max_exponent; ++e) {
				auto k = -floor_log10_pow2(e + 1 - common_info<Float>::alpha);
				if (k != prev_k) {
					assert(k == prev_k - 1);
					power_of_5.multiply_5();
					prev_k = k;
				}

				auto [mod_min, mod_max] = minmax_euclid(
					bigint_type::power_of_2(further_info<Float>::q_mp_m2 + e + k),
					power_of_5, further_info<Float>::range);

				auto divisor = power_of_5;
				divisor -= mod_max;
				auto dividend = power_of_5;
				auto division_res = dividend.long_division(divisor);

				auto log2_res_p1 = division_res.leading_one_pos.element_pos *
					division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;

				auto required_bits = common_info<Float>::extended_precision +
					e + floor_log2_pow10(k) + 1 + log2_res_p1;

				std::tie(mod_min, mod_max) = minmax_euclid(
					bigint_type::power_of_2(further_info<Float>::q_mp_m2 + e + k + 2),
					power_of_5, further_info<Float>::range / 2);

				divisor = power_of_5;
				divisor -= mod_max;
				dividend = power_of_5;
				division_res = dividend.long_division(divisor);

				log2_res_p1 = division_res.leading_one_pos.element_pos *
					division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;

				auto two_y_result = common_info<Float>::extended_precision +
					e + floor_log2_pow10(k) + 1 + log2_res_p1;

				if (two_y_result > required_bits)
					required_bits = two_y_result;
 
				auto edge_case_a = bigint_type::power_of_2(further_info<Float>::q_mp_m2 + e + k - 1);
				edge_case_a *= (further_info<Float>::range - 1);
				edge_case_a.long_division(power_of_5);

				divisor = power_of_5;
				divisor -= edge_case_a;
				dividend = power_of_5;
				division_res = dividend.long_division(divisor);

				log2_res_p1 = division_res.leading_one_pos.element_pos *
					division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;

				auto edge_case_result = common_info<Float>::extended_precision +
					e + floor_log2_pow10(k) + log2_res_p1;

				if (edge_case_result > required_bits)
					required_bits = edge_case_result;

				if (required_bits > max_required_bits)
					max_required_bits = required_bits;

				on_each(e, required_bits);
			}

			on_max(max_required_bits);
		}

		template <class Float, class F1, class F2>
		void verify_cache_length_single_type_positive_k(F1&& on_each, F2&& on_max)
		{
			using bigint_type = bigint<further_info<Float>::positive_k_max_bits>;

			std::size_t max_required_bits = 0;

			bigint_type power_of_5 = 1;
			int prev_k = 0;
			for (int e = common_info<Float>::alpha + 2; e >= common_info<Float>::min_exponent; --e) {
				auto k = -floor_log10_pow2(e + 1 - common_info<Float>::alpha);
				if (k != prev_k) {
					assert(k == prev_k + 1);
					power_of_5.multiply_5();
					prev_k = k;
				}

				auto required_bits_base = std::size_t(floor_log2_pow10(k) - k + 1);
				auto required_bits = required_bits_base;

				int exp_of_2 = -e - k - int(further_info<Float>::q_mp_m2);

				if (exp_of_2 > 0) {
					auto [mod_min, mod_max] = minmax_euclid(
						power_of_5,
						bigint_type::power_of_2(exp_of_2),
						further_info<Float>::range);

					if (mod_min.leading_one_pos.bit_pos != 0) {
						auto log2_res = mod_min.leading_one_pos.element_pos *
							mod_min.element_number_of_bits + mod_min.leading_one_pos.bit_pos - 1;

						if (log2_res > common_info<Float>::precision + 2) {
							required_bits -= (log2_res - common_info<Float>::precision - 2);
						}
					}
				}

				exp_of_2 -= 2;
				if (exp_of_2 > 0) {
					auto [mod_min, mod_max] = minmax_euclid(
						power_of_5,
						bigint_type::power_of_2(exp_of_2),
						further_info<Float>::range / 2);

					if (mod_min.leading_one_pos.bit_pos != 0) {
						auto log2_res = mod_min.leading_one_pos.element_pos *
							mod_min.element_number_of_bits + mod_min.leading_one_pos.bit_pos - 1;

						if (log2_res > common_info<Float>::precision + 1) {
							auto two_y_result = required_bits_base - log2_res + common_info<Float>::precision + 1;

							if (two_y_result > required_bits)
								required_bits = two_y_result;
						}
					}
				}

				exp_of_2 += 3;
				if (exp_of_2 > 0) {
					auto edge_case_a = power_of_5;
					edge_case_a *= (further_info<Float>::range - 1);
					edge_case_a.long_division(bigint_type::power_of_2(exp_of_2));

					if (edge_case_a.leading_one_pos.bit_pos != 0) {
						auto log2_res = edge_case_a.leading_one_pos.element_pos *
							edge_case_a.element_number_of_bits + edge_case_a.leading_one_pos.bit_pos - 1;

						if (log2_res > common_info<Float>::precision + 2) {
							auto edge_case_result = required_bits_base - log2_res + common_info<Float>::precision + 2;

							if (edge_case_result > required_bits)
								required_bits = edge_case_result;
						}
					}
				}

				if (required_bits > max_required_bits)
					max_required_bits = required_bits;

				on_each(e, required_bits);
			}

			on_max(max_required_bits);
		}
	}
}

#include <fstream>
#include <iostream>

void verify_cache_length()
{
	std::cout << "[Verifying cache length upper bound...]\n";

	std::ofstream out;
	auto on_each = [&out](auto e, auto required_bits) {
		out << e << "," << required_bits << std::endl;
	};
	auto on_max = [&out](auto max_required_bits) {
		std::cout << "Maximum required bits: " << max_required_bits << std::endl;
	};

	std::cout << "\nVerify for IEEE-754 binary32 (float) type for negative k...\n";	
	out.open("test_results/binary32_negative_k.csv");
	out << "e,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_negative_k<float>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary32 (float) type for positive k...\n";
	out.open("test_results/binary32_positive_k.csv");
	out << "e,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_positive_k<float>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary64 (double) type for negative k...\n";
	out.open("test_results/binary64_negative_k.csv");
	out << "e,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_negative_k<double>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary64 (double) type for positive k...\n";
	out.open("test_results/binary64_positive_k.csv");
	out << "e,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_positive_k<double>(on_each, on_max);
	out.close();

	std::cout << std::endl;
	std::cout << "Done.\n\n\n";
}