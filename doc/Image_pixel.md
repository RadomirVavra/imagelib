# image::Image::pixel

```cpp
std::span<T> pixel(size_t row, size_t column) noexcept;
std::span<const T> pixel(size_t row, size_t column) const noexcept;
```

Returns a span over all adjacent components of one pixel. Valid row and column indices are preconditions; debug builds assert them and violating either precondition has undefined behavior in a release build. It is preferable to repeated `operator()(row, column, channel)` calls when an operation consumes or writes every channel, because the channel count is carried by the range rather than duplicated in loop bounds.

```cpp
image::Image<float> rgb(480, 640, 3, 1.0f);
const float exposure = 0.5f;
for (float& component : rgb.pixel(10, 20)) {
    component *= exposure;
}
```

Each component may use a distinct multiplier with `std::transform`:

```cpp
const std::array<float, 3> gains = { 1.1f, 0.9f, 1.0f };
auto values = rgb.pixel(10, 20);
std::transform(values.begin(), values.end(), gains.begin(), values.begin(),
               [](float value, float gain) { return value * gain; });
```

The library does not define arithmetic operators for a pixel span: images have a runtime channel count, so component-wise arithmetic is expressed through standard range algorithms.
