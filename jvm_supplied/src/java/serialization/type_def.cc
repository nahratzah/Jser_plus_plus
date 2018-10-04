#include <java/serialization/type_def.h>

namespace java::serialization {


cycle_handler::cycle_handler() = default;
cycle_handler::~cycle_handler() noexcept = default;

auto cycle_handler::serializable_do_encode_(const java::_tags::java::io::Serializable::erased_type& s)
-> cycle_ptr::cycle_gptr<const stream::stream_element> {
  return s.do_encode_(*this);
}

auto cycle_handler::encode_str(std::string s)
-> cycle_ptr::cycle_gptr<const stream::stream_string> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  auto str_iter = string_table_.find(s);
  if (str_iter != string_table_.end())
    return str_iter->second;

  conversion_type conversion;
  auto new_str = ::cycle_ptr::make_cycle<stream_string>(conversion.from_bytes(s));
  return string_table_.emplace(std::move(s), std::move(new_str)).first->second;
}


} /* namespace java::serialization */
