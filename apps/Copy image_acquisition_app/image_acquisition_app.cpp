// Program for aquiring data from the Mueller matrix microscope in D4-112
// 
// Written by Vilde Vraalstad, last modified December 2022
// 
// Status December 2022:
// - The monochromator still remains to be added.
//	 This work involves development of code for running through and saving measurements at different wavelengths and prism positions.
// - Only the Si-CCD Lumenera camera is included, not the InGaAs camera.
//
//
// The program manages all components of the MM-microscope, and performs a measurement routine for acquiring data with the specified parameters.
// How to use:
// - Specify measurement parameters in the first part of the script
// - Run program, by selecting x64-Release and image_acquisition_app.cpp as Startup Item, and pressing the green Play-button
//
// The included components:
// - Lumenera camera
// - Reference detector for acquring data on trigger
// - ESP driver for rotating PSG and PSA prisms
// - Filter wheel for dark correction and long pass filter (to remove the higher diffraction orders from the monochromator)
// Measurement routine:
// - Initialize camera, ESP drivers and filter wheel
//		- Camera binning is set to 4x4 pixels
// - Make new directory for saving files
// - Acquire dark measurement with camera and save to file
// - Apply the correct filter wheel for the given wavelength
// - For each prism position:
//		- Move the prisms to the correct position
//		- Acquire image with camera and reference detector, and save to files
// - Run data analysis script in Python with the acquired data of the measurement series, which saves the result to file
//		- The script will print a warning if the measurement is close to, or has reached, detector saturation
// 
// About data measurements, analysis and saving to file:
// - The data are saved in a new folder under C:/Users/PolarimetriD4-112/dev/mme/data/, with foldername specifying time of data acquisition, type of data and an user-specified description of the measurement.
//		The acquired images are saved as .npy-files, and the reference detector measurements as .txt-files.
// - A data analysis script in Python is automatically run on the acquired data after a series of measurements, and for k-space images, when centerspot_analysis is set to true, saving the acquired and corrected center spot intensities to .npy-files Intensities, and saving the calculated Mueller Matrix to .txt-files Measured MM for 633nm calibration.
// Thus, all full images are saved. However, only the result of the analysis for k-space images, Intensities.npy, are needed for further use and for computing Mueller matrices. The full images can be useful to keep to be able to change the data analysis routine, correct errors etc, but only Intensities.npy needs to be exported.


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


//////////////////// Specify the measurement parameters ////////////////////

// Select a description of the measurement to be included in the foldername:
std::string folderdescription = "Optimal angles";

// Select a wavelength of the measurement:
std::string wavelength = "633";

// K-space or real image?
bool kspace = true;

// If centerspot_analysis = true, the center spot intensity will be analysed for intensity-fit in the python data analysis script
bool centerspot_analysis = true;

// Transmission or reflection mode?
bool transmission = true;

// Select exposure time.
// Max 5 at 633nm to omit saturation of the detector. May need larger for longer wavelengths, and lower for shorter wavelengths. A warning is printed during data analysis if the measurement is close to, or has reached, saturation.
double exposure = 5;

//Chosse between measuring at the optimal angles (true), or at specific rotation increments (false)
bool optimalangles = true;

//Choose the rotation increments (in degrees) of the retarders, and number of measurements
double PSG_rotstep = 1;
double PSA_rotstep = 5;
int Nmeas = 361;

//The optimal angles of the retarders
std::vector<double> PSG_pos{-51.7076, -15.1964, 15.1964, 51.7076};
std::vector<double> PSA_pos{-51.7076, -15.1964, 15.1964, 51.7076};

//////////////////////////////// Code ////////////////////////////////

// Make a new directory under mme/data, with foldername specifying time of data acquisition, type of data and the user-specified description of the measurement.
std::string make_new_directory(std::string transorref, std::string kspaceorreal) {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char* timenow = std::ctime(&now);
	std::string foldername(timenow);
	foldername.erase(std::remove(foldername.begin(), foldername.end(), '\n'), foldername.cend());
	std::replace(foldername.begin(), foldername.end(), ':', '.');
	foldername = foldername + " " + folderdescription + " " + wavelength + "nm, " + kspaceorreal + ", " + transorref;

	std::string path = "C:/Users/PolarimetriD4-112/dev/mme/data/" + foldername; 

	std::filesystem::current_path("C:/Users/PolarimetriD4-112/dev/mme/data/");

	bool newdir = std::filesystem::create_directory(foldername);
	assert(newdir);
	std::cout << "Created new folder for saving files: " << foldername << std::endl;

	return path;
}

// Initialize camera
void camera_initialize(mme::LumeneraCamera *cam) {
	cam->set_exposure(mme::Exposure{ exposure });
	cam->set_image_size(mme::ImageSize{ .height = 2048, .width = 2048 });
	cam->set_binning(mme::Binning{ 4 });
	std::cout << "Initialized camera" << std::endl;
	return;
};

