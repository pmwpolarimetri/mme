#include "mme/lumenera/lumeneracamera.h"
#include "mme/imaging/image.h"
#include <iostream>
#include <filesystem>
#include "npy.hpp"

int main()
{
	try{
		mme::LumeneraCamera cam{};
		cam.set_exposure(mme::Exposure{5});
		cam.set_image_size(mme::ImageSize{ .height=2048, .width=2048});
		cam.set_binning(mme::Binning{ 4 });
		
		auto image = cam.capture_single();
		for (size_t i = 0; i < 10; i++)
		{
			image = cam.capture_single();
			std::cout << "Captured" << std::endl;
			std::cout << "Height: " << image.size().height << ", Width: " << image.size().width << std::endl;
		}
		
		//mme::Image<float> image{ 0.0, mme::ImageSize{256, 256} };
		auto filename = std::filesystem::path("C:/Users/PolarimetriD4-112/dev/mme/data/image.npy");
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