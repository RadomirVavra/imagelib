#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef IMAGELIB_USE_HALF
#define IMAGELIB_USE_HALF 1
#endif

#if IMAGELIB_USE_HALF
#include <imagelib/third_party/half.hpp>
#endif

namespace image {

#if IMAGELIB_USE_HALF
using half = half_float::half;

static_assert(sizeof(half) == 2);
static_assert(std::is_trivially_copyable_v<half>);
#endif

struct Rect {
    std::ptrdiff_t row = 0;
    std::ptrdiff_t column = 0;
    std::size_t height = 0;
    std::size_t width = 0;
};

enum class BorderMode { constant, clamp, wrap, reflect, reflect101 };
enum class ConversionPolicy { numeric, normalized_unorm, normalized_snorm };

namespace detail {

template<class T>
using unqualified_t = std::remove_cv_t<T>;

template<class T>
inline constexpr bool supported_scalar_v =
    std::same_as<unqualified_t<T>, std::int8_t> ||
    std::same_as<unqualified_t<T>, std::uint8_t> ||
    std::same_as<unqualified_t<T>, std::int16_t> ||
    std::same_as<unqualified_t<T>, std::uint16_t> ||
    std::same_as<unqualified_t<T>, std::int32_t> ||
    std::same_as<unqualified_t<T>, std::uint32_t> ||
    std::same_as<unqualified_t<T>, std::int64_t> ||
    std::same_as<unqualified_t<T>, std::uint64_t> ||
    std::same_as<unqualified_t<T>, float> ||
    std::same_as<unqualified_t<T>, double>
#if IMAGELIB_USE_HALF
    || std::same_as<unqualified_t<T>, half>
#endif
    ;

template<class T>
inline constexpr bool floating_scalar_v = std::floating_point<unqualified_t<T>>
#if IMAGELIB_USE_HALF
    || std::same_as<unqualified_t<T>, half>
#endif
    ;

template<class T>
long double to_long_double(T value) {
#if IMAGELIB_USE_HALF
    if constexpr (std::same_as<unqualified_t<T>, half>) {
        return static_cast<float>(value);
    } else {
        return static_cast<long double>(value);
    }
#else
    return static_cast<long double>(value);
#endif
}

inline std::size_t checked_add(std::size_t left, std::size_t right) {
    if (right > std::numeric_limits<std::size_t>::max() - left) {
        throw std::length_error("image size overflow");
    }
    return left + right;
}

inline std::size_t checked_mul(std::size_t left, std::size_t right) {
    if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left) {
        throw std::length_error("image size overflow");
    }
    return left * right;
}

inline std::ptrdiff_t checked_ptrdiff(std::size_t value) {
    if (value > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())) {
        throw std::length_error("image stride overflow");
    }
    return static_cast<std::ptrdiff_t>(value);
}

inline std::ptrdiff_t checked_add(std::ptrdiff_t left, std::ptrdiff_t right) {
    if ((right > 0 && left > std::numeric_limits<std::ptrdiff_t>::max() - right) ||
        (right < 0 && left < std::numeric_limits<std::ptrdiff_t>::min() - right)) {
        throw std::length_error("image coordinate overflow");
    }
    return left + right;
}

inline std::ptrdiff_t checked_mul(std::ptrdiff_t left, std::size_t right) {
    if (left == 0) {
        return 0;
    }
    if (left < 0 || right > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) /
                                 static_cast<std::size_t>(left)) {
        throw std::length_error("image stride overflow");
    }
    return left * static_cast<std::ptrdiff_t>(right);
}

inline std::size_t ceil_div(std::size_t value, std::size_t divisor) {
    return value == 0 ? 0 : 1 + (value - 1) / divisor;
}

} // namespace detail

template<class T>
    requires detail::supported_scalar_v<T>
class ImageView;

template<class T>
    requires(detail::supported_scalar_v<T> && !std::is_const_v<T>)
class Image;

