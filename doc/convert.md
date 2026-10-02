# image::convert

```cpp
template<class To, class From>
Image<To> convert(ImageView<From> source,
                  ConversionPolicy policy = ConversionPolicy::numeric);

template<class To, class From>
Image<To> convert(const Image<From>& source,
                  ConversionPolicy policy = ConversionPolicy::numeric);
```

Accepts an `Image<From>`, `ImageView<From>`, or `ImageView<const From>` and returns an independent image with converted scalar elements and the same shape as `source`. `numeric` performs deterministic numerical conversion with saturating integer destinations. `normalized_unorm` and `normalized_snorm` require one integer and one floating type; unsupported policy/type combinations throw `std::invalid_argument`. See [Conversion and half precision](Conversion.md) for the exact range, rounding, and NaN rules.

```cpp
#if IMAGELIB_USE_HALF
auto half_hdr = image::convert<image::half>(float_hdr);
#endif
auto display = image::convert<uint16_t>(float_display,
                                        image::ConversionPolicy::normalized_unorm);
```

See [Conversion and half precision](Conversion.md) for policy semantics and MATLAB correspondence.
