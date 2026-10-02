# image::ImageView::strided_view

```cpp
ImageView<T> strided_view(size_t row_step, size_t column_step,
                          std::optional<Rect> region = std::nullopt) const;
```

Returns an in-bounds zero-copy subview that samples every `row_step`-th row and every `column_step`-th column. Both steps must be greater than zero. The effective region is the whole source view for `std::nullopt`, otherwise `*region`; an explicit region must satisfy the same half-open in-bounds rules as `view`. The output dimensions are `ceil(effective_region.height / row_step)` by `ceil(effective_region.width / column_step)`; each output `(r, c)` aliases source `(effective_region.row + r * row_step, effective_region.column + c * column_step)`. `Rect{}` is therefore a valid request for an empty view. Invalid arguments throw `std::invalid_argument`; overflow in sampling, extent, or stride calculations throws `std::length_error`.

```cpp
image::Image<float> image(100, 100);
auto all = image.view({ 0, 0, 100, 100 });

auto even_rows = all.strided_view(2, 1);          // 50 by 100
auto every_fourth_column = all.strided_view(1, 4); // 100 by 25
auto decimated = all.strided_view(2, 2, image::Rect{ 25, 25, 50, 50 });
// decimated is 25 by 25
```

The returned view has `row_stride() == source.row_stride() * row_step` and `pixel_stride() == source.pixel_stride() * column_step`. It is usually non-contiguous when either step is greater than one.
