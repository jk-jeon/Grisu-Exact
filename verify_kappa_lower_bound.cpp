#include "grisu_exact.h"

#include <iostream>

void verify_kappa_lower_bound()
{
	using namespace jkj::grisu_exact_detail;
	constexpr auto fplus = float_type_info<float>::sign_bit_mask | float_type_info<float>::boundary_bit;
	
	bool succeeded = true;

	for (int k = float_type_info<float>::min_k;
		k <= float_type_info<float>::max_k; ++k)
	{
		auto cache = get_cache<float>(k);
		auto delta1 = compute_mul<float>::extract_delta11(compute_mul<float>::prepare_delta_edge(cache));
		auto z1 = compute_mul<float>::extract_mul_upper(compute_mul<float>::prepare_mul(fplus, cache));

		if (z1 % 100 >= delta1) {
			std::cout << "Error case detected [k = " << k
				<< ", z1 = " << z1 << ", delta1 = " << delta1 << "]\n";
			succeeded = false;
		}
	}

	if (succeeded)
		std::cout << "lower bound for kappa: verified." << std::endl;
}