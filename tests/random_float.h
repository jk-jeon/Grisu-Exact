#pragma once
#include "../grisu_exact.h"
#include <cstring>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

// For correct seeding
class repeating_seed_seq {
public:
	using result_type = std::uint32_t;

	repeating_seed_seq() : stored_values{ 0 } {}
	template <class InputIterator>
	repeating_seed_seq(InputIterator first, InputIterator last) :
		stored_values(first, last) {}
	template <class T>
	repeating_seed_seq(std::initializer_list<T> list) :
		stored_values(list) {}

	repeating_seed_seq(std::random_device&& rd, std::size_t count) {
		stored_values.resize(count);
		for (auto& elem : stored_values)
			elem = rd();
	}

	template <class RandomAccessIterator>
	void generate(RandomAccessIterator first, RandomAccessIterator last)
	{
		auto count = last - first;
		auto q = count / stored_values.size();
		for (std::size_t i = 0; i < q; ++i) {
			std::copy_n(stored_values.cbegin(), stored_values.size(), first);
			first += stored_values.size();
		}
		count -= q * stored_values.size();
		std::copy_n(stored_values.cbegin(), count, first);
	}

	std::size_t size() const noexcept {
		return stored_values.size();
	}

	template <class OutputIterator>
	void param(OutputIterator first) const {
		std::copy(stored_values.begin(), stored_values.end(), first);
	}

private:
	std::vector<std::uint32_t>		stored_values;
};

inline std::mt19937_64 generate_correctly_seeded_mt19937_64()
{
	repeating_seed_seq seed_seq{ std::random_device{},
		std::mt19937_64::state_size * std::mt19937_64::word_size / (sizeof(std::uint32_t) * 8) };
	return std::mt19937_64{ seed_seq };
}

template <class Float, class RandGen>
Float uniformly_randomly_generate_finite_float(RandGen& rg)
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

template <class Float, class RandGen>
Float uniformly_randomly_generate_general_float(RandGen& rg)
{
	using common_info = jkj::grisu_exact_detail::common_info<Float>;
	using extended_significand_type =
		typename common_info::extended_significand_type;
	using uniform_distribution = std::uniform_int_distribution<extended_significand_type>;

	// Generate sign bit
	auto bit_representation = uniform_distribution{
		0, std::numeric_limits<extended_significand_type>::max() }(rg);
	Float ret;
	std::memcpy(&ret, &bit_representation, sizeof(Float));
	return ret;
}

// This function tries to uniformly randomly generate a float number with the
// given number of decimal digits, and the end-result is not perfectly bias-free.
// However, I don't think there is an easy way to do it correctly.
template <class Float, class RandGen>
Float randomly_generate_float_with_given_digits(unsigned int digits, RandGen& rg)
{
	using common_info = jkj::grisu_exact_detail::common_info<Float>;
	using extended_significand_type =
		typename common_info::extended_significand_type;
	using signed_int_t = std::make_signed_t<extended_significand_type>;

	assert(digits >= 1);
	if constexpr (std::is_same_v<Float, float>) {
		assert(digits <= 9);
	}
	else {
		static_assert(std::is_same_v<Float, double>);
		assert(digits <= 17);
	}

	// Generate exponent uniformly randomly
	auto exp = std::uniform_int_distribution<int>{
		std::numeric_limits<Float>::min_exponent10 - (int(digits) - 1),
		std::numeric_limits<Float>::max_exponent10 - (int(digits) - 1) }(rg);

	Float result;
	while (true) {
		// Try to generate significand uniformly randomly
		signed_int_t from = 1;
		for (unsigned int e = 1; e < digits; ++e) {
			from *= 10;
		}
		auto to = from * 10 - 1;
		// Allow 0 to be generated
		if (from == 1)
			--from;
		auto significand = std::uniform_int_distribution<signed_int_t>{ from, to }(rg);

		// Generate sign uniformly randomly
		if (std::uniform_int_distribution<int>{ 0, 1 }(rg) != 0)
			significand *= -1;

		// Cook up
		auto str = std::to_string(significand) + 'e' + std::to_string(exp);

		try {
			if constexpr (std::is_same_v<Float, float>)
				result = std::stof(str);
			else
				result = std::stod(str);
		}
		catch (std::out_of_range&) {
			continue;
		}
		break;
	}
	
	return result;
}