// Initialize ESP drivers
void driver_initialize(mme::ESPDriver* driver, int PSG_driver, int PSA_driver) {
	auto reply = driver->request("VE?\r\n");
	driver->home(PSG_driver);
	driver->home(PSA_driver);
	std::cout << "Initialized rotation motors: " << reply << std::endl;
}

// Function for making a camera and reference detector measurement, and saving to file.
void measure_and_save(mme::LumeneraCamera* cam, std::string path, std::string PSG_pos, std::string PSA_pos, std::string wavelength, int image_number, bool dark) {
	mme::NidaqTriggeredAdc adc_triggered{ "Dev1/ai0" };

	// When the camera captures an image, the reference detector is triggered, and will sample with a sampling rate of 25 000 during the exposure time (i.e. the camera shot)
	// Number_of_samples = exposure_time*sampling_rate, and their average value can then be used for intensity normalization in the data analysis
	adc_triggered.sample_on_trigger(std::chrono::duration<double>(0.001 * exposure) , mme::SamplingRate{ 25'000 }); //Trigger is armed.

	// Acquire an image and save to file
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

	// If not dark measurement, retrieve the sampled data from the reference detector buffer and save to file.
	if (!dark) {
		auto possible_samples = adc_triggered.retrieve_samples(2s); // Only data on success. Timeout 2s = the amount of time to wait for the function to read the samples. In most cases, it is finished much faster.
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

		// Initialize ESP drivers, camera and filter wheel

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

		// If set to measure at the optimal angles, then loop through and measure at the 16 positions.
		// Loops in an efficient way, going low-high-low-high-..., instead of low-high low-high ...
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

					//Add loop for mono here, for looping through and measuring all wavelengths
					// 
					// Choose the correct filter wheel depending on the wavelength
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
					measure_and_save(&cam, path, std::to_string(PSG_pos[i]).substr(0, std::to_string(PSG_pos[i]).size() - 5), std::to_string(PSA_pos_j).substr(0, std::to_string(PSA_pos_j).size() - 5), wavelength, i * PSG_pos.size() + j + 1, false);

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
				}
			}
			driver.home(PSG_driver);
			driver.home(PSA_driver);

			std::cout << "\n\nSuccessfully acquired " << PSG_pos.size()*PSA_pos.size() << " images and saved to files in folder " << path << std::endl;
		}

		// If set to scquire Nmeas images at specific rotation increments:
		else {
			for (int i = 0; i != Nmeas; ++i) {

				driver.move_twoaxes_absolute(PSG_driver, PSA_driver, PSG_rotstep * i, PSA_rotstep * i);

				std::cout << "Moved to positions PSG: " << PSG_rotstep * i << " PSA: " << PSA_rotstep * i << std::endl;

				//Add loop for mono here, for looping through and measuring all wavelengths

				// Choose the correct filter wheel depending on the wavelength

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

				measure_and_save(&cam, path, std::to_string(PSG_rotstep * i).substr(0, std::to_string(PSG_rotstep * i).size() - 5), std::to_string(PSA_rotstep * i).substr(0, std::to_string(PSA_rotstep * i).size() - 5), wavelength, i + 1, false);

			}

			driver.home(PSG_driver);
			driver.home(PSA_driver);

			std::cout << "\n\nSuccessfully acquired " << Nmeas << " images and saved to files in folder " << path << std::endl;
		}


		// Run python data analysis-script image_acquisition_app_dataanalysis.py, which runs the analysis in mme_microscope_dataanalysis.py on the acquired data
		// - Performs dark-correction, and prints a warning if saturation of the detector is close or reached.
		// - All acquired images are plotted.
		// - If the filename contains "k-space", a gaussian fit of the center spot is performed, the resulting peak intensities are saved to file, and the sample Mueller matrix calculated from intensity fit calibration at 633nm is saved to file.
		FILE* file;
		int argc = 6;
		wchar_t** wargv = new wchar_t* [argc];

		wargv[0] = Py_DecodeLocale("C:/Users/PolarimetriD4-112/dev/mme/apps/image_acquisition_app/image_acquisiton_app_dataanalysis.py", nullptr);
		wargv[1] = Py_DecodeLocale("-m", nullptr);
		wargv[2] = Py_DecodeLocale("/tmp/targets.lists", nullptr);
		wargv[3] = Py_DecodeLocale(path.c_str(), nullptr);
		wargv[4] = Py_DecodeLocale("C:/Users/PolarimetriD4 - 112/Anaconda3", nullptr);
		wargv[5] = Py_DecodeLocale(centerspot_analysis, nullptr);

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