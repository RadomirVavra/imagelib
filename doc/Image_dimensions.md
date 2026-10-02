# image::Image dimensions

```cpp
size_t height() const noexcept;
size_t width() const noexcept;
size_t channels() const noexcept;
```

Return the row count, column count, and scalar component count per pixel.

```cpp
image::Image<float> image(480, 640, 3);
assert(image.height() == 480);
assert(image.width() == 640);
assert(image.channels() == 3);
```
