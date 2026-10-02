# image::ImageView::data

```cpp
T* data() const noexcept;
```

Returns the stored pointer to the first scalar. A default view returns null; an externally constructed empty view returns its supplied pointer, which may be null. For a non-contiguous view this pointer alone does not describe all elements; use strides or copy into an `Image`.

```cpp
const float first = view.data()[0];
```
