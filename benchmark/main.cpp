// Small modification to https://github.com/ulfjack/ryu/blob/master/ryu/benchmark/benchmark.cc



// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

#include <math.h>
#include <inttypes.h>
#include <iostream>
#include <string.h>
#include <chrono>
#include <random>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__linux__)
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ryu/ryu.h"
#include "../grisu_exact.h"

using namespace std::chrono;

constexpr int BUFFER_SIZE = 40;

struct grisu_exact_handler {
	alignas(std::uint32_t) static constexpr char radix_100_table[] = {
		'0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
		'0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
		'1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
		'1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
		'2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
		'2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
		'3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
		'3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
		'4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
		'4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
		'5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
		'5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
		'6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
		'6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
		'7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
		'7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
		'8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
		'8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
		'9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
		'9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
	};

	template <class UInt>
	static constexpr std::uint32_t decimal_length(UInt const v) {
		if constexpr (std::is_same_v<UInt, std::uint32_t>) {
			// Function precondition: v is not a 10-digit number.
			// (f2s: 9 digits are sufficient for round-tripping.)
			// (d2fixed: We print 9-digit blocks.)
			assert(v < 1000000000);
			if (v >= 100000000) { return 9; }
			if (v >= 10000000) { return 8; }
			if (v >= 1000000) { return 7; }
			if (v >= 100000) { return 6; }
			if (v >= 10000) { return 5; }
			if (v >= 1000) { return 4; }
			if (v >= 100) { return 3; }
			if (v >= 10) { return 2; }
			return 1;
		}
		else {
			static_assert(std::is_same_v<UInt, std::uint64_t>);
			// This is slightly faster than a loop.
			// The average output length is 16.38 digits, so we check high-to-low.
			// Function precondition: v is not an 18, 19, or 20-digit number.
			// (17 digits are sufficient for round-tripping.)
			assert(v < 100000000000000000L);
			if (v >= 10000000000000000L) { return 17; }
			if (v >= 1000000000000000L) { return 16; }
			if (v >= 100000000000000L) { return 15; }
			if (v >= 10000000000000L) { return 14; }
			if (v >= 1000000000000L) { return 13; }
			if (v >= 100000000000L) { return 12; }
			if (v >= 10000000000L) { return 11; }
			if (v >= 1000000000L) { return 10; }
			if (v >= 100000000L) { return 9; }
			if (v >= 10000000L) { return 8; }
			if (v >= 1000000L) { return 7; }
			if (v >= 100000L) { return 6; }
			if (v >= 10000L) { return 5; }
			if (v >= 1000L) { return 4; }
			if (v >= 100L) { return 3; }
			if (v >= 10L) { return 2; }
			return 1;
		}
	}

