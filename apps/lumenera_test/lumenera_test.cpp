#include "mme/lumenera/lumeneracamera.h"
#include <iostream>

int main() {

	try
	{
		mme::LumeneraCamera cam{};
		for (size_t i = 0; i < 100; i++) {
			auto image = cam.capture_single();
			std::cout << image.as_view().at(1024, 1024) << std::endl;
		}
		

	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}


	return 0;
}