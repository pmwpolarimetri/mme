#include <iostream>
#include "mme/motion/espdriver.h"
#include <format>


int main() {

	try {
		mme::ESPDriver driver{ "COM1" };
		//driver.move_relative(1, 25.0);
		//driver.home(1);
		auto reply = driver.request("VE?\r\n");
		std::cout << reply << std::endl;
		auto reply2 = driver.request("VE?\r\n");
		std::cout << reply2 << std::endl;
		return 1;
	}
	catch (const asio::system_error& e) {
		std::cout << e.what() << std::endl;
		return 0;
	}

	return 0;
}