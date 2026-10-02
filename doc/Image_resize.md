# image::Image::resize

```cpp
void resize(size_t height, size_t width, size_t channels = 1,
            const T& value = T{});
```

Replaces the image with the requested dimensions and initializes every scalar to `value`. `channels` must be greater than zero. Existing pointers, references, iterators, and views are invalidated. Zero height or width is permitted; invalid channels throw `std::invalid_argument` and size overflow throws `std::length_error`.

```cpp
mask.resize(1024, 1024);
rgb.resize(1024, 1024, 3, 0.0f);
```
