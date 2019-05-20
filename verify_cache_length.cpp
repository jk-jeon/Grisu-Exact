////////////////////////////////////////////////////////////////////////////////////////
// This file is a direct translation of the included MATLAB files
// minmax_euclide_v2.m and verify_cache_length_32/64_positive/negative_k.m
////////////////////////////////////////////////////////////////////////////////////////

#include "bigint.h"
#include <vector>

namespace jkj {
	namespace grisu_exact_detail {
		// Min-Max Euclid
		// Precondition: a and b are coprime; otherwise, the result is incorrect
		template <std::size_t array_size>
		std::pair<bigint_impl<array_size>, bigint_impl<array_size>>
			minmax_euclid(bigint_impl<array_size> const& a, bigint_impl<array_size> const& b, std::uint64_t M)
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

					if (new_ui >= M) {
						ret_min = ai.back();
						ret_max -= bi.back();

						auto remaining = M - ui.back();
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

					if (new_si >= M) {
						ret_min = ai.back();
						ret_max -= bi.back();

						auto remaining = M - si.back();
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
			// we should be able to hold 5^-k and 2^((q-p-2) + e + k)(2^(p+2) - 1).
			// For the former, the necessary number of bits are
			// floor(-k * log2(5)) + 1 = floor(-k * log2(10)) + k + 1,
			// and for the latter, the necessary number of bits are q + e + k.
			// Since k = ceil(-(e+1) * log10(2)), we can show that
			// floor(-k * log2(10)) <= e + 1, so
			// the necessary bits for the former is at most e + k + 2,
			// which is of course smaller than q + e + k.
			// On the other hand, e + k is upper bounded by
			// e - (e+1)log10(2) + 1 = (e+1)(1 - log10(2)), so
			// the following is an upper bound:
			static constexpr std::size_t negative_k_max_bits =
				float_type_info<Float>::extended_precision +
				float_type_info<Float>::max_exponent + 1 -
				floor_log10_pow2(float_type_info<Float>::max_exponent + 1);


			// When k >= 0,
			// we should be able to hold 5^k * (2^(p+2) - 1) and 2^(-e - k - (q-p-3)).
			// For the former, the necessary number of bits are
			// floor(k * log2(5)) + 1 + p + 2 = floor(k * log2(10)) - k + p + 3,
			// and for the latter, the necessary number of bits are
			// -e - k - (q-p-3) + 1 = -e - q - k + p + 4.
			// Since k = ceil(-(e+1) * log10(2)), we can show that
			// floor(k * log2(10)) <= -(e+1) + log2(10) < -e + 3, so
			// the necessary bits for the former is at most -e - k + p + 5,
			// which is of course larger than -e - q - k + p + 4.
			// On the other hand, -e - k is upper bounded by
			// -e - (e+1)log10(2) = -e(1 - log10(2)) - log10(2) < -(e+1)(1 - log10(2)), so
			// the following is an upper bound:
			static constexpr std::size_t positive_k_max_bits =
				float_type_info<Float>::precision + 4 -
				float_type_info<Float>::min_exponent;


			// Useful constants
			static constexpr auto range =
				(std::uint64_t(1) << (float_type_info<Float>::precision + 2)) - 1;
			static constexpr auto q_mp_m2 =
				float_type_info<Float>::extended_precision - float_type_info<Float>::precision - 2;
		};

		template <class Float, class F1, class F2>
		void verify_cache_length_single_type_negative_k(F1&& on_each, F2&& on_max)
		{
			using bigint_type = bigint<further_info<Float>::negative_k_max_bits>;

			std::size_t max_required_bits = 0;

			bigint_type power_of_5 = 1;
			int prev_k = 0;
			for (int e = 3; e <= float_type_info<Float>::max_exponent; ++e) {
				auto k = -floor_log10_pow2(e + 1);
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
				dividend *= further_info<Float>::range;
				auto dividend_copy = dividend;
				auto division_res = dividend.long_division(divisor);

				auto log2_res_p1 = division_res.leading_one_pos.element_pos *
					division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;
				auto u = further_info<Float>::q_mp_m2 + e + k + log2_res_p1;

				auto edge_case_a = bigint_type::power_of_2(further_info<Float>::q_mp_m2 + e + k - 1);
				edge_case_a *= further_info<Float>::range;
				edge_case_a.long_division(power_of_5);

				divisor = power_of_5;
				divisor -= edge_case_a;
				division_res = dividend_copy.long_division(divisor);

				log2_res_p1 = division_res.leading_one_pos.element_pos *
					division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;
				auto edge_case_result = further_info<Float>::q_mp_m2 + e + k - 1 + log2_res_p1;

				if (edge_case_result > u)
					u = edge_case_result;

				auto required_bits = u + floor_log2_pow10(k) - k  + 1;
				if (required_bits > max_required_bits)
					max_required_bits = required_bits;

				on_each(e, u, required_bits);
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
			for (int e = 2; e >= float_type_info<Float>::min_exponent; --e) {
				auto k = -floor_log10_pow2(e + 1);
				if (k != prev_k) {
					assert(k == prev_k + 1);
					power_of_5.multiply_5();
					prev_k = k;
				}

				std::size_t l;
				if (k == 0 || -e - k <= further_info<Float>::q_mp_m2) {
					l = 0;
				}
				else {
					auto [mod_min, mod_max] = minmax_euclid(
						power_of_5,
						bigint_type::power_of_2(-e - k - further_info<Float>::q_mp_m2),
						further_info<Float>::range);

					auto division_res = mod_min.long_division(further_info<Float>::range);
					if (division_res.leading_one_pos.bit_pos != 0) {
						l = division_res.leading_one_pos.element_pos *
							division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos - 1;
					}
					else {
						l = 0;
					}

					auto edge_case_a = power_of_5;
					edge_case_a *= further_info<Float>::range;
					edge_case_a.long_division(bigint_type::power_of_2(-e - k - further_info<Float>::q_mp_m2 + 1));

					division_res = edge_case_a.long_division(further_info<Float>::range);
					if (division_res.leading_one_pos.bit_pos != 0) {
						auto edge_case_result = division_res.leading_one_pos.element_pos *
							division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos - 1;

						if (edge_case_result < l)
							l = edge_case_result;
					}
					else {
						l = 0;
					}
				}

				auto required_bits = floor_log2_pow10(k) - k  + 1 - l;
				if (required_bits > max_required_bits)
					max_required_bits = required_bits;

				on_each(e, l, required_bits);
			}

			on_max(max_required_bits);
		}
	}
}

#include <fstream>
#include <iostream>

void verify_cache_length()
{
	std::ofstream out;
	auto on_each = [&out](auto e, auto u_or_l, auto required_bits) {
		out << e << "," << u_or_l << "," << required_bits << std::endl;
	};
	auto on_max = [&out](auto max_required_bits) {
		std::cout << "Maximum required bits: " << max_required_bits << std::endl;
	};

	std::cout << "\nVerify for IEEE-754 binary32 (float) type for negative k...\n";	
	out.open("binary32_negative_k.csv");
	out << "e,u,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_negative_k<float>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary32 (float) type for positive k...\n";
	out.open("binary32_positive_k.csv");
	out << "e,l,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_positive_k<float>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary64 (double) type for negative k...\n";
	out.open("binary64_negative_k.csv");
	out << "e,u,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_negative_k<double>(on_each, on_max);
	out.close();

	std::cout << "\nVerify for IEEE-754 binary64 (double) type for positive k...\n";
	out.open("binary64_positive_k.csv");
	out << "e,l,required_bits\n";
	jkj::grisu_exact_detail::verify_cache_length_single_type_positive_k<double>(on_each, on_max);
	out.close();

	std::cout << std::endl;
}