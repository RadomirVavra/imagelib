#include <imagelib/image.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

class TestFailure final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

#define CHECK(condition)                                                                                               \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            throw TestFailure("check failed: " #condition);                                                          \
        }                                                                                                              \
    } while (false)

template<class Exception, class Function>
void expect_throw(Function&& function) {
    try {
        function();
    } catch (const Exception&) {
        return;
    } catch (...) {
        throw TestFailure("unexpected exception type");
    }
    throw TestFailure("expected exception was not thrown");
}

using AllScalars = std::tuple<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t,
                              std::int32_t, std::uint32_t, std::int64_t, std::uint64_t,
                              float, double
#if IMAGELIB_USE_HALF
                              , image::half
#endif
                              >;
using UnsignedScalars = std::tuple<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
using SignedScalars = std::tuple<std::int8_t, std::int16_t, std::int32_t, std::int64_t>;
using FloatingScalars = std::tuple<float, double
#if IMAGELIB_USE_HALF
                                   , image::half
#endif
                                   >;

template<class From, class To>
void verify_numeric_conversion_pair() {
    image::Image<From> source(1, 1, 1, static_cast<From>(1));
    const auto converted = image::convert<To>(source);
    CHECK(converted.height() == 1 && converted.width() == 1 && converted.channels() == 1);
    (void)converted(0, 0);
}

template<class From, class... To>
void verify_numeric_destinations(std::tuple<To...>) {
    (verify_numeric_conversion_pair<From, To>(), ...);
}

template<class... From>
void verify_all_numeric_conversion_pairs(std::tuple<From...> types) {
    (verify_numeric_destinations<From>(types), ...);
}

template<class From, class To>
void verify_normalized_conversion_pair(image::ConversionPolicy policy) {
    image::Image<From> source(1, 1, 1, static_cast<From>(1));
    const auto converted = image::convert<To>(source, policy);
    CHECK(converted.size() == 1);
    (void)converted(0, 0);
}

template<class Integer, class... Floating>
void verify_integer_floating_pairs(std::tuple<Floating...>, image::ConversionPolicy policy) {
    (verify_normalized_conversion_pair<Integer, Floating>(policy), ...);
    (verify_normalized_conversion_pair<Floating, Integer>(policy), ...);
}

template<class... Integer>
void verify_all_integer_floating_pairs(std::tuple<Integer...>, image::ConversionPolicy policy) {
    (verify_integer_floating_pairs<Integer>(FloatingScalars{}, policy), ...);
}

#if IMAGELIB_USE_HALF
void test_half_precision_type() {
    static_assert(sizeof(image::half) == 2);
    static_assert(std::is_trivially_copyable_v<image::half>);

    CHECK(static_cast<float>(image::half{1.5f}) == 1.5f);
    CHECK(static_cast<float>(image::half{65504.0f}) == 65504.0f);
    CHECK(static_cast<float>(image::half{0.00006103515625f}) == 0.00006103515625f);
    CHECK(static_cast<float>(image::half{0.000000059604644775390625f}) == 0.000000059604644775390625f);
    CHECK(std::isinf(static_cast<float>(std::numeric_limits<image::half>::infinity())));
    CHECK(std::isnan(static_cast<float>(std::numeric_limits<image::half>::quiet_NaN())));
    CHECK(std::signbit(static_cast<float>(image::half{-0.0f})));
}
#endif

void test_image_construction_and_ownership() {
    static_assert(std::is_constructible_v<image::Image<float>, image::ImageView<float>>);
    static_assert(std::is_constructible_v<image::Image<float>, image::ImageView<const float>>);

    image::Image<float> empty;
    CHECK(empty.empty());
    CHECK(empty.height() == 0 && empty.width() == 0 && empty.channels() == 0 && empty.size() == 0);
    CHECK(empty.data() == nullptr);
    CHECK(empty.view({0, 0, 0, 0}).empty());

    image::Image<int> filled(2, 3, 2, 7);
    CHECK(filled.size() == 12);
    CHECK(filled.height() == 2 && filled.width() == 3 && filled.channels() == 2);
    for (const int value : filled) {
        CHECK(value == 7);
    }

    const std::vector<int> input = {0, 1, 2, 3, 4, 5};
    image::Image<int> from_span(input, 2, 3);
    CHECK(from_span(1, 2) == 5);

    auto moved_values = std::vector<int>{8, 9, 10, 11};
    image::Image<int> from_vector(std::move(moved_values), 2, 2);
    CHECK(from_vector(1, 1) == 11);

    image::Image<int> copied = from_vector;
    copied(0, 0) = -1;
    CHECK(from_vector(0, 0) == 8);

    image::Image<int> copy_assigned;
    copy_assigned = from_vector;
    copy_assigned(0, 1) = -2;
    CHECK(from_vector(0, 1) == 9);

    image::Image<int> moved = std::move(copied);
    CHECK(moved(0, 0) == -1);
    CHECK(copied.empty());

    expect_throw<std::invalid_argument>([] { image::Image<int> invalid(1, 1, 0); });
    expect_throw<std::invalid_argument>([] {
        const std::vector<int> values = {1, 2};
        (void)image::Image<int>(values, 1, 3);
    });
}

