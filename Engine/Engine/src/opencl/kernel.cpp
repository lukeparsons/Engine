#include "kernel.hpp"

string get_opencl_c_code()
{
	string r = opencl_c_container();
	replace(r);
	return r;
}