# image::Image::data

```cpp
T* data() noexcept;
const T* data() const noexcept;
```

Returns a pointer to the first scalar in contiguous HWC order, or a null pointer for an empty image. The pointer is invalidated by operations that replace storage.

```cpp
write_scalars(image.data(), image.size());
```
