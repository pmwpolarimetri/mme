#include "mme/motion/espdriver.h"
#include <iostream>
#include <istream>
#include <format>
#include <ranges>

mme::ESPDriver::ESPDriver(std::string_view com_port) : m_com_port(com_port), m_io_context(), m_serial_port(m_io_context, m_com_port)
{
	std::cout << std::format("Port opened: {}\n", m_serial_port.is_open());
	m_serial_port.set_option(asio::serial_port_base::baud_rate(19200));
	m_serial_port.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::hardware));
}

void mme::ESPDriver::command(std::string cmd)
{
	const auto bytes_to_write = cmd.size();
	auto bytes_written = asio::write(m_serial_port, asio::buffer(std::move(cmd)));
	assert(bytes_written == bytes_to_write);
}

std::string mme::ESPDriver::request(std::string req)
{
	//precondition: request only generates 1 reply (1 line)
	auto bytes_to_write = req.size();
	auto bytes_written = asio::write(m_serial_port, asio::buffer(std::move(req)));
	assert(bytes_written == bytes_written);

	return read_line();
}

void mme::ESPDriver::move_relative(size_t axis, double pos)
{
	command(move_relative_cmd(axis, pos));
	wait_for_motion_done(axis);
}

void mme::ESPDriver::move_absolute(size_t axis, double pos)
{
	command(move_absolute_cmd(axis, pos));
	wait_for_motion_done(axis);
}

void mme::ESPDriver::move_twoaxes_absolute(size_t axis1, size_t axis2, double pos1, double pos2)
{
	command(move_absolute_cmd(axis1, pos1));
	command(move_absolute_cmd(axis2, pos2));
	wait_for_motion_done(axis1);
	wait_for_motion_done(axis2);
}

void mme::ESPDriver::home(size_t axis)
{
	command(home_cmd(axis));
	wait_for_motion_done(axis);
}

//void mme::ESPDriver::move_relative(std::vector<size_t> axes, std::vector<double> positions)
//{
//	assert(axes.size() == positions.size());
//}

//void mme::ESPDriver::move_absolute(std::vector<size_t> axes, std::vector<double> positions)
//{
//	assert(axes.size() == positions.size());
//}

//void mme::ESPDriver::home(std::vector<size_t> axes, std::vector<double> positions)
//{
//	assert(axes.size() == positions.size());
//}

std::string mme::ESPDriver::read_line()
{
	constexpr auto delimiter = "\r\n";
	auto bytes_read = asio::read_until(m_serial_port, m_buffer, delimiter);
	std::istream is(&m_buffer);
	std::string reply;
	std::getline(is, reply);

	assert(m_buffer.size() == 0);

	return reply;
}

void mme::ESPDriver::wait_for_motion_done(size_t axis)
{
	//TODO: Timeout
	while (true)
	{
		auto reply = request(motion_done_req(axis));
		//parse
		int motion_status = std::stoi(reply);
		if (motion_status == 1) {
			break;
		}
	}
}

//void mme::ESPDriver::wait_for_motion_done(std::vector<size_t> axes)
//{
//}

std::string mme::ESPDriver::motion_done_req(size_t axis)
{
	return std::format("{}MD?\r\n", axis);
}

std::string mme::ESPDriver::move_relative_cmd(size_t axis, double pos)
{
	return std::format("{}PR{}\r\n", axis, pos);
}

std::string mme::ESPDriver::move_absolute_cmd(size_t axis, double pos)
{
	return std::format("{}PA{}\r\n", axis, pos);
}

std::string mme::ESPDriver::home_cmd(size_t axis)
{
	return std::format("{}OR2\r\n", axis);
}

