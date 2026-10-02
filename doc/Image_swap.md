# image::Image::swap

```cpp
void swap(Image& other) noexcept;
```

Exchanges the contents of two images without copying scalar elements. Views remain associated with their original storage and therefore observe the image that receives that storage.

```cpp
front.swap(back);
```
