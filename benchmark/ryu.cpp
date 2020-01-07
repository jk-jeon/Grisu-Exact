#include "benchmark.h"
#include "ryu/ryu.h"

namespace {
	register_function_for_benchmark dummy("Ryu",
		f2s_buffered, d2s_buffered);
}