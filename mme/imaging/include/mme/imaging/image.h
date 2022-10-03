#pragma once
#include <vector>
#include <span>
#include <format>
#include <assert.h>
#include <string_view>
#include "npy.hpp"

namespace mme {

	struct ImageSize {
		size_t height;
		size_t width;
	};

	constexpr size_t num_pixels(const ImageSize& size) {
		return size.height * size.width;
	}

	template<typename Pixel>
	struct ImageView {
		ImageView() = default;
		ImageView(std::span<Pixel> pixels, ImageSize size) : m_pixels(pixels), m_size(size) {}

		Pixel& operator() (size_t row, size_t col) {
			return m_pixels[linear_index(row, col)];
		}

		const Pixel& operator() (size_t row, size_t col) const {
			return m_pixels[linear_index(row, col)];
		}

		Pixel& at(size_t row, size_t col) {
			return m_pixels[linear_index(row, col)];
		}

		const Pixel& at(size_t row, size_t col) const {
			return m_pixels[linear_index(row, col)];
		}

		std::span<Pixel> row(size_t row) {
			return m_pixels.subspan(linear_index(row, 0), m_size.width);
		}

		std::span<const Pixel> row(size_t row) const {
			return m_pixels.subspan(linear_index(row, 0), m_size.width);
		}

		ImageView<Pixel> sub_view(size_t row_start, size_t num_rows) {
			return ImageView<Pixel>(m_pixels.subspan(linear_index(row_start, 0), m_size.width * num_rows), { num_rows, m_size.width });
		}

		ImageView<const Pixel> sub_view(size_t row_start, size_t num_rows) const {
			return ImageView<Pixel>(m_pixels.subspan(linear_index(row_start, 0), m_size.width * num_rows), { num_rows, m_size.width });
		}
		ImageView<Pixel> sub_view(size_t row_start) {
			return ImageView<Pixel>(m_pixels.subspan(row_start, std::dynamic_extent), { m_size.height - row_start, m_size.width });
		}

		ImageView<const Pixel> sub_view(size_t row_start) const {
			return ImageView<Pixel>(m_pixels.subspan(row_start, std::dynamic_extent), { .height = m_size.height - row_start, .width = m_size.width });
		}

		size_t num_rows() const {
			return m_size.height;
		}

		size_t num_cols() const {
			return m_size.width;
		}

		ImageSize size() const {
			return m_size;
		}

		std::span<Pixel> pixels() {
			return m_pixels;
		}

		std::span<const Pixel> pixels() const {
			return m_pixels;
		}

	private:
		size_t linear_index(size_t row, size_t col) {
			return col + row * m_size.width;
		}

	private:
		std::span<Pixel> m_pixels;
		ImageSize m_size;
	};


	template<typename Pixel>
	struct Image {
		Image(std::vector<Pixel> pixels, ImageSize size)
			: m_pixels(std::move(pixels))
			, m_view(m_pixels, size)
		{
		}
		Image(std::span<Pixel> pixels, ImageSize size)
			: m_pixels(pixels.begin(), pixels.end())
			, m_view(m_pixels, size)
		{
		}
		Image(Pixel fill_value, ImageSize size)
			: m_pixels(num_pixels(size), fill_value), m_view(m_pixels, size)
		{
		}

		Image(ImageSize size)
		{
			m_pixels.reserve(num_pixels(size)); //logical length of owning vector is 0
			m_view = ImageView<Pixel>{ std::span<Pixel>{m_pixels.data(), num_pixels(size)}, size};
		}

		ImageSize size() const { return m_view.size(); }
		std::span<const Pixel> pixels() const { return m_view.pixels(); }
		std::span<Pixel> pixels() { return m_view.pixels(); }

		ImageView<Pixel> as_view() {
			return m_view;
		}

		ImageView<const Pixel> as_view() const {
			return ImageView<const Pixel>(m_view.pixels(), m_view.size());
		}
	private:
		std::vector<Pixel> m_pixels;
		ImageView<Pixel> m_view;
	};


	template<typename T>
	void save_to_numpy(const std::string& filename, ImageView<T> image_view) {
		const unsigned long num_rows = image_view.size().height;
		const unsigned long num_cols = image_view.size().width;
		const unsigned long shape[] = { num_rows, num_cols };
		npy::SaveArrayAsNumpy(filename, false, 2, shape, image_view.pixels().data());		
	}

	template<typename T>
	void save_to_numpy(const std::string& filename, const Image<T>& image) {
		save_to_numpy(filename, image.as_view());
	}
}