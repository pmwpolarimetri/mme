#include "FWxCCommand.h"
#include "mme/fwxc/fwxc.h"
#include <array>
#include <ranges>
#include <stdexcept>
#include <format>
#include <assert.h>
#include <utility>


mme::Fwxc::Fwxc()
	:m_handle(0), m_num_filters(0), m_current_position(0)
{
	auto connected_devices = list_connected_devices();
	if (!connected_devices) {
		throw std::runtime_error("No FW filter wheels are connected");
	}
	std::optional<int> possible_handle;
	for (const auto& device_sn : *connected_devices) {
		possible_handle = connect_to_filter_wheel(device_sn);
		if (possible_handle) {
			break;
		}
	}
	if (!possible_handle) {
		throw std::runtime_error("Could not open any filter wheels");
	}
	m_handle = *possible_handle;
	collect_device_info();
}

mme::Fwxc::Fwxc(std::string_view serial_num)
	: m_handle(0), m_num_filters(0), m_current_position(0)
{
	auto possible_handle = connect_to_filter_wheel(serial_num);
	if (!possible_handle) {
		throw std::runtime_error(std::format("Could not open filter wheel with sn: {}", serial_num));
	}
	m_handle = *possible_handle;
	collect_device_info();
}

mme::Fwxc::~Fwxc()
{
	if (m_handle >= 0) {
		Close(m_handle);
	}
}

mme::Fwxc::Fwxc(Fwxc&& other) noexcept
{
	m_handle = std::exchange(other.m_handle, -1);
	m_num_filters = std::exchange(other.m_num_filters, 0);
	m_current_position = std::exchange(other.m_current_position, 0);
}

mme::Fwxc& mme::Fwxc::operator=(Fwxc&& other) noexcept
{
	if (this != &other) {
		m_handle = std::exchange(other.m_handle, -1);
		m_num_filters = std::exchange(other.m_num_filters, 0);
		m_current_position = std::exchange(other.m_current_position, 0);
	}
	return *this;
}

size_t mme::Fwxc::num_filters() const
{
	return m_num_filters;
}

size_t mme::Fwxc::current_filter_position() const
{
	return m_current_position;
}

bool mme::Fwxc::change_filter_position(size_t position)
{
	assert(position <= m_num_filters);
	auto err_code = SetPosition(m_handle, position);
	if (err_code < 0) {
		return false;
	}
	m_current_position = position;
	return true;
}

std::optional<int> mme::Fwxc::connect_to_filter_wheel(std::string_view serial_num)
{
	//TODO: add c string termination
	std::vector<char> sn{ serial_num.begin(), serial_num.end() };
	char serial[] = "FWV5KU2V";
	//int handle = Open(sn.data(), 115200, 5000);
	int handle = Open(serial, 115200, 2);
	if (handle < 0) {
		return std::nullopt;
	}
	auto is_open = IsOpen(serial);
	return handle;
}

size_t mme::Fwxc::read_num_filters()
{
	int count = -1;
	auto err_code = GetPositionCount(m_handle, &count);
	if (err_code != 0) {
		throw std::runtime_error("Could not read number of filters in filter wheel");
	}
	if (count <= 0) {
		throw std::runtime_error("Filter wheel reports zero filter num count, clearly wrong");
	}
	return static_cast<size_t>(count);
}

size_t mme::Fwxc::read_current_position()
{
	int position = -1;
	auto err_code = GetPosition(m_handle, &position);
	if (err_code != 0 || position < 0) {
		throw std::runtime_error("Could not read current filter wheel position");
	}
	return static_cast<size_t>(position);
}

void mme::Fwxc::collect_device_info()
{
	m_num_filters = read_num_filters();
	m_current_position = read_current_position();
}

std::optional<std::vector<std::string>> mme::list_connected_devices()
{
	//TODO: the List function returns pairs of "sn,other" not "sn1, sn2, sn3"
	//actual return: "sn1, str1, sn2, str2, sn3, str3..."
	std::array<unsigned char, 512> buffer;
	auto num_devices = List(buffer.data(), buffer.size());
	if (num_devices < 0) {
		return std::nullopt;
	}
	constexpr std::string_view delim{ "," };
	std::vector<std::string> serial_nums;
	for (auto&& line : std::views::split(buffer, delim)) {
		serial_nums.emplace_back(line.begin(), line.end());
	}
	return serial_nums;
}