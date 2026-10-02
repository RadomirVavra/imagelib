# image::ImageView dimensions

```cpp
size_t height() const noexcept;
size_t width() const noexcept;
size_t channels() const noexcept;
size_t size() const noexcept;
bool empty() const noexcept;
bool same_size(ImageView<const value_type> other) const noexcept;
```

Return the view dimensions, scalar element count, and empty state. `same_size` compares height, width, and channel count; a mutable view converts directly to the read-only argument view.

```cpp
assert(view.size() == view.height() * view.width() * view.channels());
assert(view.same_size(image));
```