	char* buffer;
	template <class Float>
	void operator()(jkj::grisu_exact_impl<Float>& impl) const
	{
		// Step 5: Print the decimal representation.
		int index = 0;

		if (!impl.is_finite() || !impl.is_nonzero()) {
			if (impl.extract_significand() != 0)
			{
				std::memcpy(buffer, "NaN", 3);
				index += 3;
			}
			else {
				if (impl.is_negative()) {
					buffer[index++] = '-';
				}

				if (impl.is_nonzero()) {
					std::memcpy(buffer + index, "Infinity", 8);
					index += 8;
				}
				else {
					std::memcpy(buffer + index, "0E0", 3);
					index += 3;
				}
			}
		}
		else {
			auto v = impl();

			if (v.is_negative) {
				buffer[index++] = '-';
			}
			auto output = v.significand;
			auto const olength = decimal_length(output);

			if constexpr (sizeof(Float) == 4) {
				// Print the decimal digits.
				// The following code is equivalent to:
				// for (uint32_t i = 0; i < olength - 1; ++i) {
				//   const uint32_t c = output % 10; output /= 10;
				//   result[index + olength - i] = (char) ('0' + c);
				// }
				// result[index] = '0' + output % 10;
				uint32_t i = 0;
				while (output >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
					const uint32_t c = output - 10000 * (output / 10000);
#else
					const uint32_t c = output % 10000;
#endif
					output /= 10000;
					const uint32_t c0 = (c % 100) << 1;
					const uint32_t c1 = (c / 100) << 1;
					memcpy(buffer + index + olength - i - 1, radix_100_table + c0, 2);
					memcpy(buffer + index + olength - i - 3, radix_100_table + c1, 2);
					i += 4;
				}
				if (output >= 100) {
					const uint32_t c = (output % 100) << 1;
					output /= 100;
					memcpy(buffer + index + olength - i - 1, radix_100_table + c, 2);
					i += 2;
				}
				if (output >= 10) {
					const uint32_t c = output << 1;
					// We can't use memcpy here: the decimal dot goes between these two digits.
					buffer[index + olength - i] = radix_100_table[c + 1];
					buffer[index] = radix_100_table[c];
				}
				else {
					buffer[index] = (char)('0' + output);
				}

				// Print decimal point if needed.
				if (olength > 1) {
					buffer[index + 1] = '.';
					index += olength + 1;
				}
				else {
					++index;
				}

				// Print the exponent.
				buffer[index++] = 'E';
				int32_t exp = v.exponent + (int32_t)olength - 1;
				if (exp < 0) {
					buffer[index++] = '-';
					exp = -exp;
				}

				if (exp >= 10) {
					memcpy(buffer + index, radix_100_table + 2 * exp, 2);
					index += 2;
				}
				else {
					buffer[index++] = (char)('0' + exp);
				}
			}
			else {
				// Print the decimal digits.
				// The following code is equivalent to:
				// for (uint32_t i = 0; i < olength - 1; ++i) {
				//   const uint32_t c = output % 10; output /= 10;
				//   result[index + olength - i] = (char) ('0' + c);
				// }
				// result[index] = '0' + output % 10;

				uint32_t i = 0;
				// We prefer 32-bit operations, even on 64-bit platforms.
				// We have at most 17 digits, and uint32_t can store 9 digits.
				// If output doesn't fit into uint32_t, we cut off 8 digits,
				// so the rest will fit into uint32_t.
				if ((output >> 32) != 0) {
					// Expensive 64-bit division.
					const uint64_t q = output / 100000000;
					uint32_t output2 = ((uint32_t)output) - 100000000 * ((uint32_t)q);
					output = q;

					const uint32_t c = output2 % 10000;
					output2 /= 10000;
					const uint32_t d = output2 % 10000;
					const uint32_t c0 = (c % 100) << 1;
					const uint32_t c1 = (c / 100) << 1;
					const uint32_t d0 = (d % 100) << 1;
					const uint32_t d1 = (d / 100) << 1;
					memcpy(buffer + index + olength - i - 1, radix_100_table + c0, 2);
					memcpy(buffer + index + olength - i - 3, radix_100_table + c1, 2);
					memcpy(buffer + index + olength - i - 5, radix_100_table + d0, 2);
					memcpy(buffer + index + olength - i - 7, radix_100_table + d1, 2);
					i += 8;
				}
				uint32_t output2 = (uint32_t)output;
				while (output2 >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
					const uint32_t c = output2 - 10000 * (output2 / 10000);
#else
					const uint32_t c = output2 % 10000;
#endif
					output2 /= 10000;
					const uint32_t c0 = (c % 100) << 1;
					const uint32_t c1 = (c / 100) << 1;
					memcpy(buffer + index + olength - i - 1, radix_100_table + c0, 2);
					memcpy(buffer + index + olength - i - 3, radix_100_table + c1, 2);
					i += 4;
				}
				if (output2 >= 100) {
					const uint32_t c = (output2 % 100) << 1;
					output2 /= 100;
					memcpy(buffer + index + olength - i - 1, radix_100_table + c, 2);
					i += 2;
				}
				if (output2 >= 10) {
					const uint32_t c = output2 << 1;
					// We can't use memcpy here: the decimal dot goes between these two digits.
					buffer[index + olength - i] = radix_100_table[c + 1];
					buffer[index] = radix_100_table[c];
				}
				else {
					buffer[index] = (char)('0' + output2);
				}

				// Print decimal point if needed.
				if (olength > 1) {
					buffer[index + 1] = '.';
					index += olength + 1;
				}
				else {
					++index;
				}

				// Print the exponent.
				buffer[index++] = 'E';
				int32_t exp = v.exponent + (int32_t)olength - 1;
				if (exp < 0) {
					buffer[index++] = '-';
					exp = -exp;
				}

				if (exp >= 100) {
					const int32_t c = exp % 10;
					memcpy(buffer + index, radix_100_table + 2 * (exp / 10), 2);
					buffer[index + 2] = (char)('0' + c);
					index += 3;
				}
				else if (exp >= 10) {
					memcpy(buffer + index, radix_100_table + 2 * exp, 2);
					index += 2;
				}
				else {
					buffer[index++] = (char)('0' + exp);
				}
			}
		}

		buffer[index] = '\0';
	}
};


