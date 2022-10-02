#include "mme/lumenera/lumeneracamera.h"
#include <iostream>

int main() {

	try
	{
		mme::LumeneraCamera cam{};
		auto cam2 = std::move(cam);
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}

	//{
	//	mme::LumeneraCamera cam{};
	//	
	//}
	

	

	return 0;
}