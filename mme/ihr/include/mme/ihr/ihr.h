#pragma once
#include <string>

namespace mme {

	enum class PortLocation {
		FrontEntrance,
		FrontExit,
		SideEntrance,
		SideExit
	};

	class Ihr550 {

	public:
		Ihr550(std::string device);

		//move only
		Ihr550(const Ihr550& other) = delete;
		Ihr550& operator=(const Ihr550& other) = delete;

		Ihr550(Ihr550&& other) = default;
		Ihr550& operator=(Ihr550&& other) = default;

		void change_wavelength(double wavelength);
		void change_slit(double slit_width, PortLocation loc);
		void change_grating(size_t grating_num);
		void open_port(PortLocation loc);
		void close_port(PortLocation loc);

	private:
		struct CoInit {
			CoInit();
			~CoInit();
			CoInit(const CoInit& other) = delete;
			Ihr550::CoInit& operator=(const CoInit& other) = delete;
			CoInit(CoInit&& other) noexcept;
			Ihr550::CoInit& operator=(CoInit&& other) noexcept;
		private:
			bool m_initialized{ false };
		};
	private:
		void get_mono_state();
		/*double query_wavelength();
		PortLocation query_entrance_port();
		PortLocation query_exit_port();
		double query_slit_width(PortLocation loc);
		size_t query_grating();*/
	private:
		CoInit m_coinit; //needed for COM
	};


} //namespace mme