# image::Image::clear

```cpp
void clear() noexcept;
```

Erases all elements and makes the image empty. All pointers, references, iterators, and views are invalidated.

```cpp
image.clear();
assert(image.empty());
```
