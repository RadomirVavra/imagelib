# image::blit

```cpp
template<class Destination, class Source>
void blit(ImageView<Destination> destination, ImageView<Source> source,
        ptrdiff_t destination_row, ptrdiff_t destination_column)
    requires (!std::is_const_v<Destination> &&
          std::same_as<std::remove_const_t<Destination>,
                   std::remove_const_t<Source>>);

template<class Destination, class Source>
void blit(Image<Destination>& destination, ImageView<Source> source,
        ptrdiff_t destination_row, ptrdiff_t destination_column)
    requires std::same_as<Destination, std::remove_const_t<Source>>;

template<class Destination, class Source>
void blit(ImageView<Destination> destination, const Image<Source>& source,
        ptrdiff_t destination_row, ptrdiff_t destination_column)
    requires (!std::is_const_v<Destination> &&
          std::same_as<std::remove_const_t<Destination>, Source>);

template<class Destination, class Source>
void blit(Image<Destination>& destination, const Image<Source>& source,
          ptrdiff_t destination_row, ptrdiff_t destination_column)
    requires std::same_as<Destination, Source>;
```

Accepts an `Image` or `ImageView` destination and an `Image`, mutable view, or read-only view source, provided their unqualified scalar types match. Overwrites the intersection of `source`, placed at `(destination_row, destination_column)`, and `destination`. Parts of the source outside the destination are ignored. Source and destination must have equal channel counts; a mismatch throws `std::invalid_argument`. If source and destination overlap, the result is as if the clipped source scalars were first copied to temporary storage. Coordinate arithmetic overflow throws `std::length_error`.

```cpp
image::Image<float> canvas(512, 512, 3, 0.0f);
image::Image<float> logo(64, 64, 3, 1.0f);
image::blit(canvas, logo, -8, 480);
```
