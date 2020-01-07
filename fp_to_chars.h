#ifndef __JKJ_FP_TO_CHARS__
#define __JKJ_FP_TO_CHARS__

#include "grisu_exact.h"

namespace jkj {
	namespace fp_to_chars_detail {
		char* float_to_chars(unsigned_fp_t<float> v, char* buffer);
		char* double_to_chars(unsigned_fp_t<double> v, char* buffer);
	}

	// Returns the next-to-end position
	template <class Float,
		class RoundingMode = grisu_exact_rounding_modes::nearest_to_even,
		class CorrectRoundingSearch = grisu_exact_correct_rounding::tie_to_up
	>
	char* fp_to_chars_n(Float x, char* buffer,
		RoundingMode&& rounding_mode = {},
		CorrectRoundingSearch&& crs = {})
	{
		// Define case handler
		auto case_handler = [](auto&& dispatcher, char* buffer) {
			auto const& br = static_cast<jkj::bit_representation_t<Float> const&>(dispatcher);
			if (br.is_finite()) {
				if (br.is_negative()) {
					*buffer = '-';
					++buffer;
				}
				if ((br.f << 1) != 0) {
					if constexpr (sizeof(Float) == 4) {
						return fp_to_chars_detail::float_to_chars(dispatcher(), buffer);
					}
					else {
						return fp_to_chars_detail::double_to_chars(dispatcher(), buffer);
					}
				}
				else {
					std::memcpy(buffer, "0E0", 3);
					return buffer + 3;
				}
			}
			else{
				if ((br.f << (grisu_exact_detail::common_info<Float>::exponent_bits + 1)) != 0)
				{
					std::memcpy(buffer, "NaN", 3);
					return buffer + 3;
				}
				else {
					if (br.is_negative()) {
						*buffer = '-';
						++buffer;
					}
					std::memcpy(buffer, "Infinity", 8);
					return buffer + 8;
				}
			}
		};

		return grisu_exact<false>(x, case_handler,
			std::forward<RoundingMode>(rounding_mode),
			std::forward<CorrectRoundingSearch>(crs),
			buffer);
	}

	// Null-terminate and bypass the return value of fp_to_chars_n
	template <class Float,
		class RoundingMode = grisu_exact_rounding_modes::nearest_to_even,
		class CorrectRoundingSearch = grisu_exact_correct_rounding::tie_to_up
	>
	char* fp_to_chars(Float x, char* buffer,
		RoundingMode&& rounding_mode = {},
		CorrectRoundingSearch&& crs = {})
	{
		auto ptr = fp_to_chars_n(x, buffer,
			std::forward<RoundingMode>(rounding_mode),
			std::forward<CorrectRoundingSearch>(crs));
		*ptr = '\0';
		return ptr;
	}
}

#endif