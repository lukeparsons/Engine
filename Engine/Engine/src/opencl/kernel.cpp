#include "kernel.hpp"

string get_opencl_c_code_old()
{
	string r = old_opencl_c_container();
	replace(r);
	return r;
}

string get_opencl_c_code()
{
	string r = opencl_c_container();
	replace(r);
	return r;
}