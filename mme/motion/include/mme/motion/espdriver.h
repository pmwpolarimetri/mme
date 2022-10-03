#pragma once
#include <string_view>
#include <string>
#include <optional>
#include "asio.hpp"

namespace mme {

	//class ESPDriver;

	//class ESPAxis {
	//	friend class ESPDriver;
	//public:
	//	ESPAxis() = delete;
	//	ESPAxis(const ESPAxis&) = delete;
	//	ESPAxis& operator= (const ESPAxis&) = delete;

	//	void move_relative(double pos);
	//	void move_absolute(double pos);
	//	void home();
	//private:
	//	ESPAxis(size_t axis);
	//private:
	//	size_t m_axis;
	//};

	class ESPDriver{
	public:
		ESPDriver(std::string_view com_port);
		void command(std::string cmd);
		std::string request(std::string req);

		//ESPAxis& axis(size_t axis);
		//const ESPAxis& axis(size_t axis) const;

		void move_relative(size_t axis, double pos);
		void move_absolute(size_t axis, double pos);
		void home(size_t axis);

		//void move_relative(std::vector<size_t> axes, std::vector<double> positions);
		//void move_absolute(std::vector<size_t> axes, std::vector<double> positions);
		//void home(std::vector<size_t> axes, std::vector<double> positions);

	private:
		std::string read_line();
		void wait_for_motion_done(size_t axis);
		//void wait_for_motion_done(std::vector<size_t> axes);
		static std::string motion_done_req(size_t axis);
		static std::string move_relative_cmd(size_t axis, double pos);
		static std::string move_absolute_cmd(size_t axis, double pos);
		static std::string home_cmd(size_t axis);


	private:
		std::string m_com_port;
		asio::io_context m_io_context;
		//asio::io_service m_io_service;
		asio::serial_port m_serial_port;
		std::string m_serial_num;
		asio::streambuf m_buffer;

	};


} //namespace mme