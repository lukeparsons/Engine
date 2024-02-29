#include "kernel.hpp"

string get_fluid_code()
{
	string r1 = actions_code();
	string r2 = main_code();
	replace(r1);
	replace(r2);
	return r1 + r2;
}

string get_opencl_c_code()
{
	string r = opencl_c_container();
	replace(r);
	return r;
}