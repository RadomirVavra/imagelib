# image::Image::copy_to

```cpp
void copy_to(ImageView<T> destination) const;
```

Copies every component into `destination`. Destination dimensions and channel count must equal this image; otherwise the function throws `std::invalid_argument`. If source and destination overlap, the result is as if all source scalars were first copied to temporary storage.

```cpp
image::Image<float> image(64, 64, 3, 1.0f);
image::Image<float> canvas(128, 128, 3, 0.0f);
image.copy_to(canvas.view({ 32, 32, 64, 64 }));
```
