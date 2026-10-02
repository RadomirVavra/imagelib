# image::Image::scalars

```cpp
std::span<T> scalars() noexcept;
std::span<const T> scalars() const noexcept;
```

Returns all scalar elements in contiguous HWC order. It is equivalent to `std::span<T>(image.data(), image.size())`.

```cpp
std::fill(image.scalars().begin(), image.scalars().end(), 0.0f);
```

For a simple scalar traversal, `begin()` and `end()` are equally appropriate:

```cpp
std::fill(image.begin(), image.end(), 0.0f);
```

Use `scalars()` when an API specifically takes `std::span`, or when the length and pointer-like range are needed together.
