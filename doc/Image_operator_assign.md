# image::Image::operator=

```cpp
Image& operator=(const Image& other);
Image& operator=(Image&& other) noexcept;
Image& operator=(ImageView<const T> source);
```

Replaces this image with a copy of `other` or takes ownership of `other` storage. Existing pointers, references, iterators, and views into this image are invalidated.

```cpp
image::Image<float> source(32, 32, 3, 1.0f);
image::Image<float> destination;
destination = source;
destination = std::move(source);

auto roi = destination.view({ 0, 0, 16, 16 });
destination = roi;
```

Assignment from a view creates an independent contiguous copy. If the view aliases this image, source scalars are read before this image replaces its storage. It accepts `ImageView<T>`, `ImageView<const T>`, and `Image<T>` through their direct read-only view conversions.
