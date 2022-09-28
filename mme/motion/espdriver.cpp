#include "mme/motion/espdriver.h"
#include <iostream>
#include <format>

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
	auto bytes_written = asio::write(m_serial_port, asio::buffer(std::move(req)));
	//auto bytes_written = m_serial_port.write_some(asio::buffer(std::move(req)));
	std::cout << std::format("wrote: {} bytes\n", bytes_written);
	constexpr auto delimiter = "\r\n";
	std::string reply;
	//auto bytes_read = m_serial_port.read_some(asio::dynamic_buffer(reply));
	auto bytes_read = asio::read(m_serial_port, asio::dynamic_buffer(reply, 1));
	//auto bytes_read = asio::read_until(m_serial_port, asio::dynamic_buffer(reply), delimiter);
	std::cout << std::format("bytes read: {}\n", bytes_read);
	return reply;

	//std::string line = reply.substr(0, bytes_read);
	//reply.erase(0, bytes_read);
	////TODO: assert that only one line is received
	//line.erase(std::remove(line.begin(), line.end(), '\r\n'), line.cend()); //remove newline chars
	//return line;
}

void mme::ESPDriver::move_relative(size_t axis, double pos)
{
}

void mme::ESPDriver::move_absolute(size_t axis, double pos)
{
}

void mme::ESPDriver::home(size_t axis)
{
}

//void mme::ESPDriver::move_relative(std::vector<size_t> axes, std::vector<double> positions)
//{
//	assert(axes.size() == positions.size());
//}

void mme::ESPDriver::move_absolute(std::vector<size_t> axes, std::vector<double> positions)
{
	assert(axes.size() == positions.size());
}

void mme::ESPDriver::home(std::vector<size_t> axes, std::vector<double> positions)
{
	assert(axes.size() == positions.size());
}

void mme::ESPDriver::wait_for_motion_done(size_t axis)
{
	//TODO: Timeout
	while (true)
	{
		auto reply = request(motion_done_req(axis));
		//parse
		int reply_as_int = std::stoi(reply);
		if (reply_as_int == 1) {
			break;
		}
	}
}

void mme::ESPDriver::wait_for_motion_done(std::vector<size_t> axes)
{
}

std::string mme::ESPDriver::motion_done_req(size_t axis)
{
	return std::string();
}