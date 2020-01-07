#include "../grisu_exact.h"

#include <iostream>

void verify_delta_computation()
{
	std::cout << "[Verifying delta computation...]\n";

	using namespace jkj::grisu_exact_detail;

	auto verify_single_type = [](auto type_tag, auto&& type_name_string) {
		using float_type = typename decltype(type_tag)::float_type;

		constexpr auto fdelta =
			(typename common_info<float_type>::extended_significand_type)(3) <<
			(common_info<float_type>::extended_precision - common_info<float_type>::precision - 3);

		for (int k = common_info<float_type>::min_k;
			k <= common_info<float_type>::max_k; ++k)
		{
			auto cache = get_cache<float_type>(k);

			auto deltai_orthodox
				= compute_mul_helper<float_type>::compute_mul(fdelta, cache, -common_info<float_type>::gamma);

			auto deltai_fast
				= compute_mul_helper<float_type>::compute_delta<jkj::grisu_exact_rounding_modes::to_nearest_tag>(
					true, cache, -common_info<float_type>::gamma);

			if (deltai_orthodox != deltai_fast) {
				std::cout << "compute_delta_edge<" << type_name_string
					<< ">: mismatch! [k = " << k << ", correct deltai = "
					<< deltai_orthodox << ", computed deltai = ("
					<< deltai_fast << "]\n";

				return false;
			}
		}

		return true;
	};

	if (verify_single_type(common_info<float>{}, "float"))
		std::cout << "delta computation for binary32: verified." << std::endl;
	else
		std::cout << "delta computation for binary32: failed." << std::endl;

	if (verify_single_type(common_info<double>{}, "double"))
		std::cout << "delta computation for binary64: verified." << std::endl;
	else
		std::cout << "delta computation for binary64: failed." << std::endl;

	std::cout << "Done.\n\n\n";
}