void dcv(double x, char* buffer)
{
	jkj::grisu_exact(x, grisu_exact_handler{ buffer },
		jkj::grisu_exact_rounding_modes::to_even{});
}

void fcv(float x, char* buffer)
{
	jkj::grisu_exact(x, grisu_exact_handler{ buffer },
		jkj::grisu_exact_rounding_modes::to_even{});
}

static float int32Bits2Float(uint32_t bits) {
	float f;
	memcpy(&f, &bits, sizeof(float));
	return f;
}

static double int64Bits2Double(uint64_t bits) {
	double f;
	memcpy(&f, &bits, sizeof(double));
	return f;
}

struct mean_and_variance {
	int64_t n = 0;
	double mean = 0;
	double m2 = 0;

	void update(double x) {
		++n;
		double d = x - mean;
		mean += d / n;
		double d2 = x - mean;
		m2 += d * d2;
	}

	double variance() const {
		return m2 / (n - 1);
	}

	double stddev() const {
		return sqrt(variance());
	}
};

class benchmark_options {
public:
	benchmark_options() = default;
	benchmark_options(const benchmark_options&) = delete;
	benchmark_options& operator=(const benchmark_options&) = delete;

	bool run32() const { return m_run32; }
	bool run64() const { return m_run64; }
	int samples() const { return m_samples; }
	int iterations() const { return m_iterations; }
	bool verbose() const { return m_verbose; }
	bool ryu_only() const { return m_ryu_only; }
	bool classic() const { return m_classic; }
	int small_digits() const { return m_small_digits; }

	void parse(const char* const arg) {
		if (strcmp(arg, "-32") == 0) {
			m_run32 = true;
			m_run64 = false;
		}
		else if (strcmp(arg, "-64") == 0) {
			m_run32 = false;
			m_run64 = true;
		}
		else if (strcmp(arg, "-v") == 0) {
			m_verbose = true;
		}
		else if (strcmp(arg, "-ryu") == 0) {
			m_ryu_only = true;
		}
		else if (strcmp(arg, "-classic") == 0) {
			m_classic = true;
		}
		else if (strncmp(arg, "-samples=", 9) == 0) {
			if (sscanf(arg, "-samples=%i", &m_samples) != 1 || m_samples < 1) {
				fail(arg);
			}
		}
		else if (strncmp(arg, "-iterations=", 12) == 0) {
			if (sscanf(arg, "-iterations=%i", &m_iterations) != 1 || m_iterations < 1) {
				fail(arg);
			}
		}
		else if (strncmp(arg, "-small_digits=", 14) == 0) {
			if (sscanf(arg, "-small_digits=%i", &m_small_digits) != 1 || m_small_digits < 1 || m_small_digits > 7) {
				fail(arg);
			}
		}
		else {
			fail(arg);
		}
	}

private:
	void fail(const char* const arg) {
		printf("Unrecognized option '%s'.\n", arg);
		exit(EXIT_FAILURE);
	}

	// By default, run both 32 and 64-bit benchmarks with 10000 samples and 1000 iterations each.
	bool m_run32 = true;
	bool m_run64 = true;
	int m_samples = 10000;
	int m_iterations = 1000;
	bool m_verbose = false;
	bool m_ryu_only = false;
	bool m_classic = false;
	int m_small_digits = 0;
};

// returns 10^x
uint32_t exp10(const int x) {
	uint32_t ret = 1;

	for (int i = 0; i < x; ++i) {
		ret *= 10;
	}

	return ret;
}

