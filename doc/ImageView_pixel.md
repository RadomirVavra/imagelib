# image::ImageView::pixel

```cpp
std::span<T> pixel(size_t row, size_t column) const noexcept;
```

Returns the `channels()` adjacent scalars of one pixel. It is valid because `ImageView` preserves adjacency of components within every pixel. Valid row and column indices are preconditions; debug builds assert them and violating either precondition has undefined behavior in a release build.

```cpp
image::Image<float> image(4, 4);
auto view = image.view({ 0, 0, 4, 4 });

for (float& value : view.pixel(0, 0)) {
    value = 0.0f;
}
```
