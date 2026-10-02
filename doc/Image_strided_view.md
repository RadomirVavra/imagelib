# image::Image::strided_view

```cpp
ImageView<T> strided_view(size_t row_step, size_t column_step,
                  std::optional<Rect> region = std::nullopt);
ImageView<const T> strided_view(size_t row_step, size_t column_step,
                      std::optional<Rect> region = std::nullopt) const;
```

Returns an in-bounds zero-copy view sampled at positive row and column steps. `row_step` and `column_step` must be greater than zero. The effective region is the whole image for `std::nullopt`, otherwise `*region`; an explicit region must satisfy the same half-open in-bounds rules as `view`. The returned view has height `ceil(effective_region.height / row_step)` and width `ceil(effective_region.width / column_step)`; each output `(r, c)` aliases source `(effective_region.row + r * row_step, effective_region.column + c * column_step)`. `Rect{}` is therefore a valid request for an empty view. Invalid arguments throw `std::invalid_argument`; overflow in sampling, extent, or stride calculations throws `std::length_error`. See [ImageView::strided_view](ImageView_strided_view.md) for stride results and further examples.

```cpp
image::Image<float> image(100, 100, 3);
auto even_rows = image.strided_view(2, 1);          // 50 rows, 100 columns
auto every_fourth_column = image.strided_view(1, 4); // 100 rows, 25 columns
auto decimated_roi = image.strided_view(2, 2, image::Rect{ 10, 20, 50, 50 });
// decimated_roi is 25 by 25
```
