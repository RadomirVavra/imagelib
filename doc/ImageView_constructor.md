# image::ImageView::ImageView

```cpp
ImageView() noexcept;
ImageView(T* data, size_t height, size_t width, size_t channels = 1,
          ptrdiff_t row_stride = 0, ptrdiff_t pixel_stride = 0);

template<class U>
ImageView(ImageView<U> other) noexcept
    requires (std::same_as<std::remove_const_t<U>, value_type> &&
              std::is_const_v<T> && !std::is_const_v<U>);
```

The default constructor creates an empty view with shape `(0, 0, 0)` and a null data pointer. The converting constructor creates a read-only view from a mutable view without changing its pointer, dimensions, or strides.

The external-storage constructor creates a non-owning view. Strides are measured in scalar elements. A zero `pixel_stride` means `channels`; a zero `row_stride` means `width * pixel_stride`. The caller keeps `data` alive.

`channels` must be greater than zero. For a non-empty view, `data` must be non-null; resolved strides must be positive, `pixel_stride >= channels`, and `row_stride >= (width - 1) * pixel_stride + channels`. These conditions keep the components of each pixel adjacent and prevent rows from overlapping. The caller must ensure that `data` addresses at least `(height - 1) * row_stride + (width - 1) * pixel_stride + channels` accessible scalar elements for a non-empty view. Size and stride products are checked. Empty views may have a null data pointer and do not dereference it. Invalid arguments throw `std::invalid_argument`; overflow throws `std::length_error`.

```cpp
std::vector<float> values(512 * 512);
image::ImageView<float> view(values.data(), 512, 512);
```
