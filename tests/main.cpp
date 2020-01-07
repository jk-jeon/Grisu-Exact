#include <cstddef>

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

// Generate random float's and test Grisu-Exact's output
//#define UNIFORM_RANDOM_TEST_FLOAT
static std::size_t number_of_uniform_random_tests_float = 10000000;
extern void uniform_random_test_float(std::size_t number_of_tests);

// Generate random double's and test Grisu-Exact's output
//#define UNIFORM_RANDOM_TEST_DOUBLE
static std::size_t number_of_uniform_random_tests_double = 10000000;
extern void uniform_random_test_double(std::size_t number_of_tests);

// Run Grisu-Exact algorithm with randomly generated inputs
//#define UNIFORM_RANDOM_PERF_TEST_FLOAT
static std::size_t number_of_uniform_random_perf_tests_float = 100000000;
extern void uniform_random_perf_test_float(std::size_t number_of_tests);

// Run Grisu-Exact algorithm with randomly generated inputs
//#define UNIFORM_RANDOM_PERF_TEST_DOUBLE
static std::size_t number_of_uniform_random_perf_tests_double = 100000000;
extern void uniform_random_perf_test_double(std::size_t number_of_tests);

// Input float's and test Grisu-Exact's output
//#define LIVE_TEST_FLOAT
extern void live_test_float();

// Input double's and test Grisu-Exact's output
//#define LIVE_TEST_DOUBLE
extern void live_test_double();

// Instant test
//#define MISC_TEST
extern void misc_test();

// Do benchmark for binary32
#define BENCHMARK_TEST_FLOAT
static std::size_t number_of_uniform_benchmark_samples_float = 1000000;
static std::size_t number_of_digits_benchmark_samples_per_digits_float = 100000;
static std::size_t number_of_benchmark_iterations_float = 1000;
extern void benchmark_test_float(std::size_t number_of_uniform_samples,
	std::size_t number_of_digits_samples_per_digits, std::size_t number_of_iterations);

// Do benchmark for binary64
#define BENCHMARK_TEST_DOUBLE
static std::size_t number_of_uniform_benchmark_samples_double = 1000000;
static std::size_t number_of_digits_benchmark_samples_per_digits_double = 100000;
static std::size_t number_of_benchmark_iterations_double = 1000;
extern void benchmark_test_double(std::size_t number_of_uniform_samples,
	std::size_t number_of_digits_samples_per_digits, std::size_t number_of_iterations);

int main()
{
#ifdef VERIFY_LOG_COMPUTATION
	verify_log_computation();
#endif

#ifdef VERIFY_CACHE_LENGTH
	verify_cache_length();
#endif

#ifdef GENERATE_CACHE
	generate_cache();
#endif

#ifdef VERIFY_DELTA_COMPUTATION
	verify_delta_computation();
#endif

#ifdef UNIFORM_RANDOM_TEST_FLOAT
	uniform_random_test_float(number_of_uniform_random_tests_float);
#endif

#ifdef UNIFORM_RANDOM_TEST_DOUBLE
	uniform_random_test_double(number_of_uniform_random_tests_double);
#endif

#ifdef UNIFORM_RANDOM_PERF_TEST_FLOAT
	uniform_random_perf_test_float(number_of_uniform_random_perf_tests_float);
#endif

#ifdef UNIFORM_RANDOM_PERF_TEST_DOUBLE
	uniform_random_perf_test_double(number_of_uniform_random_perf_tests_double);
#endif

#ifdef MISC_TEST
	misc_test();
#endif

#ifdef BENCHMARK_TEST_FLOAT
	benchmark_test_float(number_of_uniform_benchmark_samples_float,
		number_of_digits_benchmark_samples_per_digits_float,
		number_of_benchmark_iterations_float);
#endif

#ifdef BENCHMARK_TEST_DOUBLE
	benchmark_test_double(number_of_uniform_benchmark_samples_double,
		number_of_digits_benchmark_samples_per_digits_double,
		number_of_benchmark_iterations_double);
#endif

#ifdef LIVE_TEST_FLOAT
	live_test_float();
#endif

#ifdef LIVE_TEST_DOUBLE
	live_test_double();
#endif
}