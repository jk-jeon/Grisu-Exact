#include "grisu_exact.h"

#include <cmath>
#include <iostream>

void verify_log_computation()
{
	// Verify ceil_log10_pow2
	bool succeeded = true;
	for (int e = 1; e <= 65536; ++e) {
		// Take lower 32 bits of 0x4d104d42 * e
		auto lower = (std::int64_t(e)* 0x4d104d42) & 0xffffffff;

		// Verify the lower bits can never overflow
		if (lower + 0x7de8 >= 0x100000000) {
			std::cout << "ceil_log10_pow2: overflow detected (e: " << e << ")\n";
			succeeded = false;
		}

		// Verify the result via direct calculation
		if (int(std::ceil(e * std::log10(2))) !=
			jkj::grisu_exact_detail::ceil_log10_pow2(e))
		{
			std::cout << "ceil_log10_pow2: mismatch! (e: " << e << ")\n";
			succeeded = false;
		}
		if (int(std::ceil(-e * std::log10(2))) !=
			jkj::grisu_exact_detail::ceil_log10_pow2(-e))
		{
			std::cout << "ceil_log10_pow2: mismatch! (e: " << e << ")\n";
			succeeded = false;
		}
	}
	if (succeeded)
		std::cout << "ceil_log10_pow2: verified." << std::endl;

	// Verify floor_log2_pow10
	succeeded = true;
	for (int e = 1; e <= 65536; ++e) {
		// Take lower 32 bits of 0x5269e12f * e
		auto lower = (std::int64_t(e)* 0x5269e12f) & 0xffffffff;

		// Verify the lower bits can never overflow
		if (lower + 0x346f >= 0x100000000) {
			std::cout << "floor_log2_pow10: overflow detected (e: " << e << ")\n";
			succeeded = false;
		}

		// Verify the result via direct calculation
		if (int(std::floor(e * std::log2(10))) !=
			jkj::grisu_exact_detail::floor_log2_pow10(e))
		{
			std::cout << "floor_log2_pow10: mismatch! (e: " << e << ")\n";
			succeeded = false;
		}
		if (int(std::floor(-e * std::log2(10))) !=
			jkj::grisu_exact_detail::floor_log2_pow10(-e))
		{
			std::cout << "floor_log2_pow10: mismatch! (e: " << e << ")\n";
			succeeded = false;
		}
	}
	if (succeeded)
		std::cout << "floor_log2_pow10: verified." << std::endl;
}