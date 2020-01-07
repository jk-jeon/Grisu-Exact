#include "benchmark.h"
#include "floaxie/ftoa.h"

namespace {
	void floaxie_ftoa_float(float x, char* buffer)
	{
		floaxie::ftoa(x, buffer);
	}
	void floaxie_ftoa_double(double x, char* buffer)
	{
		floaxie::ftoa(x, buffer);
	}
	// Too slow; just clutters the plot
	/*register_function_for_benchmark dummy("Floaxie ftoa",
		floaxie_ftoa_float,
		floaxie_ftoa_double);*/
}