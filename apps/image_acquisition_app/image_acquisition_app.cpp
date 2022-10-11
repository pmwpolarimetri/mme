#include <iostream>
#include <format>
//#include <Windows.h>
#include <span>
#include <memory>
#include <filesystem>
#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <assert.h>

#include "mme/lumenera/lumeneracamera.h"
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"
#include "mme/fwxc/fwxc.h"
#include "lucamapi.h"
#include "npy.hpp"
//#include "asio.hpp"

// Let PSG_driver be 2 for transmission mode and 3 for reflection mode
int PSG_driver = 2;
int PSA_driver = 1;

//Type in the rotations of the retarders (must be of the same size)
std::vector<double> PSG_pos{-51.7,-15.1,15.1,51.7};
std::vector<double> PSA_pos{-51.7,-15.1,15.1,51.7};

std::string make_new_directory() {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char* timenow = std::ctime(&now);
	std::string foldername(timenow);
	foldername.erase(std::remove(foldername.begin(), foldername.end(), '\n'), foldername.cend());
	std::replace(foldername.begin(), foldername.end(), ':', '.');

	std::string path = "C:/Users/PolarimetriD4-112/dev/mme/data/" + foldername;

	std::filesystem::current_path("C:/Users/PolarimetriD4-112/dev/mme/data/");

	bool newdir = std::filesystem::create_directory(foldername);
	assert(newdir);
	std::cout << "Created new folder for saving files: " << foldername << std::endl;

	return path;
}

void camera_initialize(mme::LumeneraCamera *cam) {
	cam->set_exposure(mme::Exposure{ 5 });
	cam->set_image_size(mme::ImageSize{ .height = 2048, .width = 2048 });
	cam->set_binning(mme::Binning{ 4 });
	std::cout << "Initialized camera" << std::endl;
	return;
};

void driver_initialize(mme::ESPDriver* driver) {
	auto reply = driver->request("VE?\r\n");
	std::cout << "Initialized rotation motor: " << reply << std::endl << std::endl;
	driver->home(PSG_driver);
	driver->home(PSA_driver);
}

void measure_and_save(mme::LumeneraCamera* cam, std::string path, float PSG_pos, float PSA_pos) {
	auto image = cam->capture_single();
	std::cout << "Captured image at PSG pos " << PSG_pos << " and PSA pos " << PSA_pos << std::endl;
	std::cout << "Height: " << image.size().height << ", Width: " << image.size().width << std::endl;

	auto filename = std::filesystem::path(path + "/PSG" + std::to_string(PSG_pos) + "PSA" + std::to_string(PSA_pos) + ".npy");
	mme::save_to_numpy(filename.string(), image);
	return;
};

int main()
{

	try {

		std::string path = make_new_directory();

		mme::LumeneraCamera cam{};
		camera_initialize(&cam);

		mme::ESPDriver driver{ "COM1" };
		driver_initialize(&driver);

		for (int i = 0; i != PSG_pos.size(); ++i) {

			driver.move_absolute(PSG_driver, PSG_pos[i]);
			driver.move_absolute(PSA_driver, PSA_pos[i]);

			measure_and_save(&cam, path, PSG_pos[i], PSA_pos[i]);
		}

		driver.home(PSG_driver);
		driver.home(PSA_driver);

		std::cout << "\nSuccessfully acquired images and saved to file" << std::endl;

		// Include mono and filterwheel

		return 1;
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}
	catch (const asio::system_error& e) {
		std::cout << e.what() << std::endl;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}