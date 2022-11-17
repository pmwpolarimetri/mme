#include "mme/nidaq/adcnidaq.h"
#include "NIDAQmx.h"
#include <utility>
#include "mme/nidaq/nidaqerrors.h"
#include <cassert>
#include <memory>


using namespace std::chrono_literals;

bool mme::is_error(int nidaq_error_code)
{
	return nidaq_error_code < 0;
}

bool mme::is_warning(int nidaq_error_code)
{
	return nidaq_error_code > 0;
}

bool mme::is_ok(int nidaq_error_code)
{
	return nidaq_error_code == 0;
}

void mme::throw_nidaq_error(int nidaq_error_code)
{
	throw NidaqError(nidaq_error_code);
}

void mme::throw_if_error(int nidaq_error_code)
{
	if (is_error(nidaq_error_code)) {
		throw NidaqError(nidaq_error_code);
	}
}

mme::detail::NidaqTask::NidaqTask()
	:NidaqTask(std::to_string(++s_tasks_created))
{
}

mme::detail::NidaqTask::NidaqTask(std::string task_name)
	:m_name(std::move(task_name))
	,m_handle(nullptr, cleanup)
{
	void* handle = nullptr;
	throw_if_error(DAQmxCreateTask(m_name.c_str(), &handle));
	m_handle = std::unique_ptr<void, cleanup_func_t>(handle, NidaqTask::cleanup);
}

const std::string& mme::detail::NidaqTask::name() const
{
	return m_name;
}

void* mme::detail::NidaqTask::handle()
{
	return m_handle.get();
}

void mme::detail::NidaqTask::cleanup(void* handle)
{
	
	if (handle) {
		auto err_code = DAQmxClearTask(handle);
	}
}

mme::NidaqAdc::NidaqAdc(std::string channel, SamplingSettings settings)
	:m_task()
	,m_settings(settings)
{
	throw_if_error(DAQmxCreateAIVoltageChan(m_task.handle(), channel.c_str(), "TEST", DAQmx_Val_Diff, m_settings.voltage_range.min, m_settings.voltage_range.max, DAQmx_Val_Volts, NULL));
}

std::vector<double> mme::NidaqAdc::sample(size_t num_samples)
{
	if (!m_settings.rate) { m_settings.rate = default_sampling_rate(); }

	throw_if_error(DAQmxCfgSampClkTiming(m_task.handle(), NULL, m_settings.rate->value, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, num_samples));
	throw_if_error(DAQmxStartTask(m_task.handle()));

	const double min_timeout_s = 1.0; //min 1 sec timeout
	double timeout_s = 10 * (num_samples / m_settings.rate->value); //TODO: add as function param or sane default (calculated)
	if (timeout_s < min_timeout_s) {
		timeout_s = min_timeout_s;
	}
	return read_data(num_samples, std::chrono::duration<double, std::ratio<1>>(timeout_s));
}

std::vector<double> mme::NidaqAdc::sample(std::chrono::duration<double> duration)
{
	if (!m_settings.rate) { m_settings.rate = default_sampling_rate(); }
	auto num_samples = static_cast<size_t>(m_settings.rate->value * duration.count());
	return sample(num_samples);
}

std::vector<double> mme::NidaqAdc::sample(size_t num_samples, SamplingRate rate)
{
	//TODO: validate rate before updating settings
	m_settings.rate = rate;
	return sample(num_samples);
}

std::vector<double> mme::NidaqAdc::sample(std::chrono::duration<double> duration, SamplingRate rate)
{
	//TODO: validate rate before updating settings
	m_settings.rate = rate;
	auto num_samples = static_cast<size_t>(duration.count() * rate.value);
	assert(num_samples > 0);
	return sample(num_samples);
}

