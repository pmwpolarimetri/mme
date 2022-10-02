#include "mme/lumenera/lumeneracamera.h"
#include <Windows.h>
#include "lucamapi.h"
#include <format>
#include <iostream>

mme::LumeneraCamera::LumeneraCamera(size_t camera_num)
	:m_camera_handle(std::unique_ptr<void, handle_cleaner_func_t>(LucamCameraOpen(camera_num), test_deleter))
{
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
	if (handle == 0) {
		auto close_ok = LucamCameraClose(handle);
		std::cout << "Closed Lumenera camera handle" << std::endl;
		//TODO: log if did not close correctly
		//handle = nullptr;
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

void mme::test_deleter(void* handle)
{
	std::cout << "test_deleter" << std::endl;
	if (handle == 0) {
		auto close_ok = LucamCameraClose(handle);
		std::cout << "Closed Lumenera camera handle" << std::endl;
		//TODO: log if did not close correctly
		//handle = nullptr;
	}
}
