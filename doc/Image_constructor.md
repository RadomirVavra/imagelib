# image::Image::Image

```cpp
Image();
Image(size_t height, size_t width, size_t channels = 1);
Image(size_t height, size_t width, size_t channels, const T& value);
Image(std::span<const T> values, size_t height, size_t width, size_t channels = 1);
Image(std::vector<T>&& values, size_t height, size_t width, size_t channels = 1);
Image(const Image& other);
Image(Image&& other) noexcept;
Image(ImageView<T> source);
Image(ImageView<const T> source);
```

Constructs an empty image, an owned image of the requested dimensions, or a contiguous copy of a view. `channels` defaults to one for monochrome images and must be greater than zero for every non-default constructor. Dimensions and their scalar product are checked. Span and vector inputs contain exactly `height * width * channels` values. A mismatched range throws `std::invalid_argument`; overflow throws `std::length_error`.

```cpp
image::Image<float> mask(480, 640);
image::Image<float> rgb(480, 640, 3, 0.0f);
auto roi = rgb.view({ 0, 0, 100, 100 });
image::Image<float> copy = roi;
```

The view constructor is not explicit: copy-initialization from a view creates an independent contiguous image. The copy constructor duplicates elements. The move constructor transfers owned storage; the moved-from image is empty.