void test_image_access_and_mutation() {
    image::Image<std::uint8_t> image(2, 3, 2);
    for (std::size_t index = 0; index != image.size(); ++index) {
        image.scalars()[index] = static_cast<std::uint8_t>(index);
    }

    CHECK(image(1, 2, 1) == 11);
    CHECK(image.at(1, 0, 0) == 6);
    CHECK(image.pixel(1, 1).size() == 2);
    CHECK(image.pixel(1, 1)[0] == 8 && image.pixel(1, 1)[1] == 9);
    expect_throw<std::logic_error>([&] { (void)image.at(0, 0); });
    expect_throw<std::out_of_range>([&] { (void)image.at(2, 0, 0); });

    image::Image<int> mono(2, 2, 1, 1);
    mono.at(1, 1) = 4;
    CHECK(mono(1, 1) == 4);
    mono.fill(6);
    CHECK(mono(0, 0) == 6 && mono(1, 1) == 6);

    mono.resize(1, 3, 1, 9);
    CHECK(mono.height() == 1 && mono.width() == 3 && mono.size() == 3);
    CHECK(mono(0, 2) == 9);
    mono.clear();
    CHECK(mono.empty() && mono.channels() == 0);
}

void test_image_lifecycle_const_and_view_apis() {
    image::Image<int> source(2, 2, 1);
    source(0, 0) = 1;
    source(0, 1) = 2;
    source(1, 0) = 3;
    source(1, 1) = 4;

    const image::Image<int>& constant_source = source;
    CHECK(constant_source.data() == source.data());
    CHECK(constant_source.scalars().size() == 4);
    CHECK(constant_source.begin()[0] == 1 && constant_source.end() - constant_source.begin() == 4);
    CHECK(constant_source(1, 0) == 3 && constant_source.at(1, 1) == 4);
    CHECK(constant_source.pixel(0, 1)[0] == 2);
    CHECK(constant_source.row(1)(0, 0) == 3);
    CHECK(constant_source.column(1)(1, 0) == 4);
    CHECK(constant_source.view({0, 0, 1, 2}).at(0, 1) == 2);
    CHECK(constant_source.strided_view(2, 1).at(0, 1) == 2);

    image::ImageView<int> mutable_view = source;
    image::ImageView<const int> const_view = constant_source;
    CHECK(mutable_view.same_size(const_view));
    CHECK(source.same_size(const_view));
    CHECK(!source.same_size(image::ImageView<const int>(constant_source.view({0, 0, 1, 2}))));
    mutable_view(0, 0) = 8;
    CHECK(source(0, 0) == 8);
    CHECK(const_view(0, 0) == 8 && const_view.pixel(1, 1)[0] == 4);
    CHECK(const_view.row(1).at(0, 0) == 3 && const_view.column(0).at(1, 0) == 3);
    CHECK(const_view.view({0, 0, 1, 2}).at(0, 1) == 2);
    CHECK(const_view.strided_view(2, 1).at(0, 1) == 2);

    image::Image<int> assigned;
    assigned = const_view;
    CHECK(assigned(0, 0) == 8 && assigned.data() != source.data());
    assigned.copy_from(const_view.view({1, 0, 1, 2}));
    CHECK(assigned.height() == 1 && assigned.width() == 2 && assigned(0, 1) == 4);

    image::Image<int> destination(1, 2, 1, 0);
    assigned.copy_to(static_cast<image::ImageView<int>>(destination));
    CHECK(destination(0, 0) == 3 && destination(0, 1) == 4);

    image::Image<int> other(1, 1, 1, 12);
    assigned.swap(other);
    CHECK(assigned(0, 0) == 12 && other.width() == 2);
    using std::swap;
    swap(assigned, other);
    CHECK(assigned.width() == 2 && other(0, 0) == 12);

    image::Image<int> move_assigned;
    move_assigned = std::move(other);
    CHECK(move_assigned(0, 0) == 12 && other.empty());
}

