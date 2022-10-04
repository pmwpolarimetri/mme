#include "FWxCCommand.h"
#include "mme/fwxc/fwxc.h"
#include <array>
#include <ranges>
#include <stdexcept>
#include <format>
#include <assert.h>


mme::FW_filter_wheel::FW_filter_wheel()
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

mme::FW_filter_wheel::FW_filter_wheel(std::string_view serial_num)
	: m_handle(0), m_num_filters(0), m_current_position(0)
{
	auto possible_handle = connect_to_filter_wheel(serial_num);
	if (!possible_handle) {
		throw std::runtime_error(std::format("Could not open filter wheel with sn: {}", serial_num));
	}
	m_handle = *possible_handle;
	collect_device_info();
}

mme::FW_filter_wheel::~FW_filter_wheel()
{
	Close(m_handle);
}

size_t mme::FW_filter_wheel::num_filters() const
{
	return m_num_filters;
}

size_t mme::FW_filter_wheel::current_filter_position() const
{
	return m_current_position;
}

bool mme::FW_filter_wheel::change_filter_position(size_t position)
{
	assert(position <= m_num_filters);
	auto err_code = SetPosition(m_handle, position);
	if (err_code < 0) {
		return false;
	}
	m_current_position = position;
	return true;
}

std::optional<int> mme::FW_filter_wheel::connect_to_filter_wheel(std::string_view serial_num)
{
	std::vector<char> sn{ serial_num.begin(), serial_num.end() };
	int handle = Open(sn.data(), 19200, 5000);
	if (handle < 0) {
		return std::nullopt;
	}
	return handle;
}

size_t mme::FW_filter_wheel::read_num_filters()
{
	int count = -1;
	auto err_code = GetPositionCount(m_handle, &count);
	if (err_code < 0) {
		throw std::runtime_error("Could not read number of filters in filter wheel");
	}
	if (count == 0 || count < 0) {
		throw std::runtime_error("Filter wheel reports zero filter num count, clearly wrong");
	}
	return static_cast<size_t>(count);
}

size_t mme::FW_filter_wheel::read_current_position()
{
	int position = -1;
	auto err_code = GetPosition(m_handle, &position);
	if (err_code < 0 || position < 0) {
		throw std::runtime_error("Could not read current filter wheel position");
	}
	return static_cast<size_t>(position);
}

void mme::FW_filter_wheel::collect_device_info()
{
	m_num_filters = read_num_filters();
	m_current_position = read_current_position();
}

std::optional<std::vector<std::string>> mme::list_connected_devices()
{
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