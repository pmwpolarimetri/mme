#include <iostream>
#include <format>
#include <span>
#include <memory>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>
#include <chrono>
#include <ctime>
#include <assert.h>
#include <stdio.h>
#include <Python.h>

#include "mme/lumenera/lumeneracamera.h"
#include "mme/imaging/image.h"
#include "mme/motion/espdriver.h"
#include "mme/fwxc/fwxc.h"
#include "mme/nidaq/adcnidaq.h"
#include "mme/nidaq/nidaqerrors.h"
#include "lucamapi.h"
#include "npy.hpp"
#include "asio.hpp"

using namespace std::chrono_literals;

// Type in a foldername of the measurement:
std::string folderdescription = "Optimal angles, 633nm";

// K-space or real image?
bool kspace = true;

// Transmission or reflection mode?
bool transmission = true;

std::string wavelength = "633";

double exposure = 5; //Max 5 to omit saturation of the detector


//Chosse between measuring at the optimal angles (true), or at specific rotation increments (false)
bool optimalangles = true;

//The optimal angles of the retarders
std::vector<double> PSG_pos{-51.7076, -15.1964, 15.1964, 51.7076};
std::vector<double> PSA_pos{-51.7076, -15.1964, 15.1964, 51.7076};

//The rotation increments (in degrees) of the retarders, and number of measurements
double PSG_rotstep = 1;
double PSA_rotstep = 5;
int Nmeas = 5;


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
	cam->set_exposure(mme::Exposure{ exposure });
	cam->set_image_size(mme::ImageSize{ .height = 2048, .width = 2048 });
	cam->set_binning(mme::Binning{ 4 });
	std::cout << "Initialized camera" << std::endl;
	return;
};

void driver_initialize(mme::ESPDriver* driver, int PSG_driver, int PSA_driver) {
	auto reply = driver->request("VE?\r\n");
	driver->home(PSG_driver);
	driver->home(PSA_driver);
	std::cout << "Initialized rotation motors: " << reply << std::endl;
}


