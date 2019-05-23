#include "grisu_exact.h"

#include <iostream>

// Check if our log floor/ceil computations are correct
//#define VERIFY_LOG_COMPUTATION
extern void verify_log_computation();

// Find how many bits are needed for each cache entry
//#define VERIFY_CACHE_LENGTH
extern void verify_cache_length();

// Generate all cache entries; check if overflow occurs
//#define GENERATE_CACHE
extern void generate_cache();

// Check if our delta computation is correct
//#define VERIFY_DELTA_COMPUTATION
extern void verify_delta_computation();

// Check if kappa is at least 2 for binary32 with q = 32
//#define VERIFY_KAPPA_LOWER_BOUND
extern void verify_kappa_lower_bound();

// Generate random float's and test Grisu-Exact's output
#define UNIFORM_RANDOM_TEST_FLOAT
static std::size_t number_of_uniform_random_tests_float = 10000000;
extern void uniform_random_test_float(std::size_t number_of_tests);

// Generate random double's and test Grisu-Exact's output
#define UNIFORM_RANDOM_TEST_DOUBLE
static std::size_t number_of_uniform_random_tests_double = 10000000;
extern void uniform_random_test_double(std::size_t number_of_tests);

// Input float's and test Grisu-Exact's output
//#define LIVE_TEST_FLOAT
extern void live_test_float();

// Input double's and test Grisu-Exact's output
//#define LIVE_TEST_DOUBLE
extern void live_test_double();


int main()
{
#ifdef VERIFY_LOG_COMPUTATION
	std::cout << "[Verifying log computation...]\n";
	verify_log_computation();
	std::cout << "Done.\n\n\n";
#endif

#ifdef VERIFY_CACHE_LENGTH
	std::cout << "[Verifying cache length upper bound...]\n";
	verify_cache_length();
	std::cout << "Done.\n\n\n";
#endif

#ifdef GENERATE_CACHE
	std::cout << "[Generating cache...]\n";
	generate_cache();
	std::cout << "Done.\n\n\n";
#endif

#ifdef VERIFY_DELTA_COMPUTATION
	std::cout << "[Verifying delta computation...]\n";
	verify_delta_computation();
	std::cout << "Done.\n\n\n";
#endif

#ifdef VERIFY_KAPPA_LOWER_BOUND
	std::cout << "[Verifying that 2 is a lower bound of kappa...]\n";
	verify_kappa_lower_bound();
	std::cout << "Done.\n\n\n";
#endif

#ifdef UNIFORM_RANDOM_TEST_FLOAT
	std::cout << "[Testing uniformly randomly generated float inputs...]\n";
	uniform_random_test_float(number_of_uniform_random_tests_float);
	std::cout << "Done.\n\n\n";
#endif

#ifdef UNIFORM_RANDOM_TEST_DOUBLE
	std::cout << "[Testing uniformly randomly generated double inputs...]\n";
	uniform_random_test_double(number_of_uniform_random_tests_double);
	std::cout << "Done.\n\n\n";
#endif

#ifdef LIVE_TEST_FLOAT
	std::cout << "[Start live test for float's]\n";
	live_test_float();
#endif

#ifdef LIVE_TEST_DOUBLE
	std::cout << "[Start live test for double's]\n";
	live_test_double();
#endif
}