namespace detail {

template<class T>
Image<std::remove_const_t<T>> crop_impl(ImageView<T> source, Rect region, BorderMode border,
                                        std::span<const std::remove_const_t<T>> outside);

inline std::size_t checked_count(std::size_t height, std::size_t width, std::size_t channels) {
    if (channels == 0) {
        throw std::invalid_argument("image channels must be nonzero");
    }
    return checked_mul(checked_mul(height, width), channels);
}

inline void validate_rect(const Rect& region, std::size_t height, std::size_t width) {
    if (region.row < 0 || region.column < 0) {
        throw std::invalid_argument("image region origin must be nonnegative");
    }
    const auto row = static_cast<std::size_t>(region.row);
    const auto column = static_cast<std::size_t>(region.column);
    const auto row_end = checked_add(row, region.height);
    const auto column_end = checked_add(column, region.width);
    if (row_end > height || column_end > width) {
        throw std::invalid_argument("image region is outside its source");
    }
}

inline std::ptrdiff_t modulo(std::ptrdiff_t value, std::ptrdiff_t divisor) {
    const auto result = value % divisor;
    return result < 0 ? result + divisor : result;
}

inline std::size_t border_index(std::ptrdiff_t value, std::size_t length, BorderMode mode) {
    assert(length != 0);
    const auto n = checked_ptrdiff(length);
    switch (mode) {
    case BorderMode::clamp:
        return static_cast<std::size_t>(std::clamp(value, std::ptrdiff_t{0}, n - 1));
    case BorderMode::wrap:
        return static_cast<std::size_t>(modulo(value, n));
    case BorderMode::reflect: {
        const auto index = modulo(value, checked_mul(n, std::size_t{2}));
        return static_cast<std::size_t>(index < n ? index : 2 * n - 1 - index);
    }
    case BorderMode::reflect101: {
        if (length == 1) {
            return 0;
        }
        const auto period = 2 * n - 2;
        const auto index = modulo(value, period);
        return static_cast<std::size_t>(index < n ? index : period - index);
    }
    case BorderMode::constant:
        break;
    }
    return 0;
}

template<class T>
std::vector<std::remove_const_t<T>> materialize(ImageView<T> source);

} // namespace detail

template<class T>
    requires detail::supported_scalar_v<T>
class ImageView {
public:
    using value_type = std::remove_const_t<T>;

    ImageView() noexcept = default;

    ImageView(T* data, std::size_t height, std::size_t width, std::size_t channels = 1,
              std::ptrdiff_t row_stride = 0, std::ptrdiff_t pixel_stride = 0)
        : data_(data), height_(height), width_(width), channels_(channels) {
        if (channels == 0) {
            throw std::invalid_argument("image view channels must be nonzero");
        }
        (void)detail::checked_count(height, width, channels);
        const auto resolved_pixel = pixel_stride == 0 ? detail::checked_ptrdiff(channels) : pixel_stride;
        const auto resolved_row = row_stride == 0
            ? detail::checked_mul(resolved_pixel, width)
            : row_stride;
        if (height != 0 && width != 0) {
            if (data == nullptr || resolved_pixel <= 0 || resolved_row <= 0 ||
                resolved_pixel < detail::checked_ptrdiff(channels)) {
                throw std::invalid_argument("invalid image view storage or strides");
            }
            const auto minimum_row = detail::checked_add(
                detail::checked_mul(resolved_pixel, width - 1), detail::checked_ptrdiff(channels));
            if (resolved_row < minimum_row) {
                throw std::invalid_argument("image view rows overlap");
            }
            (void)detail::checked_add(
                detail::checked_add(detail::checked_mul(resolved_row, height - 1),
                                    detail::checked_mul(resolved_pixel, width - 1)),
                detail::checked_ptrdiff(channels));
        }
        row_stride_ = resolved_row;
        pixel_stride_ = resolved_pixel;
    }