void test_image_view_external_storage_and_access() {
    std::vector<float> storage(24, -1.0f);
    image::ImageView<float> view(storage.data(), 2, 3, 1, 12, 2);
    CHECK(view.height() == 2 && view.width() == 3 && view.channels() == 1);
    CHECK(view.row_stride() == 12 && view.pixel_stride() == 2);
    CHECK(!view.is_contiguous());

    view.fill(3.0f);
    CHECK(storage[0] == 3.0f && storage[4] == 3.0f && storage[12] == 3.0f);
    CHECK(view.at(1, 2) == 3.0f);
    CHECK(view.pixel(0, 1)[0] == 3.0f);

    image::ImageView<const float> read_only = view;
    CHECK(read_only.data() == view.data());
    CHECK(read_only.same_size(view));
    expect_throw<std::out_of_range>([&] { (void)view.at(2, 0); });
    expect_throw<std::invalid_argument>([] { image::ImageView<float> invalid(nullptr, 1, 1); });
    expect_throw<std::invalid_argument>([&] { image::ImageView<float> invalid(storage.data(), 2, 2, 2, 3, 2); });
}

void test_image_view_defaults_copy_and_derived_views() {
    image::ImageView<int> empty;
    CHECK(empty.empty() && empty.data() == nullptr);
    CHECK(empty.height() == 0 && empty.width() == 0 && empty.channels() == 0);
    image::ImageView<const int> const_empty;
    CHECK(const_empty.empty() && const_empty.data() == nullptr);

    std::array<int, 6> values = {0, 1, 2, 3, 4, 5};
    image::ImageView<int> contiguous(values.data(), 2, 3);
    CHECK(contiguous.row_stride() == 3 && contiguous.pixel_stride() == 1);
    CHECK(contiguous.is_contiguous());
    CHECK(contiguous.size() == 6 && contiguous(1, 2) == 5);
    contiguous.at(0, 0) = 9;
    CHECK(values[0] == 9);
    CHECK(contiguous.row(1).at(0, 1) == 4);
    CHECK(contiguous.column(1).at(1, 0) == 4);
    CHECK(contiguous.view({0, 1, 2, 2}).at(1, 1) == 5);
    CHECK(contiguous.strided_view(1, 2, image::Rect{0, 0, 2, 3}).at(1, 1) == 5);
    CHECK(contiguous.strided_view(3, 4, image::Rect{}).empty());

    std::array<int, 4> copied = {};
    image::ImageView<int> copied_view(copied.data(), 2, 2);
    contiguous.view({0, 1, 2, 2}).copy_to(copied_view);
    CHECK(copied[0] == 1 && copied[3] == 5);
    copied_view.fill(-1);
    contiguous.view({0, 1, 2, 2}).copy_from(image::ImageView<const int>(copied_view));
    CHECK(contiguous(0, 1) == -1 && contiguous(1, 2) == -1);

    image::ImageView<int> wrong_shape(values.data(), 1, 3);
    expect_throw<std::invalid_argument>([&] { wrong_shape.copy_from(image::ImageView<const int>(copied_view)); });
    expect_throw<std::logic_error>([&] { (void)image::ImageView<int>(values.data(), 1, 1, 2).at(0, 0); });
    expect_throw<std::invalid_argument>([&] { (void)contiguous.view({0, 3, 1, 1}); });
    expect_throw<std::invalid_argument>([&] { (void)contiguous.strided_view(1, 0); });
    expect_throw<std::invalid_argument>([&] { image::ImageView<int> invalid(values.data(), 1, 1, 2, 2, 1); });
}

