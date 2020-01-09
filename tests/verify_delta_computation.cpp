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