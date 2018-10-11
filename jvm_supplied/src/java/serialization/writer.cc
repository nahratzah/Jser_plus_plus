#include <java/serialization/writer.h>
#include <cstring>
#include <cstdint>
#include <boost/endian/conversion.hpp>
#include <java/serialization/type_def.h>

namespace java::serialization {

auto annotation_writer::write_boolean(::java::boolean_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_byte(::java::byte_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_short(::java::short_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_int(::java::int_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_long(::java::long_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_float(::java::float_t v) -> void {
  ::std::uint32_t buf;
  static_assert(sizeof(buf) == sizeof(v));
  ::std::memcpy(&buf, &v, sizeof(v));

  ::boost::endian::native_to_big_inplace(buf);
  write_bytes_(&buf, sizeof(buf));
}

auto annotation_writer::write_double(::java::double_t v) -> void {
  ::std::uint64_t buf;
  static_assert(sizeof(buf) == sizeof(v));
  ::std::memcpy(&buf, &v, sizeof(v));

  ::boost::endian::native_to_big_inplace(buf);
  write_bytes_(&buf, sizeof(buf));
}

auto annotation_writer::write_char(::java::char_t v) -> void {
  ::boost::endian::native_to_big_inplace(v);
  write_bytes_(&v, sizeof(v));
}

auto annotation_writer::write_object(::java::lang::Object v) -> void {
  data_.emplace_back(handler_.encode_field(v));
}

auto annotation_writer::write_object_unshared(::java::lang::Object v) -> void {
  data_.emplace_back(handler_.encode_field_unshared(v));
}

auto annotation_writer::write_bytes_(const void* b, ::std::size_t n) -> void {
  if (data_.empty())
    data_.emplace_back(::std::in_place_type<::java::serialization::stream::blockdata>);

  const ::std::uint8_t*const iter_b = reinterpret_cast<const ::std::uint8_t*>(b);
  const ::std::uint8_t*const iter_e = iter_b + n;
  ::std::visit(
      [&](auto& elem) {
        bool append;
        if constexpr(std::is_same_v<::java::serialization::stream::blockdata, std::decay_t<decltype(elem)>>) {
          append = (!elem.empty() && elem.size() + n > MAX_BLOCKSIZE);
          if (!append)
            elem.insert(elem.end(), iter_b, iter_e);
        } else {
          append = true;
        }

        if (append)
          data_.emplace_back(::std::in_place_type<::java::serialization::stream::blockdata>, iter_b, iter_e);
      },
      data_.back());
}

// Same as in java, roll over to a new block every 1 kb.
const ::std::size_t annotation_writer::MAX_BLOCKSIZE = 1024;

} /* namespace java::serialization */
