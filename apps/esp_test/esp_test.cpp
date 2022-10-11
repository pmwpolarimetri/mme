#include <iostream>
#include "mme/motion/espdriver.h"
#include <format>


int main() {

	try {
		mme::ESPDriver driver{ "COM1" };
		driver.move_relative(1, 25.0);
		driver.home(1);

		auto reply = driver.request("VE?\r\n");
		std::cout << reply << std::endl;
		auto reply2 = driver.request("VE?\r\n");
		std::cout << reply2 << std::endl;

		//1: PSA
		//2: Transmission PSG
		//3: Rotation PSG
		
		driver.move_absolute(1, 25.0);
		driver.move_absolute(2, 25.0);
		driver.move_absolute(3, 25.0);
		//driver.move_relative(1,25.0);
		//driver.home(2);

		return 1;
	}
	catch (const asio::system_error& e) {
		std::cout << e.what() << std::endl;
		return 0;
	}

	return 0;
}