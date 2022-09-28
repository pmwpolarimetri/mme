#include <iostream>
#include "asio.hpp"
#include <span>
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"
#include <functional>


int main() {

	std::cout << "simple test" << std::endl;

	auto im = mme::Image{ 1.0, {8, 8} };

	mme::ESPDriver driver{ "COM1" };

	driver.move_relative(0, 10.0);
	driver.move_absolute({ 0,1 }, { 10, 20 });

	std::function<void(double)> axis1_func = std::bind_front(&mme::ESPDriver::move_relative, &driver, 1);

	std::function<void(double)> axis2_func = [&driver](double pos) {
		return driver.move_relative(2, pos);
	};

	

	return 0;
}