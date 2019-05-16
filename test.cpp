#include "grisu_exact.h"

#include <iostream>

#include "bigint.h"

//#define VERIFY_LOG_COMPUTATION
extern void verify_log_computation();

#define VERIFY_CACHE_LENGTH
extern void verify_cache_length();

int main()
{
#ifdef VERIFY_LOG_COMPUTATION
	std::cout << "[Start verifying log computation]\n";
	verify_log_computation();
	std::cout << "Done.\n\n";
#endif

#ifdef VERIFY_CACHE_LENGTH
	std::cout << "[Start verifying cache length upper bound]\n";
	verify_cache_length();
	std::cout << "Done.\n\n";
#endif

	jkj::grisu_exact_detail::bigint<64 * 4> a, b;
	a.elements[0] = 0x1234567812345678;
	a.elements[1] = 0x2345678923456789;
	a.elements[2] = 0x3456789a3456789a;
	a.elements[3] = 0x456789ab456789ab;
	a.leading_one_pos.element_pos = 3;
	a.leading_one_pos.bit_pos = 63;

	b.elements[0] = 0x56789abc56789abc;
	b.elements[1] = 0x000000006789abcd;
	b.elements[2] = 0;
	b.elements[3] = 0;
	b.leading_one_pos.element_pos = 1;
	b.leading_one_pos.bit_pos = 31;

	auto q = a.long_division(b);

	auto m = q * b;
	m += a;
}