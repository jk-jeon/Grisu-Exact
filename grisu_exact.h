#ifndef __JKJ_GRISU_EXACT__
#define __JKJ_GRISU_EXACT__

#include <cstddef>		// std::size_t, etc
#include <cstdint>		// std::uint32_t, etc.
#include <limits>

#if defined(_MSC_VER)
#include <intrin.h>
#include <immintrin.h>
#endif

namespace jkj {
	namespace grisu_exact_detail {
		////////////////////////////////////////////////////////////////////////////////////////
		// Utilities for 128-bit arithmetic
		////////////////////////////////////////////////////////////////////////////////////////

		// Get 128-bit result of multiplication of two 64-bit unsigned integers
		struct uint128 {
			uint128() = default;

#if defined(__GNUC__) || defined(__clang__) && defined(__SIZEOF_INT128__) && defined(__x86_64__)
			unsigned __int128	internal_;

			constexpr uint128(std::uint64_t low, std::uint64_t high) noexcept :
				internal_{ ((unsigned __int128)low) | (((unsigned __int128)high) << 64) } {}

			constexpr uint128(unsigned __int128 u) : internal_{ u } {}

			constexpr std::uint64_t low() const noexcept {
				return std::uint64_t(internal_);
			}
			constexpr std::uint64_t high() const noexcept {
				return std::uint64_t(internal_ >> 64);
			}
#else
			std::uint64_t	low_;
			std::uint64_t	high_;

			constexpr uint128(std::uint64_t low, std::uint64_t high) noexcept :
				low_{ low }, high_{ high } {}

			constexpr std::uint64_t low() const noexcept {
				return low_;
			}
			constexpr std::uint64_t high() const noexcept {
				return high_;
			}
#endif
		};
		inline uint128 umul128(std::uint64_t x, std::uint64_t y) noexcept {
#if defined(_MSC_VER)
			uint128 result;
			result.low_ = _umul128(x, y, &result.high_);
			return result;
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__) && defined(__x86_64__)
			return (unsigned __int128)(x) * (unsigned __int128)(y);
#else
			constexpr auto mask = (std::uint64_t(1) << 32) - std::uint64_t(1);

			auto a = x >> 32;
			auto b = x & mask;
			auto c = y >> 32;
			auto d = y & mask;

			auto ac = a * c;
			auto bc = b * c;
			auto ad = a * d;
			auto bd = b * d;

			auto intermediate = (bd >> 32) + (ad & mask) + (bc & mask);

			return{ (intermediate << 32) + (bd & mask),
				ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32) };
#endif
		}

		inline std::uint64_t umul128_upper64(std::uint64_t x, std::uint64_t y) noexcept {
#if defined(_MSC_VER)
			return __umulh(x, y);
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__) && defined(__x86_64__)
			auto p = (unsigned __int128)(x) * (unsigned __int128)(y);
			return std::uint64_t(p >> 64);
#else
			constexpr auto mask = (std::uint64_t(1) << 32) - std::uint64_t(1);

			auto a = x >> 32;
			auto b = x & mask;
			auto c = y >> 32;
			auto d = y & mask;

			auto ac = a * c;
			auto bc = b * c;
			auto ad = a * d;
			auto bd = b * d;

			auto intermediate = (bd >> 32) + (ad & mask) + (bc & mask);

			return ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32);
#endif
		}

		// Get upper 128-bits of multiplication of a 64-bit unsigned integer and a 128-bit unsigned integer
		inline uint128 umul192_upper128(std::uint64_t x, uint128 y) noexcept {
			auto g0 = umul128(x, y.high());
			auto g10 = umul128_upper64(x, y.low());

#if defined(_MSC_VER) && defined(_M_AMD64)
			std::uint64_t low, high;
			auto carry = _addcarryx_u64(0, g0.low(), g10, &low);
			_addcarryx_u64(carry, g0.high(), 0, &high);
			return{ low, high };
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__) && defined(__x86_64__)
			return{ g0.internal_ + g10 };
#else
			auto intermediate = g0.low() + g10;
			return{ intermediate, g0.high() + (intermediate < g10) };
#endif
		}

		// Get upper 64-bits of multiplication of a 32-bit unsigned integer and a 64-bit unsigned integer
		inline std::uint64_t umul96_upper64(std::uint32_t x, std::uint64_t y) noexcept {
			auto g0 = std::uint64_t(x) * (y >> 32);
			auto g1 = std::uint64_t(x) * (y & 0xffffffff);
			return g0 + (g1 >> 32);
		}


		////////////////////////////////////////////////////////////////////////////////////////
		// Fast and accurate log floor/ceil calculation
		////////////////////////////////////////////////////////////////////////////////////////

		// The result of this function is accurate for
		// exp in [-65536,+65536], but may not be valid outside.
		constexpr int ceil_log10_pow2(int e) noexcept {
			// The next 32 digits are 0x7de7fbcc
			constexpr std::uint32_t log10_2_up_to_32 = 0x4d104d42;

			return int(
				// Calculate 0x0.4d104d42 * exp * 2^32
				(std::int64_t(e) * log10_2_up_to_32
					// Perform ceiling
					+ ((std::int64_t(1) << 32) - 1))
				// Perform arithmetic-shift
				>> 32);
		}

		// The result of this function is accurate for
		// exp in [-65536,+65536], but may not be valid outside.
		constexpr int floor_log2_pow10(int e) noexcept {
			// The next 32 digits are 0x346e2bf9
			constexpr std::uint32_t log2_5_over_4_up_to_32 = 0x5269e12f;

			return 3 * e + int(
				// Calculate 0x0.5269e12f * exp * 2^32
				(std::int64_t(e) * log2_5_over_4_up_to_32)
				// Perform arithmetic-shift
				>> 32);
		}


		////////////////////////////////////////////////////////////////////////////////////////
		// Collection of relevent information about IEEE-754 and precision settings
		////////////////////////////////////////////////////////////////////////////////////////

		template <class Float>
		struct float_type_info {
			static_assert(std::numeric_limits<Float>::is_iec559 &&
				std::numeric_limits<Float>::radix == 2 &&
				(sizeof(Float) == 4 || sizeof(Float) == 8),
				"Grisu-Exact algorithm only applies to IEEE-754 binary32 and binary64 formats!");

			static constexpr std::size_t precision = std::numeric_limits<Float>::digits - 1;
			
			using extended_significand_type = std::conditional_t<
				sizeof(Float) == 4,
				std::uint32_t,
				std::uint64_t>;

			static constexpr std::size_t extended_precision =
				sizeof(extended_significand_type) * std::numeric_limits<unsigned char>::digits;

			static constexpr int min_exponent =
				std::numeric_limits<Float>::min_exponent - int(extended_precision);
			static constexpr int max_exponent =
				std::numeric_limits<Float>::max_exponent - int(extended_precision);
			static_assert(min_exponent < 0 && max_exponent > 0 && -min_exponent >= max_exponent);
		};
	}
}

#endif