    template<class U>
        requires(std::same_as<std::remove_const_t<U>, value_type> &&
                 std::is_const_v<T> && !std::is_const_v<U>)
    ImageView(ImageView<U> other) noexcept
        : data_(other.data()), height_(other.height()), width_(other.width()),
          channels_(other.channels()), row_stride_(other.row_stride()), pixel_stride_(other.pixel_stride()) {}

    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t channels() const noexcept { return channels_; }
    [[nodiscard]] std::size_t size() const noexcept { return height_ * width_ * channels_; }
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    [[nodiscard]] bool same_size(ImageView<const value_type> other) const noexcept {
        return height_ == other.height() && width_ == other.width() && channels_ == other.channels();
    }
    [[nodiscard]] T* data() const noexcept { return data_; }
    [[nodiscard]] std::ptrdiff_t row_stride() const noexcept { return row_stride_; }
    [[nodiscard]] std::ptrdiff_t pixel_stride() const noexcept { return pixel_stride_; }
    [[nodiscard]] bool is_contiguous() const noexcept {
        return pixel_stride_ == static_cast<std::ptrdiff_t>(channels_) &&
               row_stride_ == static_cast<std::ptrdiff_t>(width_ * channels_);
    }

    [[nodiscard]] T& operator()(std::size_t row, std::size_t column, std::size_t channel) const noexcept {
        assert(row < height_ && column < width_ && channel < channels_);
        return data_[static_cast<std::ptrdiff_t>(row) * row_stride_ +
                     static_cast<std::ptrdiff_t>(column) * pixel_stride_ + static_cast<std::ptrdiff_t>(channel)];
    }
    [[nodiscard]] T& operator()(std::size_t row, std::size_t column) const noexcept {
        assert(channels_ == 1 && row < height_ && column < width_);
        return (*this)(row, column, 0);
    }
    [[nodiscard]] T& at(std::size_t row, std::size_t column, std::size_t channel) const {
        if (row >= height_ || column >= width_ || channel >= channels_) {
            throw std::out_of_range("image view index is out of range");
        }
        return (*this)(row, column, channel);
    }
    [[nodiscard]] T& at(std::size_t row, std::size_t column) const {
        if (channels_ != 1) {
            throw std::logic_error("two-index access requires one channel");
        }
        return at(row, column, 0);
    }
    [[nodiscard]] std::span<T> pixel(std::size_t row, std::size_t column) const noexcept {
        assert(row < height_ && column < width_);
        return {data_ + static_cast<std::ptrdiff_t>(row) * row_stride_ +
                    static_cast<std::ptrdiff_t>(column) * pixel_stride_, channels_};
    }
    [[nodiscard]] ImageView row(std::size_t row_index) const noexcept {
        assert(row_index < height_);
        return unchecked(data_ + static_cast<std::ptrdiff_t>(row_index) * row_stride_, 1, width_, channels_,
                         row_stride_, pixel_stride_);
    }
    [[nodiscard]] ImageView column(std::size_t column_index) const noexcept {
        assert(column_index < width_);
        return unchecked(data_ + static_cast<std::ptrdiff_t>(column_index) * pixel_stride_, height_, 1, channels_,
                         row_stride_, pixel_stride_);
    }
    [[nodiscard]] ImageView view(Rect region) const {
        detail::validate_rect(region, height_, width_);
        auto* region_data = data_;
        if (region.height != 0 && region.width != 0) {
            region_data += region.row * row_stride_ + region.column * pixel_stride_;
        }
        return unchecked(region_data, region.height,
                         region.width, channels_, row_stride_, pixel_stride_);
    }
    [[nodiscard]] ImageView strided_view(std::size_t row_step, std::size_t column_step,
                                         std::optional<Rect> region = std::nullopt) const {
        if (row_step == 0 || column_step == 0) {
            throw std::invalid_argument("image view steps must be nonzero");
        }
        const Rect effective = region.value_or(Rect{0, 0, height_, width_});
        detail::validate_rect(effective, height_, width_);
        const auto output_height = detail::ceil_div(effective.height, row_step);
        const auto output_width = detail::ceil_div(effective.width, column_step);
        const auto output_row_stride = detail::checked_mul(row_stride_, row_step);
        const auto output_pixel_stride = detail::checked_mul(pixel_stride_, column_step);
        auto* region_data = data_;
        if (output_height != 0 && output_width != 0) {
            region_data += effective.row * row_stride_ + effective.column * pixel_stride_;
        }
        return unchecked(region_data, output_height,
                         output_width, channels_, output_row_stride, output_pixel_stride);
    }

