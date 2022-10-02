#include <iostream>
#include "asio.hpp"
#include <span>
#include <memory>
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"

#include <Windows.h>
#include "lucamapi.h"
//#include "XCamera.h"

int main() {

	std::cout << "simple test" << std::endl;

	auto num_cameras = LucamNumCameras();
	std::cout << std::format("num cameras: {}\n", num_cameras);
	auto handle = LucamCameraOpen(1);
	if (handle) {
		std::cout << "Opened" << std::endl;
		LucamCameraClose(handle);
	}

	//auto cam = std::unique_ptr<XCamera>(XCamera::Create());
	//if (cam) {
	//	std::cout << "Made xeneth cam" << std::endl;
	//}

	//try {
	//	mme::ESPDriver driver{ "COM1" };
	//	auto reply = driver.request("VE?\r\n");
	//}
	//catch( const asio::system_error& e) {
	//	std::cout << e.what() << std::endl;
	//	return 0;
	//}
	
	return 0;
}