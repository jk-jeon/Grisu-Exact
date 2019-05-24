#ifndef __PRINT_FLOAT__
#define __PRINT_FLOAT__

//#include "grisu_exact.h"
#include "grisu_exact.h"
#include "print_unsigned.h"

template <class Float>
char* print_float(Float x, char* dst_ptr) {
	char temp_buffer[41];
	char* const temp_last_letter = temp_buffer + sizeof(temp_buffer) - 1;
	*temp_last_letter = '\0';

	auto g = jkj::grisu_exact(x);

	auto src_ptr = temp_last_letter;

	unsigned char two_digits;
	while (g.significand >= 100) {
		two_digits = (unsigned char)(g.significand % 100);
		g.significand /= 100;
		src_ptr -= 2;
		std::memcpy(src_ptr,
			&jkj::print_unsigned_detail::radix_100_table[two_digits * 2], 2);
	}

	auto written = temp_last_letter - src_ptr;

	if (g.is_negative)
		* (dst_ptr++) = '-';

	if (g.significand < 10) {
		if (src_ptr != temp_last_letter) {
			*(dst_ptr++) = jkj::print_unsigned_detail::radix_100_table[g.significand * 2 + 1];
			*(dst_ptr++) = '.';
		}
		else {
			*(dst_ptr++) = jkj::print_unsigned_detail::radix_100_table[g.significand * 2 + 1];
		}
	}
	else {
		*(dst_ptr++) = jkj::print_unsigned_detail::radix_100_table[g.significand * 2];
		*(dst_ptr++) = '.';
		*(dst_ptr++) = jkj::print_unsigned_detail::radix_100_table[g.significand * 2 + 1];
		++g.exponent;
	}
	std::memcpy(dst_ptr, src_ptr, written);
	dst_ptr += written;
	g.exponent += int(written);

	if (g.exponent < 0) {
		src_ptr = jkj::print_unsigned(unsigned(-g.exponent), temp_last_letter);
		std::memcpy(dst_ptr, "e-", 2);
		std::memcpy(dst_ptr += 2, src_ptr, temp_last_letter - src_ptr + 1);
		return dst_ptr + (temp_last_letter - src_ptr + 1);
	}
	else if (g.exponent > 0) {
		src_ptr = jkj::print_unsigned(unsigned(g.exponent), temp_last_letter);
		std::memcpy(dst_ptr, "e+", 2);
		std::memcpy(dst_ptr += 2, src_ptr, temp_last_letter - src_ptr + 1);
		return dst_ptr + (temp_last_letter - src_ptr + 1);
	}
	else {
		*dst_ptr = '\0';
		return dst_ptr;
	}
}

#endif