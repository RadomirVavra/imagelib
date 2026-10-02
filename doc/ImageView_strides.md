# image::ImageView strides

```cpp
ptrdiff_t row_stride() const noexcept;
ptrdiff_t pixel_stride() const noexcept;
```

Return the scalar-element increments from one row to the next and from one pixel to the next pixel in a row.

`pixel_stride()` is at least `channels()`, so every pixel's channels remain adjacent. A view is contiguous exactly when `pixel_stride() == channels()` and `row_stride() == width() * channels()`.

```cpp
image::Image<float> image(4, 4, 3);
auto view = image.view({ 0, 0, 4, 4 });
auto next_row = view.data() + view.row_stride();
```
