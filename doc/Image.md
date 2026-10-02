# image::Image

`image::Image<T>` owns a contiguous, row-major image with scalar elements of type `T`.

The storage order is height, width, channel (HWC):

```text
offset(row, column, channel) = ((row * width) + column) * channels + channel
```

All channels of a pixel are adjacent in memory. `data()` exposes this interleaved buffer for graphics and bulk I/O.

`Image<T>` owns its data. It never aliases an input buffer. `ImageView<T>` represents non-owning data and regions.

## Validation and errors

Every non-default `Image` construction and `resize` requires `channels > 0`; zero height or width is permitted and produces an empty image retaining its requested shape. A default-constructed or cleared image has shape `(0, 0, 0)`. `empty()` is true exactly when `size() == 0`.

Invalid logical arguments, including zero channels, mismatched source sizes, invalid regions, and invalid strides, throw `std::invalid_argument`. Checked element access throws `std::out_of_range` for an invalid index. Overflow while calculating a scalar count, offset, stride, or allocation size throws `std::length_error` before allocation or pointer arithmetic. Allocation failures retain their standard exceptions.

## Template parameter

`T` is a supported non-const scalar type: `std::int8_t`, `std::uint8_t`, `std::int16_t`, `std::uint16_t`, `std::int32_t`, `std::uint32_t`, `std::int64_t`, `std::uint64_t`, `float`, `double`, or, when `IMAGELIB_USE_HALF != 0`, `image::half`. `ImageView<T>` additionally permits `const` of one of those types. `IMAGELIB_USE_HALF` defaults to `1` and must be defined before including `imagelib/image.hpp`. The library requires C++20.

## Member functions

### Construction and assignment

| Function | Description |
| --- | --- |
| [Image](Image_constructor.md) | constructs an image |
| [operator=](Image_operator_assign.md) | assigns an image or independent view copy |

### Capacity and storage

| Function | Description |
| --- | --- |
| [height, width, channels](Image_dimensions.md) | return dimensions |
| [size](Image_size.md) | returns scalar element count |
| [empty](Image_empty.md) | tests for no elements |
| [same_size](Image_same_size.md) | compares dimensions |
| [data](Image_data.md) | returns the storage pointer |
| [scalars](Image_scalars.md) | returns all scalar elements |

### Element access

| Function | Description |
| --- | --- |
| [operator()](Image_operator_function.md) | unchecked component access |
| [at](Image_at.md) | checked component access |
| [pixel](Image_pixel.md) | returns all components of one pixel |
| [begin, end](Image_begin_end.md) | iterate scalar elements |

### Views and copying

| Function | Description |
| --- | --- |
| [view, row, column](Image_view.md) | return in-bounds zero-copy views |
| [strided_view](Image_strided_view.md) | returns an in-bounds stepped zero-copy region |
| [crop](Image_crop.md) | copies a region with border handling |
| [copy_from](Image_copy_from.md) | replaces this image with a view copy |
| [copy_to](Image_copy_to.md) | copies this image into a view |
| [as ImageView](Image_as_view.md) | passes an image to view-based functions |
| [blit](blit.md) | copies a clipped source into a destination |

### Modifiers

| Function | Description |
| --- | --- |
| [resize](Image_resize.md) | replaces the image with a filled new size |
| [fill](Image_fill.md) | assigns every scalar element |
| [clear](Image_clear.md) | erases all elements |
| [swap](Image_swap.md) | exchanges image contents |

## Examples

### Single-channel image

```cpp
image::Image<float> mask(480, 640); // channels defaults to one
mask(12, 24) = 1.0f;
```

### Multi-channel image and region copy

```cpp
image::Image<float> rgb(480, 640, 3, 0.0f);
rgb(12, 24, 0) = 1.0f;

auto roi = rgb.view({ 100, 100, 128, 128 });
image::Image<float> independent = roi;
```

## See also

- [ImageView](ImageView.md)
- [Regions](Regions.md)
- [Conversion and half](Conversion.md)
