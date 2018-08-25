#ifndef JAVA_PRIMITIVES_H
#define JAVA_PRIMITIVES_H

#include <cstdint>
#include <string_view>
#include <cycle_ptr/cycle_ptr.h>
#include <java/serialization/type_def.h>

namespace java {


using boolean_t = bool;
using byte_t = std::int8_t;
using short_t = std::int16_t;
using int_t = std::int32_t;
using long_t = std::int64_t;
using float_t = float;
using double_t = double;
using char_t = char16_t;
using void_t = void;

static_assert(sizeof(char_t) == 2, "Char16_t size incompatible with java.");
static_assert(sizeof(float_t) == 4, "Float size incompatible with java.");
static_assert(sizeof(double_t) == 8, "Double size incompatible with java.");


} /* namespace java */

namespace java::serialization {


template<>
struct type_def<java::boolean_t> {
  using type = java::boolean_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"boolean"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::boolean_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::byte_t> {
  using type = java::byte_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"byte"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::byte_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::char_t> {
  using type = java::char_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"char"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::char_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::double_t> {
  using type = java::double_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"double"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::double_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::float_t> {
  using type = java::float_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"float"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::float_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::int_t> {
  using type = java::int_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"int"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::int_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::long_t> {
  using type = java::long_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"long"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::long_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::short_t> {
  using type = java::short_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"short"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(java::short_t v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;
};

template<>
struct type_def<java::void_t> {
  using type = java::void_t;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"void"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;
};


} /* namespace java::serialization */

#endif /* JAVA_PRIMITIVES_H */
