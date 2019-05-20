#include "grisu_exact.h"

#include <iostream>

void verify_kappa_lower_bound()
{
	using namespace jkj::grisu_exact_detail;
	constexpr auto fplus = float_type_info<float>::sign_bit_mask | float_type_info<float>::boundary_bit;
	constexpr auto edge_case_boundary_bit = float_type_info<float>::edge_case_boundary_bit;
	
	bool succeeded = true;

	for (int k = float_type_info<float>::min_k;
		k <= float_type_info<float>::max_k; ++k)
	{
		auto cache = get_cache<float>(k);
		auto delta1 = compute_mul_helper<float>::compute_delta(edge_case_boundary_bit, cache, 0).first;
		auto z1 = compute_mul_helper<float>::compute_mul(fplus, cache, 0).first;

		if (z1 % 100 >= delta1) {
			std::cout << "Error case detected [k = " << k
				<< ", z1 = " << z1 << ", delta1 = " << delta1 << "]\n";
			succeeded = false;
		}
	}

	if (succeeded)
		std::cout << "lower bound for kappa: verified." << std::endl;
}