    void copy_from(ImageView<const value_type> source) const
        requires(!std::is_const_v<T>) {
        if (!same_size(source)) {
            throw std::invalid_argument("image view copy sizes differ");
        }
        const auto values = detail::materialize(source);
        std::size_t index = 0;
        for (std::size_t row_index = 0; row_index != height_; ++row_index) {
            for (std::size_t column_index = 0; column_index != width_; ++column_index) {
                for (std::size_t channel = 0; channel != channels_; ++channel) {
                    (*this)(row_index, column_index, channel) = values[index++];
                }
            }
        }
    }
    void copy_to(ImageView<value_type> destination) const {
        destination.copy_from(ImageView<const value_type>(*this));
    }
    void fill(const value_type& value) const
        requires(!std::is_const_v<T>) {
        for (std::size_t row_index = 0; row_index != height_; ++row_index) {
            for (std::size_t column_index = 0; column_index != width_; ++column_index) {
                for (std::size_t channel = 0; channel != channels_; ++channel) {
                    (*this)(row_index, column_index, channel) = value;
                }
            }
        }
    }

private:
    template<class U>
        requires(detail::supported_scalar_v<U> && !std::is_const_v<U>)
    friend class Image;
    template<class U>
    friend std::vector<std::remove_const_t<U>> detail::materialize(ImageView<U> source);

    static ImageView unchecked(T* data, std::size_t height, std::size_t width, std::size_t channels,
                               std::ptrdiff_t row_stride, std::ptrdiff_t pixel_stride) noexcept {
        ImageView result;
        result.data_ = data;
        result.height_ = height;
        result.width_ = width;
        result.channels_ = channels;
        result.row_stride_ = row_stride;
        result.pixel_stride_ = pixel_stride;
        return result;
    }

    T* data_ = nullptr;
    std::size_t height_ = 0;
    std::size_t width_ = 0;
    std::size_t channels_ = 0;
    std::ptrdiff_t row_stride_ = 0;
    std::ptrdiff_t pixel_stride_ = 0;
};

namespace detail {

template<class T>
std::vector<std::remove_const_t<T>> materialize(ImageView<T> source) {
    using value_type = std::remove_const_t<T>;
    std::vector<value_type> values;
    values.reserve(source.size());
    for (std::size_t row = 0; row != source.height(); ++row) {
        for (std::size_t column = 0; column != source.width(); ++column) {
            for (std::size_t channel = 0; channel != source.channels(); ++channel) {
                values.push_back(source(row, column, channel));
            }
        }
    }
    return values;
}

} // namespace detail

template<class T>
    requires(detail::supported_scalar_v<T> && !std::is_const_v<T>)
class Image {
public:
    using value_type = T;

    Image() noexcept = default;
    Image(std::size_t height, std::size_t width, std::size_t channels = 1)
        : Image(height, width, channels, T{}) {}
    Image(std::size_t height, std::size_t width, std::size_t channels, const T& value)
        : height_(height), width_(width), channels_(channels), values_(storage_count(height, width, channels), value) {}
    Image(std::span<const T> values, std::size_t height, std::size_t width, std::size_t channels = 1)
        : height_(height), width_(width), channels_(channels) {
        const auto count = storage_count(height, width, channels);
        if (values.size() != count) {
            throw std::invalid_argument("image span size does not match shape");
        }
        values_.assign(values.begin(), values.end());
    }
    Image(std::vector<T>&& values, std::size_t height, std::size_t width, std::size_t channels = 1)
        : height_(height), width_(width), channels_(channels) {
        const auto count = storage_count(height, width, channels);
        if (values.size() != count) {
            throw std::invalid_argument("image vector size does not match shape");
        }
        values_ = std::move(values);
    }
    Image(const Image&) = default;
    Image(Image&& other) noexcept
        : height_(other.height_), width_(other.width_), channels_(other.channels_), values_(std::move(other.values_)) {
        other.clear();
    }
    Image(ImageView<T> source) { copy_from(ImageView<const T>(source)); }
    Image(ImageView<const T> source) { copy_from(source); }

