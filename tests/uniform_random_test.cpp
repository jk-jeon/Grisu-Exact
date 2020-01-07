#include "../fp_to_chars.h"
#include "random_float.h"
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string_view>

template <class Float, class TypenameString>
void uniform_random_test(std::size_t number_of_tests, TypenameString&& type_name_string)
{
	using extended_significand_type =
		typename jkj::grisu_exact_detail::common_info<Float>::extended_significand_type;

	char buffer[41];
	auto rg = generate_correctly_seeded_mt19937_64();
	bool succeeded = true;
	for (std::size_t test_idx = 0; test_idx < number_of_tests; ++test_idx) {
		auto x = uniformly_randomly_generate_general_float<Float>(rg);
		
		Float roundtrip;
		std::from_chars(buffer, jkj::fp_to_chars(x, buffer), roundtrip);

		extended_significand_type bits_original, bits_roundtrip;
		std::memcpy(&bits_original, &x, sizeof(Float));
		std::memcpy(&bits_roundtrip, &roundtrip, sizeof(Float));

		if (bits_original != bits_roundtrip) {
			std::cout << "Roundtrip error detected! [original = "
				<< std::setprecision(40)
				<< x << " (0x" << std::hex << std::setfill('0');
			if constexpr (sizeof(Float) == 4)
				std::cout << std::setw(8);
			else
				std::cout << std::setw(16);
			std::cout << bits_original << std::dec << "), roundtrip = "
				<< std::setprecision(40)
				<< roundtrip << " (0x" << std::hex << std::setfill('0');
			if constexpr (sizeof(Float) == 4)
				std::cout << std::setw(8);
			else
				std::cout << std::setw(16);
			std::cout << bits_roundtrip << std::dec << ")]\n";
			succeeded = false;
		}


	}

	if (succeeded) {
		std::cout << "Uniform random test for " << type_name_string
			<< " with " << number_of_tests << " examples succeeded.\n";
	}
}

void uniform_random_test_float(std::size_t number_of_tests) {
	std::cout << "[Testing uniformly randomly generated float inputs...]\n";
	uniform_random_test<float>(number_of_tests, "float");
	std::cout << "Done.\n\n\n";
}
void uniform_random_test_double(std::size_t number_of_tests) {
	std::cout << "[Testing uniformly randomly generated double inputs...]\n";
	uniform_random_test<double>(number_of_tests, "double");
	std::cout << "Done.\n\n\n";
}