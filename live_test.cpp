#include "grisu_exact.h"
#include "print_unsigned.h"

template <class Float>
jkj::signed_fp_t<Float> decompose_float(Float x) {
	using float_type_info = jkj::grisu_exact_detail::float_type_info<Float>;
	jkj::signed_fp_t<Float> ret_value;

	std::memcpy(&ret_value.significand, &x, sizeof(Float));

	ret_value.is_negative = (ret_value.significand & float_type_info::sign_bit_mask) != 0;
	ret_value.exponent = ((ret_value.significand << 1) >> (float_type_info::precision + 1));
	ret_value.significand <<= (float_type_info::extended_precision -
		float_type_info::precision - 1);

	// Deal with normal/subnormal dichotomy
	ret_value.significand |= (ret_value.exponent == 0 ? 0 : float_type_info::sign_bit_mask);

	ret_value.exponent += (float_type_info::exponent_bias - float_type_info::extended_precision + 1);

	// x should be a finite number
	assert(ret_value.exponent != 1 - float_type_info::exponent_bias);

	return ret_value;
}

#include <iostream>
#include <iomanip>
#include <string>
#include <charconv>

void live_test()
{
	char buffer1[41];
	char buffer2[40];
	char* const last_letter1 = buffer1 + sizeof(buffer1) - 1;
	*last_letter1 = '\0';

	using float_type = double;

	while (true) {
		float_type x;
		std::string x_str;
		while (true) {
			std::getline(std::cin, x_str);
			auto ret = std::from_chars(x_str.c_str(), x_str.c_str() + x_str.length(), x);

			if (std::make_error_code(ret.ec) || *ret.ptr != '\0') {
				std::cout << "Not a valid input; input again.\n";
				continue;
			}
			break;
		}

		auto xx = decompose_float(x);
		std::cout << "          sign: " << (xx.is_negative ? "-" : "+") << std::endl;
		std::cout << "      exponent: " << xx.exponent << std::endl;
		std::cout << "   significand: " << "0x"
			<< std::hex << xx.significand << std::dec << std::endl;
		std::cout << " Grisu-Exact output: ";

		{
			auto g = jkj::grisu_exact(x);

			auto dst_ptr = &buffer2[0];
			auto src_ptr = last_letter1;

			unsigned char two_digits;
			while (g.significand >= 100) {
				two_digits = (unsigned char)(g.significand % 100);
				g.significand /= 100;
				src_ptr -= 2;
				std::memcpy(src_ptr,
					&jkj::print_unsigned_detail::radix_100_table[two_digits * 2], 2);
			}

			auto written = last_letter1 - src_ptr;

			if (g.is_negative)
				* (dst_ptr++) = '-';

			if (g.significand < 10) {
				if (src_ptr != last_letter1) {
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
				src_ptr = jkj::print_unsigned(unsigned(-g.exponent), last_letter1);
				std::memcpy(dst_ptr, "e-", 2);
				std::memcpy(dst_ptr += 2, src_ptr, last_letter1 - src_ptr + 1);
			}
			else if (g.exponent > 0) {
				src_ptr = jkj::print_unsigned(unsigned(g.exponent), last_letter1);
				std::memcpy(dst_ptr, "e+", 2);
				std::memcpy(dst_ptr += 2, src_ptr, last_letter1 - src_ptr + 1);
			}
			else
				*dst_ptr = '\0';
		}

		std::cout << buffer2 << std::endl;
	}
}