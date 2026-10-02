# image::ImageView element access

```cpp
T& operator()(size_t row, size_t column, size_t channel) const noexcept;
T& operator()(size_t row, size_t column) const noexcept;
T& at(size_t row, size_t column, size_t channel) const;
T& at(size_t row, size_t column) const;
```

Accesses one scalar through the view strides. Two-index access requires one channel. `operator()` is unchecked: valid indices and the single-channel requirement are preconditions; debug builds assert them and violating either precondition has undefined behavior in a release build. `at` throws `std::out_of_range` for invalid indices and `std::logic_error` when two-index access is used on a multi-channel view.

```cpp
view(10, 20) = 1.0f;
float component = rgb_view.at(10, 20, 2);
```
