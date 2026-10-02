# Regions, Borders, and Blitting

## Rect

```cpp
struct Rect {
    ptrdiff_t row;
    ptrdiff_t column;
    size_t height;
    size_t width;
};
```

`row` and `column` may be negative. `height` and `width` are non-negative. Functions with in-bounds-only regions (`view` and `strided_view`) reject negative origins; border-aware `crop` accepts them.

## BorderMode

```cpp
enum class BorderMode { constant, clamp, wrap, reflect, reflect101 };
```

| Mode | Mapping outside a dimension of length `n` |
| --- | --- |
| `constant` | Uses the supplied outside pixel. |
| `clamp` | Repeats the nearest edge index. |
| `wrap` | Uses modulo `n`. |
| `reflect` | Mirrors including the edge: `0 1 2 2 1 0 ...`. |
| `reflect101` | Mirrors excluding the edge: `0 1 2 1 0 1 ...`. |

Non-constant modes require non-empty source dimensions.

## crop

```cpp
template<class T>
Image<std::remove_const_t<T>> crop(ImageView<T> source, Rect region,
                                   BorderMode border = BorderMode::constant,
                                   std::span<const std::remove_const_t<T>> outside = {});

template<class T>
Image<T> crop(const Image<T>& source, Rect region,
              BorderMode border = BorderMode::constant,
              std::span<const T> outside = {});
```

`crop` accepts an `Image<T>`, `ImageView<T>`, or `ImageView<const T>` and copies a region into a new `Image<T>`. Its output has `region.height`, `region.width`, and `source.channels()`. A zero-size region returns an empty image without sampling. For `BorderMode::constant`, `outside` must contain zero values (all zero), one value (broadcast), or exactly `source.channels()` values; other counts throw `std::invalid_argument`. Non-constant modes with a non-empty output require non-empty source dimensions and otherwise throw `std::invalid_argument`. Region arithmetic overflow throws `std::length_error`.

```cpp
image::Image<float> source(128, 128, 3, 1.0f);
auto repeated = image::crop(source, { -8, -8, 64, 64 }, image::BorderMode::clamp);
auto periodic = image::crop(source, { -8, -8, 64, 64 }, image::BorderMode::wrap);
```

## See also

- [blit](blit.md)
