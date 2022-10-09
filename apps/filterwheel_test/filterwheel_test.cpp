#include "mme/fwxc/fwxc.h"
#include <iostream>

int main() {

	try
	{
		mme::Fwxc filter_wheel{};
		std::cout << "num filters installed: " << filter_wheel.num_filters() << std::endl;
		std::cout << "current: " << filter_wheel.current_filter_position() << std::endl;
		auto ok = filter_wheel.change_filter_position(2);
		std::cout << "change ok: " << ok << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}