void test_zero_extent_invalid_stride_and_overflow_contracts() {
    image::Image<int> zero_height(0, 4, 2);
    image::Image<int> zero_width(4, 0, 2);
    CHECK(zero_height.empty() && zero_height.height() == 0 && zero_height.width() == 4 && zero_height.channels() == 2);
    CHECK(zero_width.empty() && zero_width.height() == 4 && zero_width.width() == 0 && zero_width.channels() == 2);

    image::ImageView<int> null_zero_extent(nullptr, 0, 4, 2);
    CHECK(null_zero_extent.empty() && null_zero_extent.row_stride() == 8 && null_zero_extent.pixel_stride() == 2);

    int scalar = 0;
    expect_throw<std::invalid_argument>([&] { image::ImageView<int> invalid(&scalar, 1, 1, 0); });
    expect_throw<std::invalid_argument>([&] { image::ImageView<int> invalid(&scalar, 1, 1, 1, -1, 1); });
    expect_throw<std::invalid_argument>([&] { image::ImageView<int> invalid(&scalar, 1, 1, 2, 2, 1); });
    expect_throw<std::invalid_argument>([&] { image::ImageView<int> invalid(&scalar, 2, 2, 1, 1, 1); });

    const auto maximum = std::numeric_limits<std::size_t>::max();
    expect_throw<std::length_error>([&] { (void)image::Image<int>(maximum, 2); });
    expect_throw<std::length_error>([&] { (void)image::ImageView<int>(&scalar, maximum, 2); });

    image::Image<int> source(2, 2, 1, 1);
    expect_throw<std::length_error>([&] { (void)source.strided_view(maximum, 1); });
    expect_throw<std::length_error>([&] {
        (void)image::crop(source, {std::numeric_limits<std::ptrdiff_t>::max(), 0, 2, 1});
    });
    expect_throw<std::length_error>([&] {
        image::blit(source, source, std::numeric_limits<std::ptrdiff_t>::max(), 0);
    });
}

void test_views_and_copy_operations() {
    image::Image<int> source(3, 4, 1);
    for (std::size_t index = 0; index != source.size(); ++index) {
        source.scalars()[index] = static_cast<int>(index);
    }

    auto region = source.view({1, 1, 2, 2});
    CHECK(region(0, 0) == 5 && region(1, 1) == 10);
    region(1, 1) = 99;
    CHECK(source(2, 2) == 99);

    image::Image<int> materialized = region;
    materialized(0, 0) = -1;
    CHECK(source(1, 1) == 5);

    auto row = source.row(1);
    auto column = source.column(2);
    CHECK(row.height() == 1 && row.width() == 4 && row(0, 3) == 7);
    CHECK(column.height() == 3 && column.width() == 1 && column(2, 0) == 99);

    auto stepped = source.strided_view(2, 2);
    CHECK(stepped.height() == 2 && stepped.width() == 2);
    CHECK(!stepped.is_contiguous());
    CHECK(stepped(1, 1) == 99);
    expect_throw<std::invalid_argument>([&] { (void)source.view({-1, 0, 1, 1}); });
    expect_throw<std::invalid_argument>([&] { (void)source.strided_view(0, 1); });

    image::Image<int> copy_source(2, 3, 1);
    for (std::size_t index = 0; index != copy_source.size(); ++index) {
        copy_source.scalars()[index] = static_cast<int>(index);
    }
    copy_source.view({0, 0, 2, 2}).copy_to(copy_source.view({0, 1, 2, 2}));
    CHECK(copy_source(0, 1) == 0 && copy_source(1, 2) == 4);
}

void test_regions_blit_and_conversion() {
    image::Image<float> source(2, 2, 1, 0.0f);
    source(0, 0) = 1.0f;
    source(0, 1) = 2.0f;
    source(1, 0) = 3.0f;
    source(1, 1) = 4.0f;

    CHECK(source.crop({0, 0, 1, 1})(0, 0) == 1.0f);
    CHECK(image::crop(source, {-1, -1, 2, 2})(0, 0) == 0.0f);
    CHECK(image::crop(source, {-1, -1, 2, 2}, image::BorderMode::clamp)(0, 0) == 1.0f);
    CHECK(image::crop(source, {-1, 0, 1, 1}, image::BorderMode::wrap)(0, 0) == 3.0f);
    CHECK(image::crop(source, {2, 0, 1, 1}, image::BorderMode::reflect)(0, 0) == 3.0f);
    CHECK(image::crop(source, {2, 0, 1, 1}, image::BorderMode::reflect101)(0, 0) == 1.0f);

    image::Image<float> canvas(3, 3, 1, 0.0f);
    image::blit(canvas, source, -1, -1);
    CHECK(canvas(0, 0) == 4.0f);

    image::Image<float> normalized(1, 3, 1);
    normalized(0, 0) = -1.0f;
    normalized(0, 1) = 0.5f;
    normalized(0, 2) = std::numeric_limits<float>::quiet_NaN();
    const auto bytes = image::convert<std::uint8_t>(normalized, image::ConversionPolicy::normalized_unorm);
    CHECK(bytes(0, 0) == 0 && bytes(0, 1) == 128 && bytes(0, 2) == 0);
    const auto floats = image::convert<float>(bytes, image::ConversionPolicy::normalized_unorm);
    CHECK(std::abs(floats(0, 1) - 128.0f / 255.0f) < 0.0001f);
#if IMAGELIB_USE_HALF
    const auto half_image = image::convert<image::half>(normalized);
    CHECK(static_cast<float>(half_image(0, 1)) == 0.5f);
#endif
    expect_throw<std::invalid_argument>([&] {
        (void)image::convert<std::uint16_t>(bytes, image::ConversionPolicy::normalized_unorm);
    });
}

