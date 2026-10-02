# Conversion and half precision

## image::half

When `IMAGELIB_USE_HALF != 0`, `image::half` is an IEEE 754 binary16 value type. It occupies two bytes and preserves binary16 finite values, infinities, NaNs, and signed zero. Half support is enabled by default; define `IMAGELIB_USE_HALF=0` before including `imagelib/image.hpp` to omit `image::half` and its third-party dependency.

```cpp
image::Image<image::half> hdr(1024, 1024, 3);
hdr(0, 0, 0) = image::half{ 8.0f };
```

Binary16 has 5 exponent bits and 10 explicit fraction bits. Its finite normal range is approximately $6.10 \times 10^{-5}$ through $65504$. It is appropriate for HDR values whose precision decreases with magnitude; it is not an integer-normalized colour format.

## Conversion policy

```cpp
enum class ConversionPolicy { numeric, normalized_unorm, normalized_snorm };
```

`numeric` preserves numerical meaning without normalizing image ranges. Integer destinations saturate to their representable range after truncation toward zero; a NaN source converts to zero. Floating-point destinations use the target floating type's ordinary conversion, preserving NaNs and infinities when representable.

`normalized_unorm` is valid only between one unsigned integer type and one floating type (`float`, `double`, or `half` when enabled). Integer-to-float divides by the unsigned maximum. Float-to-integer clamps to $[0, 1]$, maps to the unsigned maximum, and rounds to nearest with half-way values away from zero. NaN maps to zero.

`normalized_snorm` is valid only between one signed integer type and one floating type. Integer-to-float divides by the signed positive maximum and clamps the result to $[-1, 1]$; this maps the extra negative minimum to $-1$. Float-to-integer clamps to $[-1, 1]$, maps using the signed positive maximum, and rounds to nearest with half-way values away from zero. NaN maps to zero. Invalid policy/type combinations throw `std::invalid_argument`. Floating-point HDR measurements normally use `numeric`. See [convert](convert.md).

## MATLAB correspondence

| imagelib policy | MATLAB operation | Meaning |
| --- | --- | --- |
| `numeric` | cast, for example `single(A)` | Converts the numerical value without normalizing image ranges; integer destinations saturate. |
| `normalized_unorm` | `im2single`, `im2uint8`, `im2uint16` | Maps unsigned integer ranges to or from $[0, 1]$, clamps values outside that interval, and rounds when converting to integers. |
| `normalized_snorm` | no direct `im2*` equivalent | Maps signed integer ranges to or from $[-1, 1]$. |

For example, converting `uint8_t{128}` with `normalized_unorm` produces approximately `0.50196f`, as `im2single(uint8(128))` does. Converting `0.5f` to `uint8_t` with `normalized_unorm` produces `128`, as `im2uint8(single(0.5))` does.
