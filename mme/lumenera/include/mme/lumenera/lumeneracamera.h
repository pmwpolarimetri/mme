#pragma once
#include "mme/imaging/image.h"
#include <memory>
#include <functional>
#include <variant>




namespace mme {

	struct Exposure {
		double value;
	};

	struct Binning {
		size_t value;
	};

	class LumeneraCamera {
	public:
		using Property = std::variant<Exposure, ImageSize, Binning>;

		LumeneraCamera(size_t camera_num = 1);
		
		LumeneraCamera(const LumeneraCamera& other) = delete;
		LumeneraCamera& operator=(const LumeneraCamera& other) = delete;
		
		~LumeneraCamera() = default;
		LumeneraCamera(LumeneraCamera&& other) = default;
		LumeneraCamera& operator=(LumeneraCamera&& other) = default;

		Image<float> capture_single();
		ImageSize image_size() const;

		void set_exposure(Exposure exposure);
		void set_image_size(ImageSize size);
		void set_binning(Binning bin);

		//void set_properties(const std::vector<Property>& properties);

		struct Properties {
			Exposure exposure;
			ImageSize image_size;
			Binning binning;
		};
	private:
		static void close_handle(void* handle);
		using handle_cleaner_func_t = void(*)(void*);

		bool write_default_camera_settings();


	private:
		std::unique_ptr<void, handle_cleaner_func_t> m_camera_handle;
		Properties m_properties;
	};

}