float generate_float(const benchmark_options& options, std::mt19937& mt32, uint32_t& r) {
	r = mt32();

	if (options.small_digits() == 0) {
		float f = int32Bits2Float(r);
		return f;
	}

	// Example:
	// options.small_digits() is 3
	// lower is 100
	// upper is 1000
	// r % (1000 - 100) + 100;
	// r % 900 + 100;
	// r is [0, 899] + 100
	// r is [100, 999]
	// r / 100 is [1.00, 9.99]
	const uint32_t lower = exp10(options.small_digits() - 1);
	const uint32_t upper = lower * 10;
	r = r % (upper - lower) + lower; // slightly biased, but reproducible
	return r / static_cast<float>(lower);
}

static int bench32(const benchmark_options& options) {
	char bufferown[BUFFER_SIZE];
	char bufferown2[BUFFER_SIZE];
	std::mt19937 mt32(std::random_device{}());
	mean_and_variance mv1;
	mean_and_variance mv2;
	int throwaway = 0;
	if (options.classic()) {
		for (int i = 0; i < options.samples(); ++i) {
			uint32_t r = 0;
			const float f = generate_float(options, mt32, r);

			auto t1 = steady_clock::now();
			for (int j = 0; j < options.iterations(); ++j) {
				f2s_buffered(f, bufferown);
				throwaway += bufferown[2];
			}
			auto t2 = steady_clock::now();
			double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
			mv1.update(delta1);

			double delta2 = 0.0;
			if (!options.ryu_only()) {
				t1 = steady_clock::now();
				for (int j = 0; j < options.iterations(); ++j) {
					fcv(f, bufferown2);
					throwaway += bufferown2[2];
				}
				t2 = steady_clock::now();
				delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
				mv2.update(delta2);
			}

			if (options.verbose()) {
				if (options.ryu_only()) {
					printf("%s,%u,%f\n", bufferown, r, delta1);
				}
				else {
					printf("%s,%u,%f,%f\n", bufferown, r, delta1, delta2);
				}
			}

			if (!options.ryu_only() && strcmp(bufferown, bufferown2) != 0) {
				printf("For %x %20s %20s\n", r, bufferown, bufferown2);
			}
		}
	}
	else {
		std::vector<float> vec(options.samples());
		for (int i = 0; i < options.samples(); ++i) {
			uint32_t r = 0;
			vec[i] = generate_float(options, mt32, r);
		}

		for (int j = 0; j < options.iterations(); ++j) {
			auto t1 = steady_clock::now();
			for (int i = 0; i < options.samples(); ++i) {
				f2s_buffered(vec[i], bufferown);
				throwaway += bufferown[2];
			}
			auto t2 = steady_clock::now();
			double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
			mv1.update(delta1);

			double delta2 = 0.0;
			if (!options.ryu_only()) {
				t1 = steady_clock::now();
				for (int i = 0; i < options.samples(); ++i) {
					fcv(vec[i], bufferown2);
					throwaway += bufferown2[2];
				}
				t2 = steady_clock::now();
				delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
				mv2.update(delta2);
			}

			if (options.verbose()) {
				if (options.ryu_only()) {
					printf("%f\n", delta1);
				}
				else {
					printf("%f,%f\n", delta1, delta2);
				}
			}
		}
	}
	if (!options.verbose()) {
		printf("32: %8.3f %8.3f", mv1.mean, mv1.stddev());
		if (!options.ryu_only()) {
			printf("     %8.3f %8.3f", mv2.mean, mv2.stddev());
		}
		printf("\n");
	}
	return throwaway;
}

double generate_double(const benchmark_options& options, std::mt19937& mt32, uint64_t& r) {
	r = mt32();
	r <<= 32;
	r |= mt32(); // calling mt32() in separate statements guarantees order of evaluation

	if (options.small_digits() == 0) {
		double f = int64Bits2Double(r);
		return f;
	}

	// see example in generate_float()
	const uint32_t lower = exp10(options.small_digits() - 1);
	const uint32_t upper = lower * 10;
	r = r % (upper - lower) + lower; // slightly biased, but reproducible
	return r / static_cast<double>(lower);
}

