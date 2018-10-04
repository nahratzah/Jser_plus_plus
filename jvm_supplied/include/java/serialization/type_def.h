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
  using class_descs = std::unordered_map<
      std::string,
      cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>>;
  using string_table = std::unordered_map<
      std::string,
      cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_string>>;

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

  JSER_INLINE auto serializable_do_encode_(::java::const_ref<::java::io::Serializable> s)
  -> cycle_ptr::cycle_gptr<const stream::stream_element>;

 public:
  cycle_handler();
  ~cycle_handler() noexcept;

  template<typename T>
  auto encode_field_unshared(const T& v)
  -> cycle_ptr::cycle_gptr<const stream::stream_element> {
    return cycle_handler().template encode_field<T>(v);
  }

  template<typename T>
  auto encode_field(const T& v)
  -> cycle_ptr::cycle_gptr<const stream::stream_element>;

  template<typename Fn>
  auto encode_class_desc(std::string name, Fn&& fn)
  -> cycle_ptr::cycle_gptr<const stream::new_class_desc__class_desc> {
    const auto cd_iter = class_descs_.find(name);
    if (cd_iter != class_descs_.end())
      return cd_iter->second;

    return class_descs_.emplace(std::move(name), std::invoke(std::forward<Fn>(fn), *this)).first->second;
  }

  template<typename ElementType, typename T, typename Fn>
  auto encode_obj(cycle_ptr::cycle_gptr<const T> obj, Fn&& fn)
  -> cycle_ptr::cycle_gptr<const stream::stream_element> {
    assert(obj != nullptr);

    const auto new_obj = cycle_ptr::make_cycle<ElementType>();
    const auto ins = visit_done_.emplace(std::move(obj), new_obj);

    if (!ins.second) {
      try {
        std::invoke(std::forward<Fn>(fn), *this, *new_obj);
      } catch (...) {
        visit_done_.erase(ins.first);
        throw;
      }
    }
    return ins.first->second;
  }

  auto encode_str(std::string s)
  -> cycle_ptr::cycle_gptr<const stream::stream_string>;

 private:
  visit_done visit_done_;
  class_descs class_descs_;
  string_table string_table_;
};


} /* namespace java::serialization */

#include <java/io/Serializable.h>

namespace java::serialization {


template<typename T>
auto cycle_handler::encode_field(const T& v)
-> cycle_ptr::cycle_gptr<const stream::stream_element> {
  if (v == nullptr) return nullptr;

  return serializable_do_encode_(::java::cast<::java::const_ref<java::io::Serializable>>(v));
}

JSER_INLINE auto cycle_handler::serializable_do_encode_(::java::const_ref<::java::io::Serializable> s)
-> cycle_ptr::cycle_gptr<const stream::stream_element> {
  return serializable_do_encode_(*::java::raw_ptr<::java::_tags::java::io::Serializable>(s));
}


} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_TYPE_DEF_H */
