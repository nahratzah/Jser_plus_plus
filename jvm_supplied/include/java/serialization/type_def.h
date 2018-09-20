#ifndef JAVA_SERIALIZATION_TYPE_DEF_H
#define JAVA_SERIALIZATION_TYPE_DEF_H

#include <java/serialization/type_def_fwd.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <cycle_ptr/cycle_ptr.h>
#include <java/serialization/encdec.h>
#include <java/fwd/java/io/Serializable.h>
#include <java/ref.h>
#include <java/inline.h>

namespace java::serialization {


template<template<typename...> class Tmpl>
struct template_type_def;

template<typename> struct type_def;

template<template<typename...> class Tmpl, typename... T>
struct type_def<Tmpl<T...>>
: template_type_def<Tmpl>
{};


class cycle_handler {
 private:
  using visit_done = std::unordered_map<
      cycle_ptr::cycle_gptr<const void>,
      cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element>,
      std::hash<cycle_ptr::cycle_gptr<const void>>,
      std::equal_to<cycle_ptr::cycle_gptr<const void>>,
      std::allocator<std::pair<const cycle_ptr::cycle_gptr<const void>, cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element>>>>;

  template<typename Defn, typename T>
  using select_defn_t = std::conditional_t<
      std::is_void_v<Defn>,
      std::conditional_t<
          (std::is_polymorphic_v<std::remove_cv_t<std::remove_reference_t<T>>>
           && !std::is_final_v<std::remove_cv_t<std::remove_reference_t<T>>>),
          void,
          type_def<std::remove_cv_t<std::remove_reference_t<T>>>>, // When final, just use T directly.
      std::conditional_t<
          (std::is_polymorphic_v<std::remove_cv_t<std::remove_reference_t<T>>>
           && std::is_base_of_v<Defn, std::remove_cv_t<std::remove_reference_t<T>>>),
          void,
          type_def<Defn>>>; // Cross-type conversion, there shall be a specialized encode() method in Defn for T.

  template<typename SelectedDefn, typename T>
  struct encode_element_for_defn_ {
    using type = std::remove_cv_t<typename std::decay_t<decltype(SelectedDefn::encode(std::declval<const T&>()))>::element_type>;
  };

  template<typename T>
  struct encode_element_for_void_ {
    using type = java::serialization::stream::stream_element;
  };

  template<typename Defn, typename T>
  using encode_element_t = typename std::conditional_t<
      std::is_void_v<select_defn_t<Defn, T>>,
      encode_element_for_void_<T>,
      encode_element_for_defn_<select_defn_t<Defn, T>, T>>::type;

  template<typename Defn, typename T>
  using encode_result_t = cycle_ptr::cycle_gptr<std::add_const_t<encode_element_t<Defn, T>>>;

  auto serializable_do_encode_(const java::_tags::java::io::Serializable::erased_type& s)
  -> cycle_ptr::cycle_gptr<const stream::stream_element>;

  JSER_INLINE auto serializable_do_encode_(java::io::Serializable s)
  -> cycle_ptr::cycle_gptr<const stream::stream_element>;

 public:
  template<typename Defn = void, typename T>
  auto encode_field_unshared(const T& v)
  -> encode_result_t<
      Defn,
      std::enable_if_t<
          !std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>,
          T>>;

  template<typename Defn = void, typename T>
  auto encode_field_unshared(const cycle_ptr::cycle_gptr<T>& v)
  -> encode_result_t<Defn, T> {
    return (v == nullptr ? nullptr : encode_field_unshared<Defn>(*v));
  }

  template<typename Defn = void, typename T>
  auto encode_field_unshared(const cycle_ptr::cycle_member_ptr<T>& v)
  -> encode_result_t<Defn, T> {
    return (v == nullptr ? nullptr : encode_field_unshared<Defn>(*v));
  }

  template<typename Defn = void, typename T>
  auto encode_field(const T& v)
  -> encode_result_t<
      Defn,
      std::enable_if_t<
          !std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>,
          T>> {
    return this->encode_field_unshared<Defn>(v);
  }

  template<typename Defn = void, typename T>
  auto encode_field(const T& v)
  -> encode_result_t<Defn,
      typename std::enable_if_t<
          std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>,
          T>::element_type>;

  template<typename Defn = void, typename Iter>
  auto encode_obj_vector(Iter b, Iter e)
  -> std::vector<cycle_ptr::cycle_gptr<const stream::stream_element>> {
    std::vector<cycle_ptr::cycle_gptr<const stream::stream_element>> result;
    std::transform(b, e, std::back_inserter(result),
        [this](const auto& x) { return encode_field<Defn>(x); });
    return result;
  }

 private:
  auto register_value(typename visit_done::key_type addr, typename visit_done::mapped_type result)
  -> void {
    assert(addr != nullptr && result != nullptr);
    bool emplace_success;
    std::tie(std::ignore, emplace_success) = visit_done_.emplace(std::move(addr), std::move(result));
    if (!emplace_success) throw std::logic_error("cycle_handler already contains key");
  }

  visit_done visit_done_;
};


auto get_non_serializable_class(std::u16string_view class_name)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;


} /* namespace java::serialization */

#include <java/io/Serializable.h>

namespace java::serialization {


template<typename Defn, typename T>
auto cycle_handler::encode_field_unshared(const T& v)
-> encode_result_t<
    Defn,
    std::enable_if_t<
        !std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>,
        T>> {
  using defn = select_defn_t<Defn, T>;

  if constexpr(std::is_void_v<defn>) {
    return serializable_do_encode_(::java::cast<java::io::Serializable>(v));
  } else {
    return defn::encode(v);
  }
}

template<typename Defn, typename T>
auto cycle_handler::encode_field(const T& v)
-> encode_result_t<Defn,
    typename std::enable_if_t<
        std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>,
        T>::element_type> {
  using defn = select_defn_t<Defn, typename T::element_type>;
  using element_t = encode_element_t<Defn, typename T::element_type>;

  static_assert(std::is_base_of_v<java::serialization::stream::stream_element, element_t>,
      "Must be invoked for a stream_element derived type.");

  if (v == nullptr) return nullptr;

  auto vdone = visit_done_.find(v);
  if (vdone != visit_done_.end()) {
    // If already present, bypass encoding.
    auto result = std::dynamic_pointer_cast<const element_t>(vdone->second);
    if (result == nullptr) throw std::bad_cast();
    return result;
  }

  if constexpr(std::is_void_v<defn>) {
    return serializable_do_encode_(::java::cast<java::io::Serializable>(*v));
  } else {
    // Allocate result into separate pointer, to keep it mutable.
    auto result = cycle_ptr::make_cycle<element_t>();
    // Register result, so that future invocations will bypass the encoder.
    // (This is how recursion is broken.)
    register_value(v, result);

    // Perform encoding operation.
    defn::encode(*v, result, *this);

    return result;
  }
}

JSER_INLINE auto cycle_handler::serializable_do_encode_(java::io::Serializable s)
-> cycle_ptr::cycle_gptr<const stream::stream_element> {
  return serializable_do_encode_(*::java::raw_ptr<::java::_tags::java::io::Serializable>(s));
}


} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_TYPE_DEF_H */
