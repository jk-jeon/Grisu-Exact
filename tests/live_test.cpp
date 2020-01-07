#include "../fp_to_chars.h"

template <class Float>
jkj::signed_fp_t<Float> decompose_float(Float x) {
	using common_info = jkj::grisu_exact_detail::common_info<Float>;
	jkj::signed_fp_t<Float> ret_value;

	std::memcpy(&ret_value.significand, &x, sizeof(Float));

	ret_value.is_negative = (ret_value.significand & common_info::sign_bit_mask) != 0;
	ret_value.exponent = ((ret_value.significand << 1) >> (common_info::precision + 1));
	ret_value.significand <<= (common_info::extended_precision -
		common_info::precision - 1);

	// Deal with normal/subnormal dichotomy
	ret_value.significand |= (ret_value.exponent == 0 ? 0 : common_info::sign_bit_mask);

	ret_value.exponent += (common_info::exponent_bias - common_info::extended_precision + 1);

	// x should be a finite number
	assert(ret_value.exponent != 1 - common_info::exponent_bias);

	return ret_value;
}

#include <iostream>
#include <iomanip>
#include <string>
#include <charconv>

template <class Float>
void live_test()
{
	char buffer[41];

	while (true) {
		Float x;
		std::string x_str;
		while (true) {
			std::getline(std::cin, x_str);
			auto ret = std::from_chars(x_str.c_str(), x_str.c_str() + x_str.length(), x);

			if (std::make_error_code(ret.ec) || *ret.ptr != '\0') {
				std::cout << "Not a valid input; input again.\n";
				continue;
			}
			break;
		}

		auto xx = decompose_float(x);
		//std::cout << "               sign: " << (xx.is_negative ? "-" : "+") << std::endl;
		std::cout << "           exponent: " << xx.exponent << std::endl;
		std::cout << "        significand: " << "0x" << std::hex << std::setfill('0');
		if constexpr (sizeof(Float) == 4)
			std::cout << std::setw(8);
		else
			std::cout << std::setw(16);
		std::cout << xx.significand << std::dec << std::endl;

		jkj::fp_to_chars(x, buffer);
		std::cout << " Grisu-Exact output: " << buffer << std::endl;
	}
}

void live_test_float() {
	std::cout << "[Start live test for float's]\n";
	live_test<float>();
}
void live_test_double() {
	std::cout << "[Start live test for double's]\n";
	live_test<double>();
}