    Image& operator=(const Image&) = default;
    Image& operator=(Image&& other) noexcept {
        if (this != &other) {
            height_ = other.height_;
            width_ = other.width_;
            channels_ = other.channels_;
            values_ = std::move(other.values_);
            other.clear();
        }
        return *this;
    }
    Image& operator=(ImageView<const T> source) {
        copy_from(source);
        return *this;
    }

    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t channels() const noexcept { return channels_; }
    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }
    [[nodiscard]] bool empty() const noexcept { return values_.empty(); }
    [[nodiscard]] bool same_size(ImageView<const T> other) const noexcept {
        return height_ == other.height() && width_ == other.width() && channels_ == other.channels();
    }
    [[nodiscard]] T* data() noexcept { return values_.empty() ? nullptr : values_.data(); }
    [[nodiscard]] const T* data() const noexcept { return values_.empty() ? nullptr : values_.data(); }
    [[nodiscard]] std::span<T> scalars() noexcept { return {data(), values_.size()}; }
    [[nodiscard]] std::span<const T> scalars() const noexcept { return {data(), values_.size()}; }
    [[nodiscard]] auto begin() noexcept { return values_.begin(); }
    [[nodiscard]] auto end() noexcept { return values_.end(); }
    [[nodiscard]] auto begin() const noexcept { return values_.begin(); }
    [[nodiscard]] auto end() const noexcept { return values_.end(); }

    [[nodiscard]] T& operator()(std::size_t row, std::size_t column, std::size_t channel) noexcept {
        assert(row < height_ && column < width_ && channel < channels_);
        return values_[offset(row, column, channel)];
    }
    [[nodiscard]] const T& operator()(std::size_t row, std::size_t column, std::size_t channel) const noexcept {
        assert(row < height_ && column < width_ && channel < channels_);
        return values_[offset(row, column, channel)];
    }
    [[nodiscard]] T& operator()(std::size_t row, std::size_t column) noexcept {
        assert(channels_ == 1 && row < height_ && column < width_);
        return (*this)(row, column, 0);
    }
    [[nodiscard]] const T& operator()(std::size_t row, std::size_t column) const noexcept {
        assert(channels_ == 1 && row < height_ && column < width_);
        return (*this)(row, column, 0);
    }
    [[nodiscard]] T& at(std::size_t row, std::size_t column, std::size_t channel) {
        validate_index(row, column, channel);
        return (*this)(row, column, channel);
    }
    [[nodiscard]] const T& at(std::size_t row, std::size_t column, std::size_t channel) const {
        validate_index(row, column, channel);
        return (*this)(row, column, channel);
    }
    [[nodiscard]] T& at(std::size_t row, std::size_t column) {
        validate_two_index(row, column);
        return (*this)(row, column);
    }
    [[nodiscard]] const T& at(std::size_t row, std::size_t column) const {
        validate_two_index(row, column);
        return (*this)(row, column);
    }
    [[nodiscard]] std::span<T> pixel(std::size_t row, std::size_t column) noexcept {
        assert(row < height_ && column < width_);
        return {data() + offset(row, column, 0), channels_};
    }
    [[nodiscard]] std::span<const T> pixel(std::size_t row, std::size_t column) const noexcept {
        assert(row < height_ && column < width_);
        return {data() + offset(row, column, 0), channels_};
    }

    [[nodiscard]] ImageView<T> view(Rect region) { return as_view().view(region); }
    [[nodiscard]] ImageView<const T> view(Rect region) const { return as_view().view(region); }
    [[nodiscard]] ImageView<T> row(std::size_t row_index) noexcept { return as_view().row(row_index); }
    [[nodiscard]] ImageView<const T> row(std::size_t row_index) const noexcept { return as_view().row(row_index); }
    [[nodiscard]] ImageView<T> column(std::size_t column_index) noexcept { return as_view().column(column_index); }
    [[nodiscard]] ImageView<const T> column(std::size_t column_index) const noexcept { return as_view().column(column_index); }
    [[nodiscard]] ImageView<T> strided_view(std::size_t row_step, std::size_t column_step,
                                             std::optional<Rect> region = std::nullopt) {
        return as_view().strided_view(row_step, column_step, region);
    }
    [[nodiscard]] ImageView<const T> strided_view(std::size_t row_step, std::size_t column_step,
                                                   std::optional<Rect> region = std::nullopt) const {
        return as_view().strided_view(row_step, column_step, region);
    }

    [[nodiscard]] Image crop(Rect region, BorderMode border = BorderMode::constant,
                             std::span<const T> outside = {}) const {
        return detail::crop_impl(as_view(), region, border, outside);
    }
    void copy_from(ImageView<const T> source) {
        const auto copied = detail::materialize(source);
        Image replacement(std::move(copied), source.height(), source.width(), source.channels());
        swap(replacement);
    }
    void copy_to(ImageView<T> destination) const { as_view().copy_to(destination); }
    void resize(std::size_t height, std::size_t width, std::size_t channels = 1, const T& value = T{}) {
        Image replacement(height, width, channels, value);
        swap(replacement);
    }
    void fill(const T& value) { std::fill(values_.begin(), values_.end(), value); }
    void clear() noexcept {
        values_.clear();
        height_ = 0;
        width_ = 0;
        channels_ = 0;
    }
    void swap(Image& other) noexcept {
        using std::swap;
        swap(height_, other.height_);
        swap(width_, other.width_);
        swap(channels_, other.channels_);
        values_.swap(other.values_);
    }

    operator ImageView<T>() noexcept { return as_view(); }
    operator ImageView<const T>() const noexcept { return as_view(); }

