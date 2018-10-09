#ifndef JAVA_SERIALIZATION_MODULE_H
#define JAVA_SERIALIZATION_MODULE_H

#include <java/serialization/module_fwd.h>
#include <initializer_list>
#include <string_view>
#include <unordered_map>
#include <cycle_ptr/cycle_ptr.h>
#include <java/generics.h>
#include <java/lang/Class.h>
#include <java/serialization/decoder_fwd.h>

namespace java::serialization {

class module {
 public:
  using decoder_factory = ::cycle_ptr::cycle_gptr<::java::serialization::decoder> (*)(
      ::java::serialization::decoder_ctx&,
      ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element>);
  using class_factory = ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>(*)();

  struct decoder_spec {
    class_factory cls;
    decoder_factory decoder;
  };

 private:
  using factory_map = std::unordered_map<::std::u16string_view, decoder_spec>;

 public:
  module() = default;
  module(::std::initializer_list<factory_map::value_type> init);
  ~module() noexcept;

  auto operator+=(const module& other) -> module&;
  auto operator+=(module&& other) -> module&;

  auto decoder(
      ::std::u16string_view class_name,
      ::java::serialization::decoder_ctx& ctx,
      ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> element) const
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto get_class(::std::u16string_view class_name) const
  -> ::java::lang::Class<::java::G::pack<>>;

  ///\brief Tiny forwarder to extract the __class__ function for a given type.
  ///\tparam T the type for which to find the __class__ function.
  template<typename T>
  static auto __class__() noexcept
  -> class_factory {
    return &T::__class__;
  }

  ///\brief Tiny forwarder to extract the __decoder__ function for a given type.
  ///\tparam T the type for which to find the __decoder__ function.
  template<typename T>
  static auto __decoder__() noexcept
  -> decoder_factory {
    return &T::__decoder__;
  }

 private:
  auto spec_(::std::u16string_view class_name) const
  -> const decoder_spec&;

  factory_map factory_map_;
};

auto operator+(const module& x, const module& y) -> module;
auto operator+(module&& x, const module& y) -> module&&;
auto operator+(const module& x, module&& y) -> module&&;
auto operator+(module&& x, module&& y) -> module&&;

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_MODULE_H */