void measure_and_save(mme::LumeneraCamera* cam, std::string path, std::string PSG_pos, std::string PSA_pos, std::string wavelength, int image_number, bool dark) {
	mme::NidaqTriggeredAdc adc_triggered{ "Dev1/ai0" };

	adc_triggered.sample_on_trigger(std::chrono::duration<double>(0.001 * exposure) , mme::SamplingRate{ 25'000 }); //Trigger is armed. Will sample 0.001*exposure*sampling_rate=125 samples during one camera shot

	auto image = cam->capture_single();
	std::filesystem::path filename;
	if (!dark) {
		std::cout << "Captured image number " << image_number << " at wavelength " << wavelength << ". Height: " << image.size().height << ", Width: " << image.size().width << std::endl;
		filename = std::filesystem::path(path + "/PSG" + PSG_pos + "PSA" + PSA_pos + "Wl" + wavelength + ".npy");
	}
	else {
		std::cout << "Captured dark image. Height: " << image.size().height << ", Width: " << image.size().width << std::endl;
		filename = std::filesystem::path(path + "/Dark measurement.npy");
	}

	mme::save_to_numpy(filename.string(), image);

	if (!dark) {
		auto possible_samples = adc_triggered.retrieve_samples(2s); //retrieve data. Only data on success. Timeout 2 seconds = The amount of time, in seconds, to wait for the function to read the sample(s).
		if (possible_samples) {
			std::ofstream fw(path + "/PSG" + PSG_pos + "PSA" + PSA_pos + "Wl" + wavelength + ".txt", std::ofstream::out);
			if (fw.is_open()) {
				for (auto s : possible_samples.value()) {
					fw << s << "\n";
				}
				fw.close();
			}
			else {
				std::cout << "Problem with opening file";
			}
		}
		else {
			std::cout << "Timeout" << std::endl;
		}
	}

	return;
};

int main()
{

	try {
		int PSG_driver;
		std::string transorref;
		if (transmission) {
			PSG_driver = 2;
			transorref = "trans";
		}
		else {
			PSG_driver = 3;
			transorref = "refl";
		}
		int PSA_driver = 1;

		std::string kspaceorreal;
		if (kspace) {
			kspaceorreal = "k-space";
		}
		else {
			kspaceorreal = "real";
		}

		std::string path = make_new_directory(transorref,kspaceorreal);

		mme::LumeneraCamera cam{};
		camera_initialize(&cam);

		mme::ESPDriver driver{ "COM1" };
		driver_initialize(&driver,PSG_driver,PSA_driver);

		mme::Fwxc filter_wheel{};
		std::cout << "Initialized filter wheel " << std::endl << std::endl;

		// Make dark measurement
		auto ok = filter_wheel.change_filter_position(6);
		std::cout << "Filter wheel position: " << filter_wheel.current_filter_position() << std::endl;
		measure_and_save(&cam, path, std::to_string(0), std::to_string(0), "dark", 0, true);



		// TODO: Include mono, and run through the spectrum

		if (std::stoi(wavelength) < 800) {
			auto ok = filter_wheel.change_filter_position(2);
			std::cout << "Filter wheel position: " << filter_wheel.current_filter_position() << std::endl;
		}
		else if (std::stoi(wavelength) < 1400) {
			auto ok = filter_wheel.change_filter_position(3);
			std::cout << "Filter wheel position: " << filter_wheel.current_filter_position() << std::endl;
		}
		else if (std::stoi(wavelength) < 2000) {
			auto ok = filter_wheel.change_filter_position(4);
			std::cout << "Filter wheel position: " << filter_wheel.current_filter_position() << std::endl;
		}
		else if (std::stoi(wavelength) < 2900) {
			auto ok = filter_wheel.change_filter_position(4);
			std::cout << "Filter wheel position: " << filter_wheel.current_filter_position() << std::endl;
		}

		if (optimalangles) {
			for (int i = 0; i != PSG_pos.size(); ++i) {
				for (int j = 0; j != PSA_pos.size(); ++j) {
					double PSA_pos_j;
					if (i % 2 != 0) {
						PSA_pos_j = PSA_pos[PSA_pos.size() - j - 1];
					}
					else {
						PSA_pos_j = PSA_pos[j];
					}
					driver.move_twoaxes_absolute(PSG_driver, PSA_driver, PSG_pos[i], PSA_pos_j);
					std::cout << "Moved to positions PSG: " << PSG_pos[i] << " PSA: " << PSA_pos_j << std::endl;

					measure_and_save(&cam, path, std::to_string(PSG_pos[i]).substr(0, std::to_string(PSG_pos[i]).size() - 5), std::to_string(PSA_pos_j).substr(0, std::to_string(PSA_pos_j).size() - 5), wavelength, i * PSG_pos.size() + j + 1, false);

				}
			}
			driver.home(PSG_driver);
			driver.home(PSA_driver);

			std::cout << "\n\nSuccessfully acquired " << PSG_pos.size()*PSA_pos.size() << " images and saved to files in folder " << path << std::endl;
		}

		else {
			for (int i = 0; i != Nmeas; ++i) {

				driver.move_twoaxes_absolute(PSG_driver, PSA_driver, PSG_rotstep * i, PSA_rotstep * i);

				std::cout << "Moved to positions PSG: " << PSG_rotstep * i << " PSA: " << PSA_rotstep * i << std::endl;

				measure_and_save(&cam, path, std::to_string(PSG_rotstep * i).substr(0, std::to_string(PSG_rotstep * i).size() - 5), std::to_string(PSA_rotstep * i).substr(0, std::to_string(PSA_rotstep * i).size() - 5), wavelength, i + 1, false);

			}

			driver.home(PSG_driver);
			driver.home(PSA_driver);

			std::cout << "\n\nSuccessfully acquired " << Nmeas << " images and saved to files in folder " << path << std::endl;
		}


		// Run python plotting-script

		FILE* file;
		int argc = 5;
		wchar_t** wargv = new wchar_t* [argc];

		wargv[0] = Py_DecodeLocale("C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/image_acquisiton_app_dataanalysis.py", nullptr);
		wargv[1] = Py_DecodeLocale("-m", nullptr);
		wargv[2] = Py_DecodeLocale("/tmp/targets.lists", nullptr);
		wargv[3] = Py_DecodeLocale(path.c_str(), nullptr);
		wargv[4] = Py_DecodeLocale("C:/Users/PolarimetriD4 - 112/Anaconda3", nullptr);

		Py_SetProgramName(wargv[0]);
		Py_Initialize();
		Py_SetPath(wargv[4]);
		Py_SetPythonHome(wargv[4]);
		PySys_SetArgv(argc, wargv);
		file = fopen("C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/image_acquisiton_app_dataanalysis.py", "r");
		PyRun_SimpleFile(file, "C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/image_acquisiton_app_dataanalysis.py");
		Py_Finalize();

		for (int i = 0; i < argc; i++) {
			PyMem_RawFree(wargv[i]);
			wargv[i] = nullptr;
		}
		delete[] wargv;
		wargv = nullptr;

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