static int bench64(const benchmark_options& options) {
	char bufferown[BUFFER_SIZE];
	char bufferown2[BUFFER_SIZE];
	std::mt19937 mt32(std::random_device{}());
	mean_and_variance mv1;
	mean_and_variance mv2;
	int throwaway = 0;
	if (options.classic()) {
		for (int i = 0; i < options.samples(); ++i) {
			uint64_t r = 0;
			const double f = generate_double(options, mt32, r);

			auto t1 = steady_clock::now();
			for (int j = 0; j < options.iterations(); ++j) {
				d2s_buffered(f, bufferown);
				//dcv(f, bufferown);
				throwaway += bufferown[2];
			}
			auto t2 = steady_clock::now();
			double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
			mv1.update(delta1);

			double delta2 = 0.0;
			if (!options.ryu_only()) {
				t1 = steady_clock::now();
				for (int j = 0; j < options.iterations(); ++j) {
					dcv(f, bufferown2);
					//d2s_buffered(f, bufferown2);
					throwaway += bufferown2[2];
				}
				t2 = steady_clock::now();
				delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.iterations());
				mv2.update(delta2);
			}

			if (options.verbose()) {
				if (options.ryu_only()) {
					printf("%s,%" PRIu64 ",%f\n", bufferown, r, delta1);
				}
				else {
					printf("%s,%" PRIu64 ",%f,%f\n", bufferown, r, delta1, delta2);
				}
			}

			if (!options.ryu_only() && strcmp(bufferown, bufferown2) != 0) {
				printf("For %16" PRIX64 " %28s %28s\n", r, bufferown, bufferown2);
			}
		}
	}
	else {
		std::vector<double> vec(options.samples());
		for (int i = 0; i < options.samples(); ++i) {
			uint64_t r = 0;
			vec[i] = generate_double(options, mt32, r);
		}

		for (int j = 0; j < options.iterations(); ++j) {
			auto t1 = steady_clock::now();
			for (int i = 0; i < options.samples(); ++i) {
				d2s_buffered(vec[i], bufferown);
				throwaway += bufferown[2];
			}
			auto t2 = steady_clock::now();
			double delta1 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
			mv1.update(delta1);

			double delta2 = 0.0;
			if (!options.ryu_only()) {
				t1 = steady_clock::now();
				for (int i = 0; i < options.samples(); ++i) {
					dcv(vec[i], bufferown2);
					throwaway += bufferown2[2];
				}
				t2 = steady_clock::now();
				delta2 = duration_cast<nanoseconds>(t2 - t1).count() / static_cast<double>(options.samples());
				mv2.update(delta2);
			}

			if (options.verbose()) {
				if (options.ryu_only()) {
					printf("%f\n", delta1);
				}
				else {
					printf("%f,%f\n", delta1, delta2);
				}
			}
		}
	}
	if (!options.verbose()) {
		printf("64: %8.3f %8.3f", mv1.mean, mv1.stddev());
		if (!options.ryu_only()) {
			printf("     %8.3f %8.3f", mv2.mean, mv2.stddev());
		}
		printf("\n");
	}
	return throwaway;
}

int main(int argc, char** argv) {
#if defined(__linux__)
	// Also disable hyperthreading with something like this:
	// cat /sys/devices/system/cpu/cpu*/topology/core_id
	// sudo /bin/bash -c "echo 0 > /sys/devices/system/cpu/cpu6/online"
	cpu_set_t my_set;
	CPU_ZERO(&my_set);
	CPU_SET(2, &my_set);
	sched_setaffinity(getpid(), sizeof(cpu_set_t), &my_set);
#endif

	benchmark_options options;

	for (int i = 1; i < argc; ++i) {
		options.parse(argv[i]);
	}

	if (!options.verbose()) {
		// No need to buffer the output if we're just going to print three lines.
		setbuf(stdout, NULL);
	}

	if (options.verbose()) {
		printf("%sryu_time_in_ns%s\n", options.classic() ? "ryu_output,float_bits_as_int," : "", options.ryu_only() ? "" : ",grisu_exact_time_in_ns");
	}
	else {
		printf("    Average & Stddev Ryu%s\n", options.ryu_only() ? "" : "  Average & Stddev Grisu-Exact");
	}
	int throwaway = 0;
	if (options.run32()) {
		throwaway += bench32(options);
	}
	if (options.run64()) {
		throwaway += bench64(options);
	}
	if (argc == 1000) {
		// Prevent the compiler from optimizing the code away.
		printf("%d\n", throwaway);
	}
	return 0;
}
