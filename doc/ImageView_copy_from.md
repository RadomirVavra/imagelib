# image::ImageView::copy_from

```cpp
void copy_from(ImageView<const value_type> source) const
	requires (!std::is_const_v<T>);
```

Copies every component of an equal-shaped source view into this mutable view. Source and destination may be strided. Different dimensions or channel counts throw `std::invalid_argument`. If source and destination overlap, the result is as if all source scalars were first copied to temporary storage.

Unlike `Image::copy_from`, this function never reallocates or changes shape: it writes through the view into its existing referenced storage.

```cpp
image::Image<float> source(64, 64, 3, 1.0f);
image::Image<float> destination(64, 64, 3, 0.0f);
destination.view({ 0, 0, 64, 64 }).copy_from(source);
```
