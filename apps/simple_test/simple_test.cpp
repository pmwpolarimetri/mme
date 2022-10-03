#include <iostream>
#include "asio.hpp"
#include <span>
#include <memory>
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"
#include <format>

#include <Windows.h>
#include "lucamapi.h"
//#include "XCamera.h"

int main() {

	try {
		mme::ESPDriver driver{ "COM1" };
		auto reply = driver.request("VE?\r\n");
	}
	catch( const asio::system_error& e) {
		std::cout << e.what() << std::endl;
		return 0;
	}
	
	return 0;
}