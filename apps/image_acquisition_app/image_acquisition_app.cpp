#include <iostream>
#include <format>
#include <span>
#include <memory>
#include <filesystem>
#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <assert.h>
//#include <Windows.h>

#include "mme/lumenera/lumeneracamera.h"
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"
#include "mme/fwxc/fwxc.h"
#include "lucamapi.h"
#include "npy.hpp"
#include "asio.hpp"

// Type in a foldername of the measurement:
std::string folderdescription = "Frosted glass";

// K-space or real image?
bool kspace = false;

// Transmission or reflection mode?
bool transmission = true;

//Type in the rotations of the retarders (must be of the same size)
std::vector<double> PSG_pos{-51.7,-15.1,15.1,51.7};
std::vector<double> PSA_pos{-51.7,-15.1,15.1,51.7};


std::string make_new_directory(std::string transorref, std::string kspaceorreal) {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char* timenow = std::ctime(&now);
	std::string foldername(timenow);
	foldername.erase(std::remove(foldername.begin(), foldername.end(), '\n'), foldername.cend());
	std::replace(foldername.begin(), foldername.end(), ':', '.');
	foldername = foldername + " " + folderdescription + ", " + kspaceorreal + ", " + transorref;

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

void driver_initialize(mme::ESPDriver* driver, int PSG_driver, int PSA_driver) {
	auto reply = driver->request("VE?\r\n");
	std::cout << "Initialized rotation motors: " << reply << std::endl << std::endl;
	driver->home(PSG_driver);
	driver->home(PSA_driver);
}

void measure_and_save(mme::LumeneraCamera* cam, std::string path, std::string PSG_pos, std::string PSA_pos, std::string wavelength) {
	auto image = cam->capture_single();
	std::cout << "Captured image at wavelength " << wavelength << std::endl;
	std::cout << "Height: " << image.size().height << ", Width: " << image.size().width << std::endl;

	auto filename = std::filesystem::path(path + "/PSG" + PSG_pos + "PSA" + PSA_pos + "Wl" + wavelength + ".npy");
	mme::save_to_numpy(filename.string(), image);
	return;
};

int main()
{

	try {

		int PSG_driver;
		std::string transorref;
		if (transmission) {
			PSG_driver = 2;
			transorref = "transmission mode";
		}
		else {
			PSG_driver = 3;
			transorref = "reflection mode";
		}
		int PSA_driver = 1;

		std::string kspaceorreal;
		if (kspace) {
			kspaceorreal = "k-space image";
		}
		else {
			kspaceorreal = "real image";
		}

		std::string path = make_new_directory(transorref,kspaceorreal);

		mme::LumeneraCamera cam{};
		camera_initialize(&cam);

		mme::ESPDriver driver{ "COM1" };
		driver_initialize(&driver,PSG_driver,PSA_driver);

		for (int i = 0; i != PSG_pos.size(); ++i) {

			driver.move_absolute(PSG_driver, PSG_pos[i]);
			driver.move_absolute(PSA_driver, PSA_pos[i]);
			std::cout << "Moved to positions PSG: " << PSG_pos[i] << " PSA: " << PSA_pos[i] << std::endl;

			// Include mono and filterwheel for each position
			std::string wavelength = "white spectrum";

			measure_and_save(&cam, path, std::to_string(PSG_pos[i]).substr(0, std::to_string(PSG_pos[i]).size() - 5), std::to_string(PSA_pos[i]).substr(0, std::to_string(PSA_pos[i]).size() - 5), wavelength);
		}

		driver.home(PSG_driver);
		driver.home(PSA_driver);

		std::cout << "\nSuccessfully acquired images and saved to file" << std::endl;

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