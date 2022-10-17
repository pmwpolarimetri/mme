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
std::string folderdescription = "Air";

// K-space or real image?
bool kspace = true;

// Transmission or reflection mode?
bool transmission = true;

//Type in the rotation increments (in degrees) of the retarders, and number of measurements
float PSG_rotstep = 7.5;
float PSA_rotstep = 37.5;
int Nmeas = 30;

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

void measure_and_save(mme::LumeneraCamera* cam, std::string path, std::string PSG_pos, std::string PSA_pos, std::string wavelength, int image_number) {
	auto image = cam->capture_single();
	std::cout << "Captured image number " << image_number <<  " at wavelength " << wavelength << ". Height: " << image.size().height << ", Width: " << image.size().width << std::endl;

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


		for (int i = 0; i != Nmeas; ++i) {

			driver.move_twoaxes_absolute(PSG_driver, PSA_driver, PSG_rotstep * i, PSA_rotstep * i);

			std::cout << "Moved to positions PSG: " << PSG_rotstep*i << " PSA: " << PSA_rotstep*i << std::endl;

			// Include mono and filterwheel for each position
			std::string wavelength = "white spectrum";
			measure_and_save(&cam, path, std::to_string(PSG_rotstep * i).substr(0, std::to_string(PSG_rotstep * i).size() - 5), std::to_string(PSA_rotstep * i).substr(0, std::to_string(PSA_rotstep * i).size() - 5), wavelength, i+1);

		}

		driver.home(PSG_driver);
		driver.home(PSA_driver);

		std::cout << "\nSuccessfully acquired " << Nmeas << " images and saved to files in folder " << path << std::endl;

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