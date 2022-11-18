#include "mme/nidaq/adcnidaq.h"
#include "mme/nidaq/nidaqerrors.h"
#include <iostream>
#include <format>

using namespace std::chrono_literals;

int main() {
	try
	{
		mme::NidaqAdc adc{ "Dev1/ai0" };								//default sampling rate is 100kHz
		//auto samples = adc.sample(20)									//sample 20 samples at default rate
		//auto samples = adc.sample(5ms);								//sample for 5 milliseconds at default rate
		//auto samples = adc.sample(5ms, mme::SamplingRate{25'000});	//sample for 5 milliseconds at 25kHz
		auto samples = adc.sample(10, mme::SamplingRate{ 25'000 });		//sample 10 samples at 25kHz

		std::cout << std::format("Printing {} samples", samples.size()) << std::endl;
		for (auto s : samples) {
			std::cout << s << std::endl;
		}
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