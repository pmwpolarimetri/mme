#include "mme/nidaq/nidaqerrors.h"
#include "NIDAQmx.h"
#include <array>
#include <string>
#include <vector>

mme::NidaqError::NidaqError(int error_code)
	:std::runtime_error("")
	,m_error_code(error_code)
	,m_message(get_error_info(error_code))
{
}

mme::NidaqError::NidaqError(int error_code, const std::string& what_arg)
	:std::runtime_error(what_arg)
	,m_error_code(error_code)
	, m_message(get_error_info(error_code))
{
}

const char* mme::NidaqError::what() const noexcept
{
	return m_message.c_str();
}

std::string mme::NidaqError::get_error_info(int error_code)
{
	auto default_message = "Could not get error description";
	auto size = DAQmxGetErrorString(m_error_code, NULL, 0);
	if (size < 0) {
		return default_message;
	}
	const size_t buffer_size = static_cast<size_t>(2*size);
	std::vector<char> buffer(buffer_size, '\0' );

	auto code = DAQmxGetErrorString(m_error_code, buffer.data(), buffer_size);
	if (code < 0) {
		return default_message;
	}
	return std::string{ buffer.begin(), buffer.end() };
}
