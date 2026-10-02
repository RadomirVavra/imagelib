# image::ImageView

`image::ImageView<T>` is a non-owning rectangular HWC image view. It supports contiguous and strided data.

```cpp
template<class T>
class ImageView;
```

Views provide zero-copy access to an `Image`, an ROI, or an external buffer. They do not manage lifetime.

`value_type` is `std::remove_const_t<T>`, which must be a supported image scalar type. Mutating members participate in overload resolution only when `T` is not const.

## Member functions

| Function | Description |
| --- | --- |
| [ImageView](ImageView_constructor.md) | creates a view over external storage |
| [height, width, channels, size, empty, same_size](ImageView_dimensions.md) | return view dimensions and state |
| [data](ImageView_data.md) | returns the first scalar pointer |
| [row_stride, pixel_stride](ImageView_strides.md) | return strides in scalar elements |
| [is_contiguous](ImageView_is_contiguous.md) | tests HWC contiguity |
| [operator(), at](ImageView_access.md) | access scalar components |
| [pixel](ImageView_pixel.md) | accesses adjacent components of one pixel |
| [row, column](ImageView_row.md) | return one-row or one-column views |
| [view](ImageView_view.md) | returns a subview |
| [strided_view](ImageView_strided_view.md) | returns a stepped subview |
| [copy_from](ImageView_copy_from.md) | copies an equal-shaped source view into this view |
| [copy_to](ImageView_copy_to.md) | copies into an equal-shaped destination view |
| [fill](ImageView_fill.md) | assigns every scalar in a mutable view |

## Example

```cpp
void normalize(image::ImageView<float> destination);
void inspect(image::ImageView<const float> source);

image::Image<float> image(64, 64);
normalize(image);
inspect(image);

void clear_region(image::ImageView<float> region) {
	for (size_t row = 0; row != region.height(); ++row) {
		for (size_t column = 0; column != region.width(); ++column) {
			region(row, column) = 0.0f;
		}
	}
}

clear_region(image.view({ 16, 16, 32, 32 }));
```

An `Image<T>&` converts directly to both `ImageView<T>` and `ImageView<const T>`. An `Image<T> const&` converts directly to `ImageView<const T>`. A non-template function with an `ImageView` parameter therefore also accepts a compatible `Image`; generic free algorithms document their explicit owner overloads.

## Constness

`ImageView<const T>` provides read-only access. A mutable view converts to its const counterpart.

```cpp
image::ImageView<const float> readonly = image.view({ 0, 0, 32, 32 });
float first = readonly(0, 0);
```

## Lifetime

The caller keeps the referenced data alive. Resizing, clearing, moving from, or destroying an owning `Image` invalidates all views into it.
