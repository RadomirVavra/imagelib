# image::ImageView::is_contiguous

```cpp
bool is_contiguous() const noexcept;
```

Returns true when `pixel_stride() == channels()` and `row_stride() == width() * channels()`. Such a view has contiguous HWC scalar storage.

```cpp
image::Image<float> image(100, 100);
auto view = image.view({ 0, 0, 100, 100 });
if (!view.is_contiguous()) {
    image::Image<float> contiguous(view);
    consume(contiguous.data(), contiguous.size());
}
```

`Image<T> copy(view);` and `copy.copy_from(view);` materialize any strided view as a contiguous independent image.
