#pragma once
#include "mme/imaging/image.h"
#include <memory>
#include <functional>

struct Exposure {
	double value;
};

struct Binning {
	size_t value;
};


namespace mme {

	class LumeneraCamera {
	public:
		LumeneraCamera(size_t camera_num = 1);
		
		LumeneraCamera(const LumeneraCamera& other) = delete;
		LumeneraCamera& operator=(const LumeneraCamera& other) = delete;
		
		~LumeneraCamera() = default;
		LumeneraCamera(LumeneraCamera&& other) = default;
		LumeneraCamera& operator=(LumeneraCamera&& other) = default;

		void capture_single();
		ImageSize image_size() const;

		//void set_exposure(double ms);
		//void set_binning();
	private:
		static void close_handle(void* handle);
		using handle_cleaner_func_t = void(*)(void*);

		bool write_default_camera_settings();

	private:
		std::unique_ptr<void, handle_cleaner_func_t> m_camera_handle;
		//void* m_camera_handle;
	};

}