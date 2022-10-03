#include "mme/lumenera/lumeneracamera.h"
#include "mme/imaging/image.h"
#include <iostream>
#include <filesystem>
#include "npy.hpp"

int main()
{
	try{
		mme::LumeneraCamera cam{};
		auto image = cam.capture_single();
		//mme::Image<float> image{ 0.0, mme::ImageSize{256, 256} };
		auto filename = std::filesystem::path("C:/Users/permw/phd/dev/mme/data/image.npy");
		mme::save_to_numpy(filename.string(), image);
		return 1;
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}
	return 0;
}