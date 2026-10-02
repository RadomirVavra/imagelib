# image::Image::begin, end

```cpp
auto begin() noexcept;
auto end() noexcept;
auto begin() const noexcept;
auto end() const noexcept;
```

Return scalar iterators in HWC order.

```cpp
std::fill(image.begin(), image.end(), 0.0f);
```
