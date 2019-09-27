#include "print_float.h"
#include <random>
#include <charconv>

template <class Float, class RandGen>
Float uniformly_randomly_generate_float(RandGen& rg)
{
	using common_info = jkj::grisu_exact_detail::common_info<Float>;
	using extended_significand_type =
		typename common_info::extended_significand_type;
	using uniform_distribution = std::uniform_int_distribution<extended_significand_type>;

	// Generate sign bit
	auto sign_bit = uniform_distribution{ 0, 1 }(rg);

	// Generate exponent bits
	auto exponent_bits = uniform_distribution{ 0,
		(extended_significand_type(1) << common_info::exponent_bits) - 2 }(rg);

	// Generate significand bits
	auto significand_bits = uniform_distribution{ 0,
		(extended_significand_type(1) << common_info::precision) - 1 }(rg);

	auto bit_representation = (sign_bit << (common_info::extended_precision - 1))
		| (exponent_bits << (common_info::precision))
		| significand_bits;

	Float ret;
	std::memcpy(&ret, &bit_representation, sizeof(Float));
	return ret;
}

#include <iostream>
#include <iomanip>

template <class Float, class TypenameString>
void uniform_random_test(std::size_t number_of_tests, TypenameString&& type_name_string)
{
	using extended_significand_type =
		typename jkj::grisu_exact_detail::common_info<Float>::extended_significand_type;

	char buffer[41];
	std::mt19937_64 rg{ std::random_device{}() };
	bool succeeded = true;
	for (std::size_t test_idx = 0; test_idx < number_of_tests; ++test_idx) {
		auto x = uniformly_randomly_generate_float<Float>(rg);
		
		Float roundtrip;
		std::from_chars(buffer, print_float(x, buffer), roundtrip);

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
	uniform_random_test<float>(number_of_tests, "float");
}
void uniform_random_test_double(std::size_t number_of_tests) {
	uniform_random_test<double>(number_of_tests, "double");
}