private:
    static std::size_t storage_count(std::size_t height, std::size_t width, std::size_t channels) {
        const auto count = detail::checked_count(height, width, channels);
        (void)detail::checked_ptrdiff(detail::checked_mul(width, channels));
        return count;
    }
    [[nodiscard]] std::size_t offset(std::size_t row, std::size_t column, std::size_t channel) const noexcept {
        return ((row * width_) + column) * channels_ + channel;
    }
    void validate_index(std::size_t row, std::size_t column, std::size_t channel) const {
        if (row >= height_ || column >= width_ || channel >= channels_) {
            throw std::out_of_range("image index is out of range");
        }
    }
    void validate_two_index(std::size_t row, std::size_t column) const {
        if (channels_ != 1) {
            throw std::logic_error("two-index access requires one channel");
        }
        validate_index(row, column, 0);
    }
    [[nodiscard]] ImageView<T> as_view() noexcept {
        return ImageView<T>::unchecked(data(), height_, width_, channels_,
                                       static_cast<std::ptrdiff_t>(width_ * channels_), static_cast<std::ptrdiff_t>(channels_));
    }
    [[nodiscard]] ImageView<const T> as_view() const noexcept {
        return ImageView<const T>::unchecked(data(), height_, width_, channels_,
                                             static_cast<std::ptrdiff_t>(width_ * channels_), static_cast<std::ptrdiff_t>(channels_));
    }

    std::size_t height_ = 0;
    std::size_t width_ = 0;
    std::size_t channels_ = 0;
    std::vector<T> values_;
};

