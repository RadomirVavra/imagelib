# imagelib

`imagelib` is a C++20 header-only HWC image container and view library. It provides contiguous owned `image::Image<T>` storage, zero-copy `image::ImageView<T>` regions and stepped views, border-aware crop, clipped blit, and explicit scalar conversion policies.

Supported scalar types are fixed-width signed and unsigned integers, `float`, `double`, and `image::half`.

`image.hpp` enables `image::half` by default. Define `IMAGELIB_USE_HALF=0` before including it to omit the optional Christian Rau dependency and support all remaining scalar types.

## CMake

```cmake
find_package(imagelib CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE imagelib::imagelib)
```

## Example

```cpp
#include <imagelib/image.hpp>

image::Image<float> rgb(480, 640, 3, 0.0f);
auto roi = rgb.view({ 100, 100, 128, 128 });
image::Image<float> copy = roi;
```

Build tests into `build/` with `cmake -S . -B build -DIMAGELIB_BUILD_TESTING=ON`, then run `cmake --build build` and `ctest --test-dir build --output-on-failure`.

## License

Licensed under the [Apache License, Version 2.0](LICENSE).

## Third-party notices

`image::half` aliases Christian Rau's [half 2.2.1](include/imagelib/third_party/half.hpp), distributed under the MIT License. See [the included notice](include/imagelib/third_party/half-MIT.txt).