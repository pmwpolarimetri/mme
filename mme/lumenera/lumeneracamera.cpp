#include "mme/lumenera/lumeneracamera.h"
#include <Windows.h>
#include "lucamapi.h"
#include <format>
#include <iostream>

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
	//if (!m_camera_handle) {
	//	throw std::runtime_error(std::format("Lumenera camera with number {} could not be opened, check if it is connected", camera_num));
	//}

}

//mme::LumeneraCamera::~LumeneraCamera()
//{
//	if (m_camera_handle) {
//		auto close_ok = LucamCameraClose(m_camera_handle);
//		//TODO: log if did not close correctly
//	}
//}
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
//
//mme::LumeneraCamera::LumeneraCamera(LumeneraCamera&& other)
//{
//}
//
//mme::LumeneraCamera& mme::LumeneraCamera::operator=(LumeneraCamera&& other)
//{
//	// TODO: insert return statement here
//}