namespace detail {

template<class T>
Image<std::remove_const_t<T>> crop_impl(ImageView<T> source, Rect region, BorderMode border,
                                        std::span<const std::remove_const_t<T>> outside) {
    using value_type = std::remove_const_t<T>;
    if (region.height == 0 || region.width == 0) {
        return Image<value_type>(region.height, region.width, source.channels());
    }
    if (border == BorderMode::constant && outside.size() != 0 && outside.size() != 1 && outside.size() != source.channels()) {
        throw std::invalid_argument("constant border size does not match channels");
    }
    if (border != BorderMode::constant && (source.height() == 0 || source.width() == 0)) {
        throw std::invalid_argument("nonconstant border requires a nonempty source");
    }
    Image<value_type> result(region.height, region.width, source.channels());
    for (std::size_t row = 0; row != region.height; ++row) {
        const auto source_row = detail::checked_add(region.row, detail::checked_ptrdiff(row));
        for (std::size_t column = 0; column != region.width; ++column) {
            const auto source_column = detail::checked_add(region.column, detail::checked_ptrdiff(column));
            const bool inside = source_row >= 0 && source_column >= 0 &&
                static_cast<std::size_t>(source_row) < source.height() &&
                static_cast<std::size_t>(source_column) < source.width();
            for (std::size_t channel = 0; channel != source.channels(); ++channel) {
                if (inside) {
                    result(row, column, channel) = source(static_cast<std::size_t>(source_row),
                                                          static_cast<std::size_t>(source_column), channel);
                } else if (border == BorderMode::constant) {
                    result(row, column, channel) = outside.empty() ? value_type{} : outside[outside.size() == 1 ? 0 : channel];
                } else {
                    result(row, column, channel) = source(detail::border_index(source_row, source.height(), border),
                                                          detail::border_index(source_column, source.width(), border), channel);
                }
            }
        }
    }
    return result;
}

} // namespace detail

template<class T>
Image<std::remove_const_t<T>> crop(ImageView<T> source, Rect region, BorderMode border = BorderMode::constant,
                                   std::span<const std::type_identity_t<std::remove_const_t<T>>> outside = {}) {
    return detail::crop_impl(source, region, border, outside);
}

template<class T>
Image<T> crop(const Image<T>& source, Rect region, BorderMode border = BorderMode::constant,
              std::span<const std::type_identity_t<T>> outside = {}) {
    return detail::crop_impl(static_cast<ImageView<const T>>(source), region, border, outside);
}

template<class Destination, class Source>
void blit(ImageView<Destination> destination, ImageView<Source> source,
          std::ptrdiff_t destination_row, std::ptrdiff_t destination_column)
    requires(!std::is_const_v<Destination> &&
             std::same_as<std::remove_const_t<Destination>, std::remove_const_t<Source>>) {
    if (destination.channels() != source.channels()) {
        throw std::invalid_argument("blit channel counts differ");
    }
    const auto values = detail::materialize(source);
    for (std::size_t row = 0; row != source.height(); ++row) {
        const auto target_row = detail::checked_add(destination_row, detail::checked_ptrdiff(row));
        if (target_row < 0 || static_cast<std::size_t>(target_row) >= destination.height()) {
            continue;
        }
        for (std::size_t column = 0; column != source.width(); ++column) {
            const auto target_column = detail::checked_add(destination_column, detail::checked_ptrdiff(column));
            if (target_column < 0 || static_cast<std::size_t>(target_column) >= destination.width()) {
                continue;
            }
            for (std::size_t channel = 0; channel != source.channels(); ++channel) {
                const auto index = ((row * source.width()) + column) * source.channels() + channel;
                destination(static_cast<std::size_t>(target_row), static_cast<std::size_t>(target_column), channel) = values[index];
            }
        }
    }
}

template<class Destination, class Source>
void blit(Image<Destination>& destination, ImageView<Source> source,
          std::ptrdiff_t row, std::ptrdiff_t column)
    requires std::same_as<Destination, std::remove_const_t<Source>> {
    blit(static_cast<ImageView<Destination>>(destination), source, row, column);
}

template<class Destination, class Source>
void blit(ImageView<Destination> destination, const Image<Source>& source,
          std::ptrdiff_t row, std::ptrdiff_t column)
    requires(!std::is_const_v<Destination> && std::same_as<std::remove_const_t<Destination>, Source>) {
    blit(destination, static_cast<ImageView<const Source>>(source), row, column);
}

template<class Destination, class Source>
void blit(Image<Destination>& destination, const Image<Source>& source,
          std::ptrdiff_t row, std::ptrdiff_t column)
    requires std::same_as<Destination, Source> {
    blit(static_cast<ImageView<Destination>>(destination), static_cast<ImageView<const Source>>(source), row, column);
}

