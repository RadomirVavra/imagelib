# image::ImageView::row, column

```cpp
ImageView<T> row(size_t row) const noexcept;
ImageView<T> column(size_t column) const noexcept;
```

`row` returns a one-row view with the same `pixel_stride()` as its source. It does not return `std::span`, because a view with `pixel_stride() != channels()` has gaps between its pixels. Access a component as `one_row(0, column, channel)` or `one_row(0, column)`, and construct `Image<T> copy = one_row` when contiguous storage is needed.

`column` returns a one-column view with the same height and channel count. It preserves the source `row_stride()`, so it also works for strided views. A valid row or column index is a precondition; debug builds assert it and violating it has undefined behavior in a release build.

```cpp
image::Image<float> image(4, 4);
auto view = image.view({ 0, 0, 4, 4 });

auto first_row = view.row(0);
float value = first_row(0, 3);

auto first_column = view.column(0);
```
