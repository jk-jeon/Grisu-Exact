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

#include <cmath>
#include <iostream>

void verify_log_computation()
{
	std::cout << "[Verifying log computation...]\n";

	// Verify floor_log10_pow2
	bool succeeded = true;
	for (int e = 1; e <= 65536; ++e) {
		// Take lower 32 bits of 0x4d104d42 * e
		auto lower = (std::int64_t(e) * 0x4d104d42) & 0xffffffff;

		// Verify the lower bits can never overflow
		if (lower + 0x7de8 >= 0x100000000) {
			std::cout << "floor_log10_pow2: overflow detected [e = " << e << "]\n";
			succeeded = false;
		}

		// Verify the result via direct calculation
		if (int(std::floor(e * std::log10(2))) !=
			jkj::grisu_exact_detail::floor_log10_pow2(e))
		{
			std::cout << "floor_log10_pow2: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
		if (int(std::floor(-e * std::log10(2))) !=
			jkj::grisu_exact_detail::floor_log10_pow2(-e))
		{
			std::cout << "floor_log10_pow2: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
	}
	if (succeeded)
		std::cout << "floor_log10_pow2: verified." << std::endl;


	// Verify floor_log2_pow10
	succeeded = true;
	for (int e = 1; e <= 65536; ++e) {
		// Take lower 32 bits of 0x5269e12f * e
		auto lower = (std::int64_t(e) * 0x5269e12f) & 0xffffffff;

		// Verify the lower bits can never overflow
		if (lower + 0x346f >= 0x100000000) {
			std::cout << "floor_log2_pow10: overflow detected [e = " << e << "]\n";
			succeeded = false;
		}

		// Verify the result via direct calculation
		if (int(std::floor(e * std::log2(10))) !=
			jkj::grisu_exact_detail::floor_log2_pow10(e))
		{
			std::cout << "floor_log2_pow10: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
		if (int(std::floor(-e * std::log2(10))) !=
			jkj::grisu_exact_detail::floor_log2_pow10(-e))
		{
			std::cout << "floor_log2_pow10: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
	}
	if (succeeded)
		std::cout << "floor_log2_pow10: verified." << std::endl;


	// Verify floor_log5_pow2
	succeeded = true;
	for (int e = 1; e <= 65536; ++e) {
		// Take lower 32 bits of 0x6e40d1a4 * e
		auto lower = (std::int64_t(e) * 0x6e40d1a4) & 0xffffffff;

		// Verify the lower bits can never overflow
		if (lower + 0x143e >= 0x100000000) {
			std::cout << "floor_log5_pow2: overflow detected [e = " << e << "]\n";
			succeeded = false;
		}

		// Verify the result via direct calculation
		if (int(std::floor(e * (std::log(2) / std::log(5)))) !=
			jkj::grisu_exact_detail::floor_log5_pow2(e))
		{
			std::cout << "floor_log5_pow2: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
		if (int(std::floor(-e * (std::log(2) / std::log(5)))) !=
			jkj::grisu_exact_detail::floor_log5_pow2(-e))
		{
			std::cout << "floor_log5_pow2: mismatch! [e = " << e << "]\n";
			succeeded = false;
		}
	}
	if (succeeded)
		std::cout << "floor_log5_pow2: verified." << std::endl;

	std::cout << "Done.\n\n\n";
}