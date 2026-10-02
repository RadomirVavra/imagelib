# image::Image::at

```cpp
T& at(size_t row, size_t column, size_t channel);
const T& at(size_t row, size_t column, size_t channel) const;
T& at(size_t row, size_t column);
const T& at(size_t row, size_t column) const;
```

Returns a checked component reference. Invalid indices throw `std::out_of_range`. Two-index access on a multi-channel image throws `std::logic_error`.

```cpp
float value = mask.at(10, 20);
float blue = rgb.at(10, 20, 2);
```
