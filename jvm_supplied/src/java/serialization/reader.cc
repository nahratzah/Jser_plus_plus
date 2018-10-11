#include <java/serialization/reader.h>
#include <cstdint>
#include <cstring>
#include <variant>
#include <boost/endian/conversion.hpp>
#include <java/serialization/decoder.h>

namespace java::serialization {

auto annotation_reader::read_boolean()
-> ::java::boolean_t {
  ::std::uint8_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);
  return buf != 0u;
}

auto annotation_reader::read_byte()
-> ::java::byte_t {
  ::java::byte_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);
  return buf;
}

auto annotation_reader::read_short()
-> ::java::short_t {
  ::java::short_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);
  return buf;
}

auto annotation_reader::read_int()
-> ::java::int_t {
  ::java::int_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);
  return buf;
}

auto annotation_reader::read_long()
-> ::java::long_t {
  ::java::long_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);
  return buf;
}

auto annotation_reader::read_float()
-> ::java::float_t {
  ::std::int32_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);

  ::java::float_t v;
  static_assert(sizeof(v) == sizeof(buf));
  ::std::memcpy(&v, &buf, sizeof(v));
  return v;
}

auto annotation_reader::read_double()
-> ::java::double_t {
  ::std::uint64_t buf;
  get_bytes_(&buf, sizeof(buf));
  ::boost::endian::big_to_native_inplace(buf);

  ::java::double_t v;
  static_assert(sizeof(v) == sizeof(buf));
  ::std::memcpy(&v, &buf, sizeof(v));
  return v;
}

auto annotation_reader::at_end()
-> bool {
  ::cycle_ptr::cycle_gptr<stream::stream_element> result;

  while (iter_ != end_) {
    const bool at_end_of_elem = ::std::visit(
        [&](const auto& elem) -> bool {
          if constexpr(std::is_same_v<stream::blockdata, std::decay_t<decltype(elem)>>) {
            if (off_ != elem.size())
              return false;
            ++iter_;
            off_ = 0;
            return true;
          } else {
            return false;
          }
        },
        *iter_);
    if (!at_end_of_elem) return false;
  }

  return true;
}

auto annotation_reader::read_object_()
-> ::cycle_ptr::cycle_gptr<const stream::stream_element> {
  ::cycle_ptr::cycle_gptr<const stream::stream_element> result;

  do {
    if (iter_ == end_)
      throw decoding_error("expected object, encountered annotation end");

    ::std::visit(
        [&](const auto& elem) -> void {
          if constexpr(std::is_same_v<stream::blockdata, std::decay_t<decltype(elem)>>) {
            if (off_ != elem.size())
              throw decoding_error("expected object, encountered binary data");
            ++iter_;
            off_ = 0;
          } else {
            result = elem;
            ++iter_;
          }
        },
        *iter_);
  } while (result == nullptr);

  return result;
}

auto annotation_reader::read_initial_object_()
-> ::java::lang::Object {
  return intf_.initial_field(read_object_());
}

auto annotation_reader::read_comparable_object_()
-> ::java::lang::Object {
  return intf_.comparable_field(read_object_());
}

auto annotation_reader::read_complete_object_()
-> ::java::lang::Object {
  return intf_.complete_field(read_object_());
}

auto annotation_reader::get_bytes_(void* buf, ::std::size_t n)
-> void {
  stream::new_object::annotation_vector::const_iterator iter = iter_;
  ::std::size_t off = off_;

  while (n > 0) {
    if (iter == end_)
      throw decoding_error("expected binary data, encountered annotation end");

    ::std::visit(
        [&](const auto& elem) -> void {
          if constexpr(!std::is_same_v<stream::blockdata, std::decay_t<decltype(elem)>>) {
            throw decoding_error("expected binary data, but encountered object");
          } else {
            assert(off <= elem.size());
            if (off == elem.size()) {
              ++iter;
              off = 0;
            } else {
              ::std::size_t rlen = ::std::min(n, elem.size() - off);
              ::std::memcpy(buf, elem.data() + off, rlen);
              buf = reinterpret_cast<::std::uint8_t*>(buf) + rlen;
              n -= rlen;
            }
          }
        },
        *iter);
  }

  iter_ = iter;
  off_ = off;
}

} /* namespace java::serialization */