mme::SamplingRate mme::NidaqAdc::default_sampling_rate()
{
	return SamplingRate(100'000);
}

std::vector<double> mme::NidaqAdc::read_data(size_t num_samples, std::chrono::duration<double> timeout)
{
	std::vector<double> buffer(num_samples);
	int32 num_samples_read = 0;
	throw_if_error(DAQmxReadAnalogF64(m_task.handle(), -1, timeout.count(), DAQmx_Val_GroupByChannel, buffer.data(), buffer.size(), &num_samples_read, NULL));
	throw_if_error(DAQmxStopTask(m_task.handle()));
	assert(num_samples == num_samples_read);
	return buffer;
}


mme::NidaqTriggeredAdc::NidaqTriggeredAdc(std::string channel, TriggerSettings trigger_settings, SamplingSettings sampling_settings)
	:m_task()
	,m_sampling_settings(std::move(sampling_settings))
	,m_trigger_settings(std::move(trigger_settings))
	,m_is_armed(false)
	,m_samples_to_read(0)
{
	throw_if_error(DAQmxCreateAIVoltageChan(m_task.handle(), channel.c_str(), "TEST", DAQmx_Val_Diff, m_sampling_settings.voltage_range.min, m_sampling_settings.voltage_range.max, DAQmx_Val_Volts, NULL));
	int code{};
	switch (m_trigger_settings.edge) {
		case TriggerEdge::Rising: code = DAQmxCfgDigEdgeStartTrig(m_task.handle(), m_trigger_settings.source.c_str(), DAQmx_Val_Rising);
		case TriggerEdge::Falling: code = DAQmxCfgDigEdgeStartTrig(m_task.handle(), m_trigger_settings.source.c_str(), DAQmx_Val_Falling);
	}
	throw_if_error(code);
}

void mme::NidaqTriggeredAdc::sample_on_trigger(size_t num_samples)
{
	if (m_is_armed) { return; } //TODO: log
	if (!m_sampling_settings.rate) { m_sampling_settings.rate = DEFAULT_SAMPLING_RATE; }

	throw_if_error(DAQmxCfgSampClkTiming(m_task.handle(), NULL, m_sampling_settings.rate->value, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, num_samples));
	throw_if_error(DAQmxStartTask(m_task.handle()));

	m_is_armed = true;
	m_samples_to_read = num_samples;
	return;
}

void mme::NidaqTriggeredAdc::sample_on_trigger(std::chrono::duration<double> duration)
{
	if (m_is_armed) { return; } //TODO: log
	if (!m_sampling_settings.rate) { m_sampling_settings.rate = DEFAULT_SAMPLING_RATE; }
	auto num_samples = static_cast<size_t>(m_sampling_settings.rate->value * duration.count());
	return sample_on_trigger(num_samples);
}

void mme::NidaqTriggeredAdc::sample_on_trigger(size_t num_samples, SamplingRate rate)
{
	if (m_is_armed) { return; } //TODO: log
	//TODO: validate rate before updating settings
	m_sampling_settings.rate = rate;
	return sample_on_trigger(num_samples);
}

void mme::NidaqTriggeredAdc::sample_on_trigger(std::chrono::duration<double> duration, SamplingRate rate)
{
	if (m_is_armed) { return; } //TODO: log
	//TODO: validate rate before updating settings
	m_sampling_settings.rate = rate;
	auto num_samples = static_cast<size_t>(duration.count() * rate.value);
	assert(num_samples > 0);
	return sample_on_trigger(num_samples);
}

std::optional<std::vector<double>> mme::NidaqTriggeredAdc::retrieve_samples(std::chrono::duration<double> timeout)
{
	if (!m_is_armed) { return std::nullopt; }
	return read_data(m_samples_to_read, timeout);
}


std::vector<double> mme::NidaqTriggeredAdc::read_data(size_t num_samples, std::chrono::duration<double> timeout)
{
	std::vector<double> buffer(num_samples);
	int32 num_samples_read = 0;
	throw_if_error(DAQmxReadAnalogF64(m_task.handle(), -1, timeout.count(), DAQmx_Val_GroupByChannel, buffer.data(), buffer.size(), &num_samples_read, NULL));
	throw_if_error(DAQmxStopTask(m_task.handle()));
	assert(num_samples == num_samples_read);
	return buffer;
}
