#include "mme/ihr/ihr.h"
#include <string>
#include "combaseapi.h"

mme::Ihr550::Ihr550(std::string device)
{
}

void mme::Ihr550::change_wavelength(double wavelength)
{
}

void mme::Ihr550::change_slit(double slit_width, PortLocation loc)
{
}

void mme::Ihr550::change_grating(size_t grating_num)
{
}

void mme::Ihr550::open_port(PortLocation loc)
{
}

void mme::Ihr550::close_port(PortLocation loc)
{
}

void mme::Ihr550::get_mono_state()
{
	//query mono for state
}

mme::Ihr550::CoInit::CoInit()
	:m_initialized(false)
{
	auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)) {
		m_initialized = true;
	}
}

mme::Ihr550::CoInit::~CoInit()
{
	if (m_initialized) {
		CoUninitialize();
	}
}

mme::Ihr550::CoInit::CoInit(CoInit&& other) noexcept
{
	m_initialized = std::exchange(other.m_initialized, false);
}

mme::Ihr550::CoInit& mme::Ihr550::CoInit::operator=(CoInit&& other) noexcept
{
	if (this != &other) {
		m_initialized = std::exchange(other.m_initialized, false);
	}
	return *this;
}
