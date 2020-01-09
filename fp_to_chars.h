// Copyright 2020 Junekey Jeon
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the MIT License:
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this softwareand associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//  
//  The above copyright noticeand this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

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
		auto br = get_bit_representation(x);
		if (br.is_finite()) {
			if (br.is_negative()) {
				*buffer = '-';
				++buffer;
			}
			if ((br.f << 1) != 0) {
				if constexpr (sizeof(Float) == 4) {
					return fp_to_chars_detail::float_to_chars(grisu_exact<false>(x,
						std::forward<RoundingMode>(rounding_mode),
						std::forward<CorrectRoundingSearch>(crs)), buffer);
				}
				else {
					return fp_to_chars_detail::double_to_chars(grisu_exact<false>(x,
						std::forward<RoundingMode>(rounding_mode),
						std::forward<CorrectRoundingSearch>(crs)), buffer);
				}
			}
			else {
				std::memcpy(buffer, "0E0", 3);
				return buffer + 3;
			}
		}
		else {
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