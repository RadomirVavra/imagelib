# image::Image::view

```cpp
ImageView<T> view(Rect region);
ImageView<const T> view(Rect region) const;
ImageView<T> row(size_t row) noexcept;
ImageView<const T> row(size_t row) const noexcept;
ImageView<T> column(size_t column) noexcept;
ImageView<const T> column(size_t column) const noexcept;
```

Returns a zero-copy view of an in-bounds region. `region.row` and `region.column` must be non-negative, and its half-open extent `[row, row + height) x [column, column + width)` must lie inside the image. A zero-size region is valid, including one positioned at the corresponding boundary. Invalid regions throw `std::invalid_argument`; overflow in extent calculations throws `std::length_error`. A view aliases the image and is invalidated when the image is resized, cleared, moved from, or destroyed. See [Regions](Regions.md) for `Rect`.

```cpp
image::Image<float> image(480, 640, 3, 0.0f);

image::Rect upper_left{ 0, 0, 128, 128 };
auto first_tile = image.view(upper_left);

auto centered = image.view({ 176, 256, 128, 128 });
auto bottom_band = image.view({ image.height() - 32, 0, 32, image.width() });

first_tile(0, 0, 0) = 1.0f;
```

## row, column

`row` returns a zero-copy one-row view with the same width and channel count. It is equivalent to `view({ row, 0, 1, width() })`.

`column` returns a zero-copy one-column view with the same height and channel count. It is equivalent to `strided_view(1, 1, image::Rect{ 0, column, height(), 1 })`. A valid row or column index is a precondition; debug builds assert it and violating it has undefined behavior in a release build.

```cpp
void consume_image(image::ImageView<float>);

image::Image<float> rgb(480, 640, 3);
consume_image(rgb.row(0));
consume_image(rgb.column(0));
```

Use `scalars()` on an owning `Image` for a contiguous scalar range. The line views have consistent semantics for both `Image` and strided `ImageView` objects.
