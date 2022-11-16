#pragma once
#include <stdexcept>
#include <string>

namespace mme {

	class NidaqError : public std::runtime_error {

	public:
		NidaqError(int error_code);
		NidaqError(int error_code, const std::string& what_arg);
		const char* what() const noexcept override;
	private:
		std::string get_error_info(int error_code);
	private:
		int m_error_code;
		std::string m_message;
	};


}