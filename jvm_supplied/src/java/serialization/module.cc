#include <java/serialization/module.h>
#include <java/serialization/exception.h>
#include <cassert>
#include <functional>

namespace java::serialization {


module::module(::std::initializer_list<factory_map::value_type> init)
: factory_map_(std::move(init))
{}

module::~module() noexcept = default;

auto module::operator+=(const module& other) -> module& {
  factory_map_.insert(other.factory_map_.begin(), other.factory_map_.end());
  return *this;
}

auto module::operator+=(module&& other) -> module& {
#if __has_cpp_attribute(__cpp_lib_node_extract) >= 201606
  factory_map_.merge(std::move(other.factory_map_));
#else
  factory_map_.insert(other.factory_map_.begin(), other.factory_map_.end());
#endif
  return *this;
}

auto module::decoder(
    ::std::u16string_view class_name,
    ::java::serialization::decoder_ctx& ctx,
    ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> element) const
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  const auto& spec = spec_(class_name);
  if (spec.decoder == nullptr) {
    conversion_type conversion;
    throw ::java::serialization::decoding_error("class "
        + conversion.to_bytes(class_name.data(), class_name.data() + class_name.size())
        + " is not decodable");
  }

  ::cycle_ptr::cycle_gptr<::java::serialization::decoder> result =
      ::std::invoke(spec.decoder, ctx, std::move(element));
  assert(result != nullptr);
  return result;
}

auto module::get_class(::std::u16string_view class_name) const
-> ::java::lang::Class<::java::G::pack<>> {
  const auto& spec = spec_(class_name);
  assert(spec.cls != nullptr);

  return ::java::lang::Class<::java::G::pack<>>(
      ::java::_direct(),
      ::std::invoke(spec.cls));
}

auto module::spec_(::std::u16string_view class_name) const
-> const decoder_spec& {
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  const auto found = factory_map_.find(class_name);
  if (found == factory_map_.end()) {
    conversion_type conversion;
    throw ::java::serialization::decoding_error("class "
        + conversion.to_bytes(class_name.data(), class_name.data() + class_name.size())
        + " not found in module");
  }

  return found->second;
}

auto operator+(const module& x, const module& y) -> module {
  module result = x;
  result += y;
  return result;
}

auto operator+(module&& x, const module& y) -> module&& {
  x += y;
  return std::move(x);
}

auto operator+(const module& x, module&& y) -> module&& {
  y += x;
  return std::move(y);
}

auto operator+(module&& x, module&& y) -> module&& {
  x += std::move(y);
  return std::move(x);
}


} /* namespace java::serialization */
