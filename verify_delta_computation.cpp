#include "grisu_exact.h"

#include <iostream>

void verify_delta_computation()
{
	using namespace jkj::grisu_exact_detail;

	auto verify_single_type = [](auto type_tag, auto&& type_name_string) {
		using float_type = typename decltype(type_tag)::float_type;

		constexpr auto fdelta =
			(typename float_type_info<float_type>::extended_significand_type)(3) <<
			(float_type_info<float_type>::extended_precision - float_type_info<float_type>::precision - 3);
		constexpr auto edge_case_boundary_bit = float_type_info<float_type>::edge_case_boundary_bit;

		for (int k = float_type_info<float_type>::min_k;
			k <= float_type_info<float_type>::max_k; ++k)
		{
			auto cache = get_cache<float_type>(k);

			auto [delta11_orthodox, delta12_orthodox]
				= compute_mul_helper<float_type>::compute_mul(fdelta, cache, 3);

			auto [delta11_fast, delta12_fast]
				= compute_mul_helper<float_type>::compute_delta(edge_case_boundary_bit, cache, 3);

			if (delta11_orthodox != delta11_fast || delta12_orthodox != delta12_fast) {
				std::cout << "compute_delta_edge<" << type_name_string
					<< ">: mismatch! [k = " << k << ", correct delta1 = ("
					<< delta11_orthodox << ", " << delta12_orthodox << "), computed delta1 = ("
					<< delta11_fast << ", " << delta12_fast << ")]\n";

				return false;
			}
		}

		return true;
	};

	if (verify_single_type(float_type_info<float>{}, "float"))
		std::cout << "delta computation for binary32: verified." << std::endl;

	if (verify_single_type(float_type_info<double>{}, "double"))
		std::cout << "delta computation for binary64: verified." << std::endl;
}