void test_crop_and_blit_overloads_and_failures() {
    image::Image<int> rgb(1, 2, 3);
    rgb(0, 0, 0) = 1;
    rgb(0, 0, 1) = 2;
    rgb(0, 0, 2) = 3;
    rgb(0, 1, 0) = 4;
    rgb(0, 1, 1) = 5;
    rgb(0, 1, 2) = 6;
    const std::array<int, 1> broadcast = {7};
    const std::array<int, 3> per_channel = {8, 9, 10};
    CHECK(image::crop(rgb, {-1, 0, 1, 1}, image::BorderMode::constant, broadcast).pixel(0, 0)[2] == 7);
    CHECK(image::crop(rgb.view({0, 0, 1, 2}), {-1, 0, 1, 1}, image::BorderMode::constant, per_channel).pixel(0, 0)[1] == 9);
    const image::ImageView<const int> const_rgb = rgb;
    CHECK(image::crop(const_rgb, {0, 1, 1, 1}).pixel(0, 0)[0] == 4);
    CHECK(image::crop(rgb, {0, 0, 0, 1}).empty());
    expect_throw<std::invalid_argument>([&] { (void)image::crop(rgb, {-1, 0, 1, 1}, image::BorderMode::constant, std::array<int, 2>{}); });

    image::Image<int> empty(0, 0, 1);
    expect_throw<std::invalid_argument>([&] { (void)image::crop(empty, {0, 0, 1, 1}, image::BorderMode::clamp); });

    image::Image<int> source(2, 2, 1);
    source(0, 0) = 1;
    source(0, 1) = 2;
    source(1, 0) = 3;
    source(1, 1) = 4;
    image::Image<int> owner_destination(3, 3, 1, 0);
    image::blit(owner_destination, source, 1, 1);
    CHECK(owner_destination(2, 2) == 4);
    image::blit(static_cast<image::ImageView<int>>(owner_destination), source, 0, 0);
    CHECK(owner_destination(1, 1) == 4);
    image::Image<int> from_view(2, 2, 1, 0);
    image::blit(from_view, source.view({0, 0, 2, 2}), 0, 0);
    CHECK(from_view(1, 0) == 3);
    image::Image<int> view_destination_owner(2, 2, 1, 0);
    image::blit(static_cast<image::ImageView<int>>(view_destination_owner), source.view({0, 0, 2, 2}), 0, 0);
    CHECK(view_destination_owner(0, 1) == 2);
    image::Image<int> const_view_destination(2, 2, 1, 0);
    const image::ImageView<const int> read_only_source = source;
    image::blit(static_cast<image::ImageView<int>>(const_view_destination), read_only_source, 0, 0);
    CHECK(const_view_destination(1, 0) == 3);
    image::blit(owner_destination, source, -1, 2);
    CHECK(owner_destination(0, 2) == 3);

    image::Image<int> overlapping(1, 4, 1);
    for (std::size_t column = 0; column != 4; ++column) {
        overlapping(0, column) = static_cast<int>(column);
    }
    image::blit(overlapping.view({0, 1, 1, 3}), overlapping.view({0, 0, 1, 3}), 0, 0);
    CHECK(overlapping(0, 0) == 0 && overlapping(0, 1) == 0 && overlapping(0, 3) == 2);
    expect_throw<std::invalid_argument>([&] { image::blit(owner_destination, rgb, 0, 0); });
}

