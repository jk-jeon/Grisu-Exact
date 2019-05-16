#include "bigint.h"
#include <utility>
#include <vector>

using namespace jkj::grisu_exact_detail;

// Min-Max Euclid
template <std::size_t max_bits>
std::pair<bigint<max_bits>, bigint<max_bits>>
minmax_euclid(bigint<max_bits> const& a, bigint<max_bits> const& b, std::uint64_t M)
{
	std::pair<bigint<max_bits>, bigint<max_bits>> ret;
	auto& ret_min = ret.first;
	auto& ret_max = ret.second;
	ret_max = b;

	// a * si - b * ti = ai
	// b * vi - a * ui = bi
	std::vector<bigint<max_bits>> ai{ a };
	std::vector<bigint<max_bits>> bi{ b };
	std::vector<std::uint64_t> si{ 1 };
	std::vector<std::uint64_t> ui{ 0 };
	bigint<max_bits> ti{ 0 };
	bigint<max_bits> vi{ 1 };
	
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
			ui.push_back(new_ui.elements[0]);
			vi += q * ti;
		}

		if (bi.back() == 0) {
			ret_min = 1;
			ret_max -= 1;
			return{ ret_min, ret_max };
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
			si.push_back(new_si.elements[0]);
			ti += q * vi;
		}

		if (ai.back() == 0) {
			ret_min = 1;
			ret_max -= 1;
			return{ ret_min, ret_max };
		}
	}
}

template <class Float>
struct max_bits {
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
	static constexpr std::size_t negative_k =
		float_type_info<Float>::extended_precision +
		float_type_info<Float>::max_exponent + 1 + 
		ceil_log10_pow2(-float_type_info<Float>::max_exponent - 1);

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
	static constexpr std::size_t positive_k =
		float_type_info<Float>::precision + 4 -
		float_type_info<Float>::min_exponent;
};

#include <iostream>
#include <fstream>
void verify_cache_length()
{
	// Verify binary32 for negative k
	std::cout << "Verify for IEEE-754 binary32 (float) type for negative k...\n";
	std::ofstream binary32_negative_k_out{ "binary32_negative_k.csv" };

	constexpr auto range = (std::uint64_t(1) << (float_type_info<float>::precision + 2)) - 1;
	constexpr auto q_mp_m2 = float_type_info<float>::extended_precision - float_type_info<float>::precision - 2;

	bigint<max_bits<float>::negative_k> power_of_5 = 1;
	int prev_k = 0;
	for (int e = 3; e <= float_type_info<float>::max_exponent; ++e) {
		auto k = ceil_log10_pow2(-e - 1);
		if (k != prev_k) {
			assert(k == prev_k - 1);
			power_of_5.multiply_5();
		}
		prev_k = k;

		auto [mod_min, mod_max] = minmax_euclid<max_bits<float>::negative_k>(
			bigint<max_bits<float>::negative_k>::power_of_2(q_mp_m2 + e + k),
			power_of_5, range);

		auto divisor = power_of_5;
		divisor -= mod_max;
		auto dividend = power_of_5 * range;
		auto dividend_copy = dividend;
		auto division_res = dividend.long_division(divisor);

		auto log2_res_p1 = division_res.leading_one_pos.element_pos *
			division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;		
		auto u = q_mp_m2 + e + k + log2_res_p1;

		auto edge_case_a = bigint<max_bits<float>::negative_k>::power_of_2(q_mp_m2 + e + k - 1);
		edge_case_a *= range;
		edge_case_a.long_division(power_of_5);

		divisor = power_of_5;
		divisor -= edge_case_a;
		division_res = dividend_copy.long_division(divisor);

		log2_res_p1 = division_res.leading_one_pos.element_pos *
			division_res.element_number_of_bits + division_res.leading_one_pos.bit_pos;
		auto edge_case_result = q_mp_m2 + e + k + log2_res_p1;

		if (edge_case_result > u)
			u = edge_case_result;

		auto bits_required = u + floor_log2_pow10(k) - k  + 1;

		binary32_negative_k_out << e << "," << u << "," << bits_required << std::endl;
	}
}