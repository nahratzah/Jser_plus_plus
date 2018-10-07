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
