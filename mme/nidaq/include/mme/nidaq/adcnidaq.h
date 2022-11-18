#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <chrono>
#include <memory>
#include <atomic>

namespace mme 
{
	namespace detail {
		class NidaqTask {
		public:
			NidaqTask();
			NidaqTask(std::string task_name);

			//Move only type
			NidaqTask(const NidaqTask& other) = delete;
			NidaqTask& operator=(const NidaqTask& other) = delete;
			NidaqTask(NidaqTask&& other) = default;
			NidaqTask& operator=(NidaqTask&& other) = default;

			const std::string& name() const;
			void* handle();
		private:
			using cleanup_func_t = void(*)(void*);
			static void cleanup(void* handle);
		private:
			std::string m_name;
			std::unique_ptr<void, cleanup_func_t> m_handle;
			inline static std::atomic<size_t> s_tasks_created = 0;
		};
	}

	bool is_error(int nidaq_error_code);
	bool is_warning(int nidaq_error_code);
	bool is_ok(int nidaq_error_code);
	void throw_nidaq_error(int nidaq_error_code);
	void throw_if_error(int nidaq_error_code);

	struct VoltageRange {
		double min = 0.0;
		double max = 10.0;
	};

	enum class TriggerEdge {
		Rising,
		Falling
	};

	struct TriggerSettings {
		TriggerEdge edge = TriggerEdge::Falling;
		std::string source = "/Dev1/PFI0";
	};

	struct SamplingRate {
		double value = 100'000.0; //Hz
	};

	struct SamplingSettings {
		VoltageRange voltage_range;
		std::optional<SamplingRate> rate;
	};

	inline constexpr SamplingRate DEFAULT_SAMPLING_RATE{ 100'000 };

	class NidaqAdc {
	public:
		NidaqAdc(std::string channel, SamplingSettings settings = SamplingSettings{});

		//Move only type
		NidaqAdc(const NidaqAdc& other) = delete;
		NidaqAdc& operator=(const NidaqAdc& other) = delete;
		NidaqAdc(NidaqAdc&& other) = default;
		NidaqAdc& operator=(NidaqAdc&& other) = default;

		std::vector<double> sample(size_t num_samples);
		std::vector<double> sample(std::chrono::duration<double> duration);
		std::vector<double> sample(size_t num_samples, SamplingRate rate);
		std::vector<double> sample(std::chrono::duration<double> duration, SamplingRate rate);
	private:
		SamplingRate default_sampling_rate();
		std::vector<double> read_data(size_t num_samples, std::chrono::duration<double> timeout);
	private:
		detail::NidaqTask m_task;
		SamplingSettings m_settings;
	};
	
	class NidaqTriggeredAdc {
	public:
		NidaqTriggeredAdc(std::string channel, TriggerSettings trigger_settings = TriggerSettings{}, SamplingSettings sampling_settings = SamplingSettings{});

		//Move only type
		NidaqTriggeredAdc(const NidaqTriggeredAdc& other) = delete;
		NidaqTriggeredAdc& operator=(const NidaqTriggeredAdc& other) = delete;
		NidaqTriggeredAdc(NidaqTriggeredAdc&& other) = default;
		NidaqTriggeredAdc& operator=(NidaqTriggeredAdc&& other) = default;

		void sample_on_trigger(size_t num_samples);
		void sample_on_trigger(std::chrono::duration<double> duration);
		void sample_on_trigger(size_t num_samples, SamplingRate rate);
		void sample_on_trigger(std::chrono::duration<double> duration, SamplingRate rate);
		std::optional<std::vector<double>> retrieve_samples(std::chrono::duration<double> timeout);
	private:
		std::vector<double> read_data(size_t num_samples, std::chrono::duration<double> timeout);
	private:
		detail::NidaqTask m_task;
		SamplingSettings m_sampling_settings;
		TriggerSettings m_trigger_settings;
		bool m_is_armed;
		size_t m_samples_to_read;
	};

} //namespace mme