namespace detail {

template<class T>
inline constexpr bool unsigned_integer_v = std::is_integral_v<unqualified_t<T>> && std::is_unsigned_v<unqualified_t<T>>;

template<class T>
inline constexpr bool signed_integer_v = std::is_integral_v<unqualified_t<T>> && std::is_signed_v<unqualified_t<T>>;

template<class To>
To floating_result(long double value) {
#if IMAGELIB_USE_HALF
    if constexpr (std::same_as<unqualified_t<To>, half>) {
        return half(static_cast<float>(value));
    } else {
        return static_cast<To>(value);
    }
#else
    return static_cast<To>(value);
#endif
}

template<class To>
To saturated_integer(long double value) {
    if (std::isnan(value)) {
        return To{};
    }
    const auto minimum = static_cast<long double>(std::numeric_limits<To>::lowest());
    const auto maximum = static_cast<long double>(std::numeric_limits<To>::max());
    value = std::clamp(std::trunc(value), minimum, maximum);
    return static_cast<To>(value);
}

template<class To, class From>
To convert_scalar(From source, ConversionPolicy policy) {
    static_assert(supported_scalar_v<To> && supported_scalar_v<From>);
    using destination_type = unqualified_t<To>;
    using source_type = unqualified_t<From>;
    const long double input = to_long_double(source);

    if (policy == ConversionPolicy::numeric) {
        if constexpr (std::is_integral_v<destination_type>) {
            return saturated_integer<destination_type>(input);
        } else {
            return floating_result<destination_type>(input);
        }
    }

    const bool unorm = policy == ConversionPolicy::normalized_unorm;
    const bool valid = unorm
        ? ((unsigned_integer_v<source_type> && floating_scalar_v<destination_type>) ||
           (floating_scalar_v<source_type> && unsigned_integer_v<destination_type>))
        : ((signed_integer_v<source_type> && floating_scalar_v<destination_type>) ||
           (floating_scalar_v<source_type> && signed_integer_v<destination_type>));
    if (!valid) {
        throw std::invalid_argument("invalid conversion policy and scalar types");
    }

    if constexpr (std::is_integral_v<source_type> && floating_scalar_v<destination_type>) {
        if (unorm) {
            return floating_result<destination_type>(input / static_cast<long double>(std::numeric_limits<source_type>::max()));
        }
        const auto normalized = std::clamp(input / static_cast<long double>(std::numeric_limits<source_type>::max()), -1.0L, 1.0L);
        return floating_result<destination_type>(normalized);
    } else if constexpr (floating_scalar_v<source_type> && std::is_integral_v<destination_type>) {
        const long double lower = unorm ? 0.0L : -1.0L;
        const auto clamped = std::isnan(input) ? 0.0L : std::clamp(input, lower, 1.0L);
        const auto scale = static_cast<long double>(std::numeric_limits<destination_type>::max());
        return saturated_integer<destination_type>(std::round(clamped * scale));
    } else {
        throw std::invalid_argument("invalid conversion policy and scalar types");
    }
}

template<class To, class From>
Image<To> convert_impl(ImageView<From> source, ConversionPolicy policy) {
    Image<To> result(source.height(), source.width(), source.channels());
    for (std::size_t row = 0; row != source.height(); ++row) {
        for (std::size_t column = 0; column != source.width(); ++column) {
            for (std::size_t channel = 0; channel != source.channels(); ++channel) {
                result(row, column, channel) = convert_scalar<To>(source(row, column, channel), policy);
            }
        }
    }
    return result;
}

} // namespace detail

template<class To, class From>
    requires(detail::supported_scalar_v<To> && !std::is_const_v<To>)
Image<To> convert(ImageView<From> source, ConversionPolicy policy = ConversionPolicy::numeric) {
    return detail::convert_impl<To>(source, policy);
}

template<class To, class From>
    requires(detail::supported_scalar_v<To> && !std::is_const_v<To>)
Image<To> convert(const Image<From>& source, ConversionPolicy policy = ConversionPolicy::numeric) {
    return detail::convert_impl<To>(static_cast<ImageView<const From>>(source), policy);
}

} // namespace image