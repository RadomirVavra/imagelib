# image::Image::empty

```cpp
bool empty() const noexcept;
```

Returns true when the image has no scalar elements.

```cpp
image::Image<float> image;
assert(image.empty());
```
