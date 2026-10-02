# image::Image::crop

```cpp
Image<T> crop(Rect region, BorderMode border = BorderMode::constant,
              std::span<const T> outside = {}) const;
```

Returns an independent copy of a region. `region` may extend outside the image. See [Regions](Regions.md) for border modes. For a constant border, `outside` is empty, one scalar to broadcast, or one scalar per channel.

```cpp
image::Image<float> rgb(480, 640, 3, 0.0f);
const std::array<float, 3> black = { 0.0f, 0.0f, 0.0f };
auto padded = rgb.crop({ -16, -16, 128, 128 }, image::BorderMode::constant, black);
auto periodic = rgb.crop({ -16, -16, 128, 128 }, image::BorderMode::wrap);
```
