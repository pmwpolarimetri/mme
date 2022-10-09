#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <optional>

namespace mme {

	class Fwxc {

	public:
		Fwxc();
		Fwxc(std::string_view serial_num);
		~Fwxc();

		Fwxc(const Fwxc& other) = delete;
		Fwxc& operator=(const Fwxc& other) = delete;

		Fwxc(Fwxc&& other) noexcept;
		Fwxc& operator=(Fwxc&& other) noexcept;

		size_t num_filters() const;
		size_t current_filter_position() const;
		[[nodiscard]] bool change_filter_position(size_t position);

	private:
		std::optional<int> connect_to_filter_wheel(std::string_view serial_num);
		size_t read_num_filters();
		size_t read_current_position();
		void collect_device_info();
		
	private:
		int m_handle;
		size_t m_num_filters;
		size_t m_current_position;
		
	};

	std::optional<std::vector<std::string>> list_connected_devices();

} //namespace mme