# image::ImageView::copy_to

```cpp
void copy_to(ImageView<value_type> destination) const;
```

Copies every component of this view into an equal-shaped destination view. The source and destination may be strided. Different dimensions or channel counts throw `std::invalid_argument`. If source and destination overlap, the result is as if all source scalars were first copied to temporary storage.

```cpp
image::Image<float> source(64, 64, 3, 1.0f);
image::Image<float> destination(64, 64, 3, 0.0f);
source.view({ 0, 0, 64, 64 }).copy_to(destination);
```
