#ifndef __JKJ_GRISU_EXACT_NONFINITE_HANDLERS__
#define __JKJ_GRISU_EXACT_NONFINITE_HANDLERS__

#include "grisu_exact.h"
#include <bitset>
#include <variant>

namespace jkj {
	namespace grisu_exact_nonfinite_handlers {
		// This policy is mainly for debugging purpose
		struct ignore_and_continue {
			template <class Float>
			signed_fp_t<Float> operator()(grisu_exact_impl<Float>& impl, bool) const {
				return impl();
			}
		};

		// A helper that might be useful for further formatting
		template <class Float>
		class nonfinite_float {
		public:
			static constexpr std::size_t payload_length =
				grisu_exact_detail::float_type_info<Float>::precision - 1;

			using float_type_info = grisu_exact_detail::float_type_info<Float>;
			using extended_significand_type =
				typename float_type_info::extended_significand_type;

			nonfinite_float(extended_significand_type bit_representation) {
				assert((bit_representation & float_type_info::exponent_bits_mask) ==
					float_type_info::exponent_bits_mask);

				is_negative_ = (bit_representation & float_type_info::sign_bit_mask) != 0;
				payload_ = bit_representation &
					((extended_significand_type(1) << (float_type_info::precision - 1)) - 1);

				auto quiet_or_signal_indicator =
					extended_significand_type(1) << (float_type_info::precision - 1);
				auto quiet_or_signal = bit_representation & quiet_or_signal_indicator;

				if (payload_ == 0 && quiet_or_signal == 0)
					is_quiet_nan_ = false;
				else {
					auto a_quiet_nan = std::numeric_limits<Float>::quiet_NaN();
					extended_significand_type a_quiet_nan_bit_representation;
					std::memcpy(&a_quiet_nan_bit_representation, &a_quiet_nan, sizeof(Float));

					is_quiet_nan_ = (a_quiet_nan_bit_representation & quiet_or_signal_indicator)
						== quiet_or_signal;
				}
			}

			bool is_negative() const noexcept {
				return is_negative_;
			}
			bool is_infinity() const noexcept {
				return !is_quiet_nan_ && payload_ == 0;
			}
			bool is_nan() const noexcept {
				return !is_infinity();
			}
			bool is_plus_infinity() const noexcept {
				return is_infinity() && !is_negative();
			}
			bool is_minus_infinity() const noexcept {
				return is_infinity() && is_negative();
			}
			bool is_quiet_nan() const noexcept {
				return is_quiet_nan_;
			}
			bool is_signaling_nan() const noexcept {
				return !is_quiet_nan_ && payload != 0;
			}
			std::bitset<payload_length> const& payload() const noexcept {
				return payload_;
			}

		private:
			std::bitset<payload_length> payload_;
			bool	is_negative_;
			bool	is_quiet_nan_;	// true for quiet NaN's, false for +/- infinities and signaling NaN's
		};

		struct return_result_or_nonfinite {
			template <class Float>
			std::variant<signed_fp_t<Float>, nonfinite_float<Float>> operator()(
				grisu_exact_impl<Float>& impl, bool is_finite)
			{
				if (is_finite)
					return impl();
				else
					return nonfinite_float<Float>{ impl.bit_representation };
			}
		};
	}
}

#endif