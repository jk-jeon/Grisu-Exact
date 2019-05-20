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

		for (int k = float_type_info<float_type>::min_k;
			k <= float_type_info<float_type>::max_k; ++k)
		{
			auto cache = get_cache<float_type>(k);

			auto prepared_orthodox = compute_mul<float_type>::prepare_mul(fdelta, cache);
			auto delta11_orthodox = compute_mul<float_type>::extract_mul_upper(prepared_orthodox);
			auto delta12_orthodox = compute_mul<float_type>::extract_mul_lower(prepared_orthodox, 3);

			auto prepared_fast = compute_mul<float_type>::prepare_delta_edge(cache);
			auto delta11_fast = compute_mul<float_type>::extract_delta11(prepared_fast);
			auto delta12_fast = compute_mul<float_type>::extract_delta12(prepared_fast, 3);

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