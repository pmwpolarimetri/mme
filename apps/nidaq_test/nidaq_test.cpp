#include "mme/nidaq/adcnidaq.h"
#include "mme/nidaq/nidaqerrors.h"
#include "mme/lumenera/lumeneracamera.h" //IDE henger
#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <filesystem>
#include <Windows.h>
using namespace std::chrono_literals;

int main() {
	try
	{
		double exposure_time = 5; //ms

		mme::LumeneraCamera cam{};
		cam.set_exposure(mme::Exposure{ exposure_time });

		mme::NidaqTriggeredAdc adc_triggered{ "Dev1/ai0" };
		adc_triggered.sample_on_trigger(std::chrono::duration<double>(0.001 * exposure_time), mme::SamplingRate{ 25'000 }); //trigger is armed
		//trigger happens here
		//prøv en camera capture her, burde være kobling fra kamera til dac kortet
		//sett number of samples slik at camera exposure = adc tid
		auto image = cam.capture_single();
		auto possible_samples = adc_triggered.retrieve_samples(2s); //retrieve data. Only data on success. Timeout 2 seconds.
		if (possible_samples) {
			//samples available
			std::cout << std::format("Printing {} samples", possible_samples.value().size()) << std::endl;
			for (auto s : possible_samples.value()) {
				std::cout << s << std::endl;
			}
		}
		else {
			std::cout << "Timeout" << std::endl;
		}


		//double exposure_time = 25; //ms
		//
		//mme::LumeneraCamera cam{};
		//
		//cam.set_exposure(mme::Exposure{ exposure_time });
		//
		//for (int i = 0; i < 3; i++) {
		//
		//	mme::NidaqTriggeredAdc adc_triggered{ "Dev1/ai0" };
		//
		//	adc_triggered.sample_on_trigger(std::chrono::duration<double>(0.001 * exposure_time), mme::SamplingRate{ 1000 }); //trigger is armed
		//
		//	//trigger happens here
		//	//prøv en camera capture her, burde være kobling fra kamera til dac kortet
		//	auto image = cam.capture_single();
		//	auto filename = std::filesystem::path("C:/Users/PolarimetriD4-112/dev/mme/data/test" + std::to_string(i) + ".npy");
		//	mme::save_to_numpy(filename.string(), image);
		//
		//	//sett number of samples slik at camera exposure = adc tid
		//
		//	auto possible_samples = adc_triggered.retrieve_samples(2s); //retrieve data. Only data on success. Timeout 2 seconds = The amount of time, in seconds, to wait for the function to read the sample(s).
		//	if (possible_samples) {
		//		//samples available
		//		std::cout << std::format("Printing {} samples", possible_samples.value().size()) << std::endl;
		//		std::ofstream fw("C:/Users/PolarimetriD4-112/dev/mme/data/test" + std::to_string(i) + ".txt", std::ofstream::out);
		//		if (fw.is_open()) {
		//			for (auto s : possible_samples.value()) {
		//				std::cout << s << std::endl;
		//				fw << s << "\n";
		//			}
		//			fw.close();
		//		}
		//		else {
		//			std::cout << "Problem with opening file";
		//		}
		//	}
		//	else {
		//		std::cout << "Timeout" << std::endl;
		//	}
		//
		//	std::this_thread::sleep_for(5s);
		//}




		//mme::NidaqAdc adc{ "Dev1/ai0" };								//default sampling rate is 100kHz
		////auto samples = adc.sample(20)									//sample 20 samples at default rate
		////auto samples = adc.sample(5ms);								//sample for 5 milliseconds at default rate
		////auto samples = adc.sample(5ms, mme::SamplingRate{25'000});	//sample for 5 milliseconds at 25kHz
		//auto samples = adc.sample(10, mme::SamplingRate{ 25'000 });		//sample 10 samples at 25kHz

		//std::cout << std::format("Printing {} samples", samples.size()) << std::endl;
		//for (auto s : samples) {
		//	std::cout << s << std::endl;
		//}
	}
	catch (const mme::NidaqError & nidaq_err)
	{
		std::cout << nidaq_err.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}


	return 0;
}