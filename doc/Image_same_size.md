# image::Image::same_size

```cpp
bool same_size(ImageView<const T> other) const noexcept;
```

Returns true when height, width, and channel count are equal. `Image<T>` and `ImageView<T>` convert directly to the read-only view parameter.

```cpp
assert(image.same_size(other));
assert(image.same_size(other.view({ 0, 0, image.height(), image.width() })));
```
