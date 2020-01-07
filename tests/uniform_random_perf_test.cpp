#include "../fp_to_chars.h"
#include "random_float.h"
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string_view>

template <class Float, class TypenameString>
void uniform_random_perf_test(std::size_t number_of_tests, TypenameString&& type_name_string)
{
	using extended_significand_type =
		typename jkj::grisu_exact_detail::common_info<Float>::extended_significand_type;

	char buffer[41];
	auto rg = generate_correctly_seeded_mt19937_64();
	for (std::size_t test_idx = 0; test_idx < number_of_tests; ++test_idx) {
		auto x = uniformly_randomly_generate_general_float<Float>(rg);
		jkj::fp_to_chars(x, buffer);
	}
}

void uniform_random_perf_test_float(std::size_t number_of_tests) {
	std::cout << "[Running the algorithm with uniformly randomly generated float inputs...]\n";
	uniform_random_perf_test<float>(number_of_tests, "float");
	std::cout << "Done.\n\n\n";
}
void uniform_random_perf_test_double(std::size_t number_of_tests) {
	std::cout << "[Running the algorithm with uniformly randomly generated double inputs...]\n";
	uniform_random_perf_test<double>(number_of_tests, "double");
	std::cout << "Done.\n\n\n";
}