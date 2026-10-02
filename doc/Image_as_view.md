# image::Image as ImageView

```cpp
operator ImageView<T>() noexcept;
operator ImageView<const T>() const noexcept;
```

An image converts directly to a mutable or read-only view when permitted by its constness. This lets a non-template API with an `ImageView` parameter accept an `Image` without a duplicate overload. Function templates that must deduce their scalar type provide explicit `Image` overloads as well.

```cpp
void blur(image::ImageView<float> destination);
void measure(image::ImageView<const float> source);

image::Image<float> image(32, 32);
blur(image);
measure(image);
```

The resulting view aliases the image and follows the [ImageView lifetime rules](ImageView.md#lifetime).
