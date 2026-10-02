# image::Image::size

```cpp
size_t size() const noexcept;
```

Returns `height() * width() * channels()`, the number of scalar elements.

```cpp
image::Image<float> rgb(2, 3, 3);
assert(rgb.size() == 18);
```
