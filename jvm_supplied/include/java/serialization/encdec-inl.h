#include <boost/endian/conversion.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/io/ios_state.hpp>
#include <java/serialization/exception.h>
#include <java/serialization/string.h>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <string>
#include <string_view>
#include <stack>
#include <codecvt>
#include <ostream>

namespace java::serialization::stream {


template<typename T>
auto indent_(const T& elem, std::size_t n) {
  std::string s;
  if constexpr(std::is_same_v<std::string, T>)
    s = elem;
  else
    s = to_string(elem);

  if (n > 0) {
    for (auto pos = s.find('\n');
        pos != std::string::npos;
        pos = s.find('\n', pos + 1u + n))
      s.insert(pos + 1u, n, ' ');
  }
  return s;
}


inline auto eq_ctx::hasher::operator()(const std::tuple<const void*, const void*>& v) const
noexcept
-> std::size_t {
  std::hash<const void*> vptr_hash;
  return (vptr_hash(std::get<0>(v)) * 73u) + vptr_hash(std::get<1>(v));
}

inline auto eq_ctx::operator()(
    const stream_element& x,
    const stream_element& y)
-> bool {
  using std::swap;

  if (typeid(x) != typeid(y)) return false;

  std::tuple<const void*, const void*> ptrs = { &x, &y };
  if (std::get<1>(ptrs) < std::get<0>(ptrs))
    swap(std::get<0>(ptrs), std::get<1>(ptrs));

  const auto insertion = recurse_.insert(std::move(ptrs));
  const bool absent = std::get<1>(insertion);
  const auto& iter = std::get<0>(insertion);
  if (!absent || x.eq_(y, *this)) return true;

  recurse_.erase(iter);
  return false;
}

template<typename T>
auto eq_ctx::operator()(const T& x, const T& y)
-> std::enable_if_t<!std::is_base_of_v<stream_element, T>, bool> {
  return x.eq_(y, *this);
}


inline auto operator<<(std::ostream& out, const field_descriptor& fd)
-> std::ostream& {
  return out << to_string(fd);
}

inline auto to_string(const field_descriptor& fd)
-> std::string {
  using conversion_type = std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  conversion_type conversion;
  return conversion.to_bytes(fd.to_string());
}


inline auto reader::read_(basic_read_wrapper& read)
-> content_type {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  // Decode and validate header.
  if (!header_consumed_) {
    std::uint16_t magic;
    std::uint16_t version;

    read({ buffer(&magic, sizeof(magic)), buffer(&version, sizeof(version)) });
    big_to_native_inplace(magic);
    big_to_native_inplace(version);

    if (magic != MAGIC)
      throw decoding_error("invalid stream header");
    if (version != VERSION)
      throw decoding_error("invalid stream version");
    header_consumed_ = true;
  }

  bool is_exception = false;
  for (;;) {
    std::uint8_t hdr;
    for (;;) {
      read(buffer(&hdr, sizeof(hdr)));
      big_to_native_inplace(hdr);

      if (hdr != TC_RESET) break;
      referents_.clear();
    }

    try {
      auto decode_result = content::decode(*this, read, hdr);
      if (is_exception) referents_.clear();
      try {
        return std::visit(
            [is_exception](auto&& c) {
              return content_type(std::forward<decltype(c)>(c), is_exception);
            },
            std::move(decode_result));
      } catch (const std::invalid_argument& e) {
        throw decoding_error(e.what());
      }
    } catch (const encountered_tc_exception&) {
      // Flag as exception...
      is_exception = true;
      referents_.clear();
      // ... and restart.
    }
  }
}

inline auto reader::next_handle(cycle_ptr::cycle_gptr<const stream_element> v)
-> std::uint32_t {
  if (v == nullptr)
    throw std::invalid_argument("attempted to register nullptr handle");

  std::uint32_t seq = referents_.size() + base_wire_handle;
  referents_.push_back(std::move(v));
  assert(referents_.back() != nullptr);
  return seq;
}

template<typename T>
auto reader::get_handle(std::uint32_t idx) const
-> cycle_ptr::cycle_gptr<const T> {
  if (idx < base_wire_handle)
    throw decoding_error("invalid handle");
  idx -= base_wire_handle;
  if (idx >= referents_.size())
    throw decoding_error("invalid handle");

  cycle_ptr::cycle_gptr<const T> result = std::dynamic_pointer_cast<const T>(referents_[idx]);
  if (result != nullptr) return result;
  assert(referents_[idx] != nullptr);
  throw decoding_error("invalid handle type");
}


} /* namespace java::serialization::stream */
