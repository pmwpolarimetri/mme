#include "mme/lumenera/lumeneracamera.h"
#include <Windows.h>
#include "lucamapi.h"
#include <format>
#include <iostream>
#include <vector>
#include <stdexcept>

LUCAM_SNAPSHOT default_camera_settings() {
    LUCAM_SNAPSHOT camera_settings;
    camera_settings.format.flagsX = LUCAM_FRAME_FORMAT_FLAGS_BINNING;//frameFormat.flagsX;
    camera_settings.format.flagsY = LUCAM_FRAME_FORMAT_FLAGS_BINNING; //frameFormat.flagsY;
    camera_settings.format.height = 2048;//frameFormat.height;
    camera_settings.format.pixelFormat = LUCAM_PF_16;// frameFormat.pixelFormat;
    camera_settings.format.binningX = 1;
    camera_settings.format.binningY = 1;
    camera_settings.format.width = 2048;//frameFormat.width;
    camera_settings.format.xOffset = 0;//980;//0;//frameFormat.xOffset;
    camera_settings.format.yOffset = 0;//2;//312;//frameFormat.xOffset;
    camera_settings.bufferlastframe = FALSE;
    camera_settings.exposure = 50; // 50ms exposure
    camera_settings.exposureDelay = 0.0;
    camera_settings.flReserved1 = 0.0;
    camera_settings.flReserved2 = 0.0;
    camera_settings.gain = 1.0;
    camera_settings.gainBlue = 1.0;
    camera_settings.gainGrn1 = 1.0;
    camera_settings.gainGrn2 = 1.0;
    camera_settings.gainRed = 1.0;
    camera_settings.shutterType = LUCAM_SHUTTER_TYPE_GLOBAL;
    camera_settings.strobeDelay = 0.0;
    camera_settings.timeout = 60000 * 60; // timeout at 1h, testing dark rate
    camera_settings.ulReserved1 = 0;
    camera_settings.ulReserved2 = 0;
    camera_settings.useHwTrigger = FALSE;
    camera_settings.useStrobe = TRUE;

    return camera_settings;
}

bool change_camera_settings(void* camera_handle, LUCAM_SNAPSHOT settings) {
    const bool isDisabled = LucamDisableFastFrames(camera_handle);
    const bool isEnabled = LucamEnableFastFrames(camera_handle, &settings);
    return isDisabled && isEnabled;
}

mme::LumeneraCamera::LumeneraCamera(size_t camera_num)
	//:m_camera_handle(std::unique_ptr<void, handle_cleaner_func_t>(LucamCameraOpen(camera_num), test_deleter))
	: m_camera_handle(nullptr, close_handle)
{
	auto handle = LucamCameraOpen(camera_num);
	if (handle == NULL) {
		throw std::runtime_error(std::format("Lumenera camera with number {} could not be opened, check if it is connected", camera_num));
	}
	else {
		m_camera_handle = std::unique_ptr<void, handle_cleaner_func_t>(handle, LumeneraCamera::close_handle);
	}

    if (!write_default_camera_settings()) {
        throw std::runtime_error("Could not write default camera settings");
    }

}


mme::Image<float> mme::LumeneraCamera::capture_single()
{
    auto size = image_size();
    std::vector<uint16_t> bytes(size.height * size.width, 0);
    bool ok = LucamTakeFastFrame(m_camera_handle.get(), reinterpret_cast<uint8_t*>(bytes.data()));
    if (!ok) {
        throw std::runtime_error("Could not capture frame with Lumenera camera");
    }
    Image<float> image(0.0, size); //TODO: fix bug on image constructor with uninitialized memory
    for (size_t i = 0; auto& pixel : image.pixels()) {
        pixel = static_cast<float>(bytes[i] >> 4);
        i++;
    }
    return image;
}

mme::ImageSize mme::LumeneraCamera::image_size() const
{
    return m_properties.image_size;
}

void mme::LumeneraCamera::set_exposure(Exposure exposure)
{
    auto settings = default_camera_settings();
    settings.exposure = static_cast<float>(exposure.value);

    auto ok = change_camera_settings(m_camera_handle.get(), std::move(settings));
    if (!ok) {
        throw std::runtime_error("Failed to change exposure for Lumenera camera");
    }
    m_properties.exposure = exposure;
}

void mme::LumeneraCamera::set_image_size(ImageSize size)
{
    auto settings = default_camera_settings();
    settings.format.height = static_cast<unsigned long>(size.height);
    settings.format.width = static_cast<unsigned long>(size.width);
    
    auto ok = change_camera_settings(m_camera_handle.get(), std::move(settings));
    if (!ok) {
        throw std::runtime_error("Failed to change image size for Lumenera camera");
    }
    m_properties.image_size = size;
}

void mme::LumeneraCamera::set_binning(Binning bin)
{
    auto settings = default_camera_settings();
    settings.format.binningX = static_cast<unsigned short>(bin.value);
    settings.format.binningY = static_cast<unsigned short>(bin.value);

    auto ok = change_camera_settings(m_camera_handle.get(), std::move(settings));
    if (!ok) {
        throw std::runtime_error("Failed to change image size for Lumenera camera");
    }
    m_properties.image_size = ImageSize{ m_properties.image_size.height / bin.value, m_properties.image_size.width / bin.value };
    m_properties.binning = bin;
}

void mme::LumeneraCamera::close_handle(void* handle)
{
	//TODO: change to logging instead of stdout
	std::cout << "Starting to close Lumenera camera handle" << std::endl;
	auto close_ok = LucamCameraClose(handle);
	if (close_ok) {
		std::cout << "Closed Lumenera camera handle" << std::endl;
	}
	else {
		std::cout << "Could not close Lumenera camera handle" << std::endl;
	}
}

bool mme::LumeneraCamera::write_default_camera_settings()
{
    const bool isDisabled = LucamDisableFastFrames(m_camera_handle.get());
    auto settings = default_camera_settings();
    const bool isEnabled = LucamEnableFastFrames(m_camera_handle.get(), &settings);

    if (!isDisabled || !isEnabled) {
        return false;
    }

    m_properties.binning = Binning{ settings.format.binningX };
    m_properties.exposure = Exposure{ settings.exposure };
    m_properties.image_size = ImageSize{settings.format.height/settings.format.binningY, settings.format.width / settings.format.binningX };
}

bool mme::LumeneraCamera::change_properties(Properties properties)
{
    return false;
}
