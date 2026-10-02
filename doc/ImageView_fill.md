# image::ImageView::fill

```cpp
void fill(const value_type& value) const
	requires (!std::is_const_v<T>);
```

Assigns `value` to every scalar in a mutable view without changing its shape or strides. It visits the view logically, so it also works for regions and stepped views. This member is unavailable on `ImageView<const T>`.

```cpp
image::Image<float> image(100, 100, 3, 1.0f);
auto roi = image.view({ 20, 20, 40, 40 });
roi.fill(0.0f);
```
