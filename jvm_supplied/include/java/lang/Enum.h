#ifndef JAVA_LANG_ENUM_H
#define JAVA_LANG_ENUM_H

#include <utility>
#include <string>
#include <string_view>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <cycle_ptr/cycle_ptr.h>
#include <java/serialization/type_def.h>
#include <java/serialization/encdec.h>
#include <java/lang/Object.h>

namespace java::lang {
template<typename = Object> class Enum;
} /* namespace java::lang */

namespace java::serialization {


template<>
struct template_type_def<java::lang::Enum> {
  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"java.lang.Enum"sv;
  }

  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto get_specialized_class(std::u16string_view name)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;

  static auto encode(const java::lang::Enum<>& v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_enum>;

  template<typename E>
  static auto encode(const java::lang::Enum<E>& v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_enum>;
};


} /* namespace java::serialization */

namespace java::lang {

template<typename E>
class Enum final
: public Object
{
  static_assert(std::is_enum_v<E>,
      "Enum template argument must be an enum, or void.");

 public:
  Enum() = default;

  Enum(E e)
  noexcept
  : value(e)
  {}

  operator E() const noexcept {
    return value;
  }

  E value;

 private:
  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override {
    return java::serialization::type_def<Enum>::get_class();
  }
};

template<>
class Enum<Object> final
: public Object
{
  friend struct java::serialization::template_type_def<java::lang::Enum>;

 public:
  Enum() = default; // Creates an invalid enum.

  Enum(std::u16string cls_name, std::u16string value)
  : cls_name_(std::move(cls_name)),
    value_(std::move(value))
  {}

  template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
  explicit Enum(const Enum<E>& e)
#if __cplusplus >= 201703 // Use string_view to string conversion.
  : Enum(
      java::serialization::type_def<Enum<E>>::java_class_name(),
      java::serialization::type_def<Enum<E>>::val_to_str(e))
#else // Before C++17, string_view to string was unimplemented.
  : Enum(
      sv2s_(java::serialization::type_def<Enum<E>>::java_class_name()),
      sv2s_(java::serialization::type_def<Enum<E>>::val_to_str(e)))
#endif
  {}

  template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
  auto operator=(const Enum<E>& e) -> Enum& {
    return *this = Enum(e);
  }

  auto operator==(const Enum& y) const
  noexcept
  -> bool {
    return cls_name_ == y.cls_name_ && value_ == y.value_;
  }

  template<typename E>
  auto operator==(const Enum<E>& y) const
  noexcept
  -> std::enable_if_t<std::is_enum_v<E>, bool> {
    return cls_name_ == java::serialization::type_def<Enum<E>>::java_class_name()
        && value_ == java::serialization::type_def<Enum<E>>::val_to_str(y);
  }

  template<typename E>
  operator Enum<E>() const {
    using defn = java::serialization::type_def<E>;

    if (cls_name_ != defn::java_class_name())
      throw std::invalid_argument("Mismatching enum class");
    return defn::str_to_val(value_);
  }

 private:
#if __cplusplus < 201703
  static inline auto sv2s_(std::u16string_view s)
  -> std::u16string {
    return std::u16string(s.data(), s.size());
  }
#endif

  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override {
    return java::serialization::type_def<Enum>::get_class();
  }

  std::u16string cls_name_;
  std::u16string value_;
};


template<typename E>
constexpr auto operator==(const Enum<E>& x, const Enum<E>& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  E x_val = x, y_val = y;
  return x_val == y_val;
}

template<typename E>
constexpr auto operator==(const E& x, const Enum<E>& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  E x_val = x, y_val = y;
  return x_val == y_val;
}

template<typename E>
constexpr auto operator==(const Enum<E>& x, const E& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  E x_val = x, y_val = y;
  return x_val == y_val;
}

template<typename E>
constexpr auto operator!=(const Enum<E>& x, const Enum<E>& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  return !(x == y);
}

template<typename E>
constexpr auto operator!=(const E& x, const Enum<E>& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  return !(x == y);
}

template<typename E>
constexpr auto operator!=(const Enum<E>& x, const E& y) noexcept
-> std::enable_if_t<std::is_enum_v<E>, bool> {
  return !(x == y);
}


template<typename E>
auto operator==(const Enum<E>& x, const Enum<>& y) noexcept
-> bool {
  return y == x; // Invokes class-member operator==.
}

inline auto operator!=(const Enum<>& x, const Enum<>& y) noexcept
-> bool {
  return !(x == y);
}

template<typename E>
auto operator!=(const Enum<>& x, const Enum<E>& y) noexcept
-> bool {
  return !(x == y);
}

template<typename E>
auto operator!=(const Enum<E>& x, const Enum<>& y) noexcept
-> bool {
  return !(x == y);
}


} /* namespace java::lang */

namespace java::serialization {


inline auto template_type_def<java::lang::Enum>::encode(const java::lang::Enum<>& v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_enum> {
  using namespace java::serialization::stream;

  const auto result = cycle_ptr::make_cycle<new_enum>();
  result->type = get_specialized_class(v.cls_name_);
  result->value = cycle_ptr::make_cycle<stream_string>(v.value_);
  return result;
}

template<typename E>
auto template_type_def<java::lang::Enum>::encode(const java::lang::Enum<E>& v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_enum> {
  using namespace java::serialization::stream;
  using defn = type_def<E>;

  const auto result = cycle_ptr::make_cycle<new_enum>();
  result->type = get_specialized_class(defn::java_class_name());
  result->value = cycle_ptr::make_cycle<stream_string>(defn::val_to_str(v));
  return result;
}


} /* namespace java::serialization */

#endif /* JAVA_LANG_ENUM_H */
