# image::Image::copy_from

```cpp
void copy_from(ImageView<const T> source);
```

Replaces this image with a contiguous owned copy of `source`. The source may be strided. Existing pointers, references, iterators, and views into this image are invalidated.

```cpp
image::Image<float> source(480, 640, 3, 1.0f);
image::Image<float> copy;
copy.copy_from(source);

auto roi = source.view({ 100, 100, 128, 128 });
copy.copy_from(roi);
```

Both calls replace `copy` with a new independent owned copy of the argument's pixels. The parameter type is `ImageView<const T>`: passing `source` first creates a temporary read-only view of the image, while passing `roi` converts its mutable `ImageView<T>` to a read-only view. These conversions affect only how `copy_from` reads the argument; neither argument is modified or made non-owning. If `source` aliases `copy`, all source scalars are read before `copy` replaces its storage.
