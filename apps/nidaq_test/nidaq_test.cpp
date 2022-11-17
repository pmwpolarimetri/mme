#include "mme/nidaq/adcnidaq.h"
#include "mme/nidaq/nidaqerrors.h"
#include <iostream>

int main() {

	try
	{
		mme::NidaqAdc adc{ "Dev1/ai0" };
		auto result = adc.sample(100);
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