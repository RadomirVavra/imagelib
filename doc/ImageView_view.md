# image::ImageView::view

```cpp
ImageView<T> view(Rect region) const;
```

Returns a zero-copy subview of an in-bounds region. The subview retains the original strides. `region.row` and `region.column` must be non-negative, and its half-open extent `[row, row + height) x [column, column + width)` must lie inside the view. A zero-size region is valid, including one positioned at the corresponding boundary. Invalid regions throw `std::invalid_argument`; overflow in extent calculations throws `std::length_error`.

```cpp
image::Image<float> image(256, 256);
auto view = image.view({ 0, 0, 256, 256 });
auto tile = view.view({ 64, 64, 128, 128 });
```

See [strided_view](ImageView_strided_view.md) for sampling rows or columns at a step greater than one.