void test_conversion_policies_and_view_sources() {
    image::Image<float> values(1, 5, 1);
    values(0, 0) = -40000.5f;
    values(0, 1) = -1.9f;
    values(0, 2) = 1.9f;
    values(0, 3) = 40000.5f;
    values(0, 4) = std::numeric_limits<float>::quiet_NaN();
    const auto numeric = image::convert<std::int16_t>(values);
    CHECK(numeric(0, 0) == std::numeric_limits<std::int16_t>::lowest());
    CHECK(numeric(0, 1) == -1 && numeric(0, 2) == 1);
    CHECK(numeric(0, 3) == std::numeric_limits<std::int16_t>::max() && numeric(0, 4) == 0);

    image::Image<std::int8_t> signed_values(1, 3, 1);
    signed_values(0, 0) = std::numeric_limits<std::int8_t>::lowest();
    signed_values(0, 1) = 0;
    signed_values(0, 2) = std::numeric_limits<std::int8_t>::max();
    const auto signed_floats = image::convert<float>(signed_values, image::ConversionPolicy::normalized_snorm);
    CHECK(signed_floats(0, 0) == -1.0f && signed_floats(0, 1) == 0.0f && signed_floats(0, 2) == 1.0f);
    const auto signed_round_trip = image::convert<std::int8_t>(signed_floats, image::ConversionPolicy::normalized_snorm);
    CHECK(signed_round_trip(0, 0) == -127 && signed_round_trip(0, 2) == 127);

    image::Image<float> unorm_values(1, 4, 1);
    unorm_values(0, 0) = -1.0f;
    unorm_values(0, 1) = 0.5f;
    unorm_values(0, 2) = 2.0f;
    unorm_values(0, 3) = std::numeric_limits<float>::infinity();
    const auto unorm = image::convert<std::uint8_t>(unorm_values.view({0, 0, 1, 4}), image::ConversionPolicy::normalized_unorm);
    CHECK(unorm(0, 0) == 0 && unorm(0, 1) == 128 && unorm(0, 2) == 255 && unorm(0, 3) == 255);
    const image::ImageView<const float> read_only = unorm_values;
    const auto copied_float = image::convert<double>(read_only);
    CHECK(copied_float.height() == 1 && copied_float.width() == 4 && copied_float(0, 1) == 0.5);
    expect_throw<std::invalid_argument>([&] { (void)image::convert<float>(values, image::ConversionPolicy::normalized_unorm); });
    expect_throw<std::invalid_argument>([&] { (void)image::convert<std::uint8_t>(signed_values, image::ConversionPolicy::normalized_snorm); });
}

void test_all_supported_scalar_conversion_pairs() {
    verify_all_numeric_conversion_pairs(AllScalars{});
    verify_all_integer_floating_pairs(UnsignedScalars{}, image::ConversionPolicy::normalized_unorm);
    verify_all_integer_floating_pairs(SignedScalars{}, image::ConversionPolicy::normalized_snorm);
}

struct TestCase {
    std::string_view name;
    void (*function)();
};

int main() {
    const TestCase tests[] = {
#if IMAGELIB_USE_HALF
        {"Half precision type", test_half_precision_type},
#endif
        {"Image construction and ownership", test_image_construction_and_ownership},
        {"Image access and mutation", test_image_access_and_mutation},
        {"Image lifecycle, const, and view APIs", test_image_lifecycle_const_and_view_apis},
        {"ImageView external storage and access", test_image_view_external_storage_and_access},
        {"ImageView defaults, copies, and derived views", test_image_view_defaults_copy_and_derived_views},
        {"Zero extent, invalid stride, and overflow contracts", test_zero_extent_invalid_stride_and_overflow_contracts},
        {"Derived views and copy operations", test_views_and_copy_operations},
        {"Regions, blit, and conversion", test_regions_blit_and_conversion},
        {"Crop and blit overloads and failures", test_crop_and_blit_overloads_and_failures},
        {"Conversion policies and view sources", test_conversion_policies_and_view_sources},
        {"All supported scalar conversion pairs", test_all_supported_scalar_conversion_pairs},
    };

    std::size_t failures = 0;
    for (const TestCase& test : tests) {
        try {
            test.function();
            std::cout << "PASS: " << test.name << '\n';
        } catch (const std::exception& exception) {
            ++failures;
            std::cerr << "FAIL: " << test.name << ": " << exception.what() << '\n';
        }
    }
    return failures == 0 ? 0 : 1;
}