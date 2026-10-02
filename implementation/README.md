# imagelib implementation notes

## Language and scalar types

- Minimum language level: C++20.
- `Image<T>` permits exactly the fixed-width signed and unsigned integer types, `float`, `double`, and, when `IMAGELIB_USE_HALF != 0`, `image::half`; `ImageView` additionally permits const-qualified forms.

## Contract

- Owned image storage: `std::vector<T>`.
- Layout: contiguous row-major HWC.
- Scalar offset: `((row * width) + column) * channels + channel`.
- Image, resize, and view dimension APIs default `channels` to one where no channel count is supplied.
- Dimensions and all size products: checked before allocation or copy.
- Default or cleared owners and default views have shape `(0, 0, 0)`; other Image/ImageView construction and resize require nonzero channels, while zero height or width is valid.
- Invalid logical arguments throw `std::invalid_argument`; checked index failures throw `std::out_of_range`; checked size, stride, offset, and region arithmetic overflow throws `std::length_error`.
- `Image<T>`: owning value type only.
- `ImageView<T>`: non-owning pointer, dimensions, row stride, pixel stride; components within a pixel remain adjacent.
- Non-empty views require positive strides, `pixel_stride >= channels`, and non-overlapping rows.
- `ImageView` lifetime follows its source; no shared ownership or raw-pointer ownership flag.
- Mutable views convert directly and without allocation to read-only views; non-empty external views require non-null data.
- Generic free algorithms (`crop`, `convert`, and `blit`) provide owner and view overloads because C++ template deduction does not apply user-defined Image-to-ImageView conversions.
- External-buffer callers guarantee accessible storage through the final logical scalar; unchecked access, pixel, row, and column APIs assert preconditions in debug builds and have undefined behavior when violated in release builds.
- `view`: in-bounds, zero-copy ROI.
- `strided_view`: zero-copy sampling view with positive row and column steps; validates steps and region, so it is not `noexcept`. `std::nullopt` selects the whole source; an optional `Rect`, including `Rect{}`, is an unstepped source region with output dimensions ceiling-divided by the steps.
- `crop`: independent allocation; out-of-bounds handled by `BorderMode`.
- `blit`: clipped overwrite, channel-count match, overlap-safe with temporary-equivalent source semantics.
- `copy_from`: accepts any compatible view and creates contiguous owned HWC storage.
- `ImageView::copy_from` and `copy_to`: copy between equal-shaped views; overlap-safe with temporary-equivalent source semantics.
- `Image::copy_from`: creates contiguous owned storage from any compatible view.
- Mutable `ImageView` supports `fill`; storage-changing operations remain exclusive to `Image`.
- `Image` assignment and `copy_from` from `ImageView<const T>` are alias-safe contiguous copies: read source scalars before replacing owned storage.
- Mutating `ImageView` members are constrained away for `ImageView<const T>`.
- Two-index access: valid only when channels equal one; unchecked `operator()` asserts that invariant in debug builds, checked `at()` throws `std::logic_error`.
- `Image<T>&` implicitly converts directly to `ImageView<T>` and `ImageView<const T>`; `Image<T> const&` converts directly to `ImageView<const T>`.
- `numeric` conversion saturates integer destinations after truncation toward zero; normalized conversions are only integer/floating pairs and use documented clamp, round, and NaN behavior.

## Half precision

- Public type: `image::half`, an alias of Christian Rau's maintained, portable IEEE binary16 `half_float::half` implementation, version 2.2.1 (MIT).
- Dependency source: vendored at `include/imagelib/third_party/half.hpp`, with its license at `include/imagelib/third_party/half-MIT.txt`.
- `IMAGELIB_USE_HALF` defaults to `1`; define it as `0` before including `image.hpp` to exclude `image::half` and its third-party dependency.
- Required checks: `sizeof(image::half) == 2`, trivial copyability, IEEE binary16 round-trip tests for normals, subnormals, infinities, NaNs, and signed zero.
- Do not use normalized conversion for HDR measurement data; use `ConversionPolicy::numeric`.

## Validation

- Constructor overflow and mismatched-span tests.
- HWC indexing, two-index single-channel access, pixel span, row/column views, fill, and stepped-view tests.
- Mutable/const view conversion, empty shapes, null-data rules, invalid ROI, and error-category tests.
- Compile-time and behavior tests for `crop`, `convert`, and `blit` with owners, mutable views, read-only views, and every supported source/destination combination.
- Crop tests for `constant`, `clamp`, `wrap`, `reflect`, and `reflect101` on one- and multi-channel images.
- Constant border tests: zero, one-value broadcast, per-channel value, invalid count.
- Blit tests: fully inside, each clipped edge, and overlapping views.

## Candidate methods

| Candidate | Decision | Reason |
| --- | --- | --- |
| Empty/fill/span/vector constructors; copy/move | Implement | Value semantics and bulk I/O. |
| Raw owning pointer or shared-data constructor | No | Lifetime ambiguity. |
| `ImageView` external/strided constructor | Implement | ROIs and adapters without copying. |
| Shape and same-size comparison across Image/View, `data`, scalar span, three-index access | Implement | Core HWC contract and generic owner/view workflows. |
| Two-index single-channel access | Implement | Fast monochrome workflows with checked channel invariant. |
| Pixel span; row and column views; scalar iterators | Implement | Efficient component and line operations without misrepresenting strided data as contiguous. |
| Mutable `vector()` accessor | No | It can violate the shape/storage contract. |
| `view` | Implement | Independent, zero-copy ROI access. |
| `strided_view` | Implement | Whole-image or ROI decimation with output dimensions derived from the steps. |
| `crop` with constant/clamp/wrap/reflect/reflect101 | Implement | Required sampling and padding behavior. |
| Per-channel constant border | Implement | Required multi-channel behavior. |
| `blit`/embed with clipping | Implement | Required composition behavior. |
| Image `copy_from`; ImageView `copy_from` / `copy_to` | Implement | Explicit contiguous materialization and symmetric bulk view copies. |
| Image resize, fill, clear, swap; ImageView fill | Implement | Ownership lifecycle stays on Image; view fill is a logical write operation. |
| Reshape | No | HWC image meaning changes without transforming data. |
| Insert/erase rows or columns; reserve | No | Not an image workload; crop/blit covers required edits. |
| Elementwise arithmetic; transpose/flip/rotate/repeat | Later | Add focused free functions only after a BTF caller requires them. |
| Matrix multiply, `eye`, container concatenation | No | Outside image storage scope. |
| `half` and explicit conversion policies | Implement | HDR and controlled numeric conversion. |
| Implicit image conversion | No | Conversion policy must be explicit. |
| RGB/RGBA pixel classes | Later | Semantic convenience, not universal BTF storage. |
| Metadata, codecs, HDF5 calls, paging, GPU ownership | No | btflib/adapters own these concerns. |
