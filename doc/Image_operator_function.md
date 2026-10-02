# image::Image::operator()

```cpp
T& operator()(size_t row, size_t column, size_t channel) noexcept;
const T& operator()(size_t row, size_t column, size_t channel) const noexcept;
T& operator()(size_t row, size_t column) noexcept;
const T& operator()(size_t row, size_t column) const noexcept;
```

Returns an unchecked component reference. The two-index overload requires a single-channel image. Valid indices and the single-channel requirement are preconditions; debug builds assert them and violating either precondition has undefined behavior in a release build.

```cpp
mask(10, 20) = 1.0f;
rgb(10, 20, 2) = 0.5f;
```
