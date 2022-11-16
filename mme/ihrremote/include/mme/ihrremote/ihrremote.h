#pragma once
#include <string>
#include "mme/ihr/ihr.h"

namespace mme {

	class Ihr550Remote {

	public:
		Ihr550Remote(std::string device);

		//move only
		Ihr550Remote(const Ihr550Remote& other) = delete;
		Ihr550Remote& operator=(const Ihr550Remote& other) = delete;

		Ihr550Remote(Ihr550Remote&& other) = default;
		Ihr550Remote& operator=(Ihr550Remote&& other) = default;

		void change_wavelength(double wavelength);
		void change_slit(double slit_width, PortLocation loc);
		void change_grating(size_t grating_num);
		void open_port(PortLocation loc);
		void close_port(PortLocation loc);

	private:
		//void get_mono_state();
		/*double query_wavelength();
		PortLocation query_entrance_port();
		PortLocation query_exit_port();
		double query_slit_width(PortLocation loc);
		size_t query_grating();*/
	};

}