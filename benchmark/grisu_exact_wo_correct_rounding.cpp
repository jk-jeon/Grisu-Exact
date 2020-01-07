#include "benchmark.h"
#include "../fp_to_chars.h"

namespace {
	void grisu_exact_wo_correct_rounding_float_to_chars(float x, char* buffer)
	{
		jkj::fp_to_chars(x, buffer,
			jkj::grisu_exact_rounding_modes::nearest_to_even{},
			jkj::grisu_exact_correct_rounding::do_not_care{});
	}
	void grisu_exact_wo_correct_rounding_double_to_chars(double x, char* buffer)
	{
		jkj::fp_to_chars(x, buffer,
			jkj::grisu_exact_rounding_modes::nearest_to_even{},
			jkj::grisu_exact_correct_rounding::do_not_care{});
	}
	register_function_for_benchmark dummy("Grisu-Exact (w/o correct rounding)",
		grisu_exact_wo_correct_rounding_float_to_chars,
		grisu_exact_wo_correct_rounding_double_to_chars);
}