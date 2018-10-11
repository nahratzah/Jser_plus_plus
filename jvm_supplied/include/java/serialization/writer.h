#ifndef JAVA_SERIALIZATION_WRITER_H
#define JAVA_SERIALIZATION_WRITER_H

#include <cstddef>
#include <variant>
#include <vector>
#include <cycle_ptr/cycle_ptr.h>
#include <java/primitives.h>
#include <java/serialization/type_def_fwd.h>
#include <java/serialization/encdec.h>
#include <java/lang/Object.h>

namespace java::serialization {

class annotation_writer {
 public:
  using vector = ::std::vector<::std::variant<::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element>, ::java::serialization::stream::blockdata>>;

  explicit annotation_writer(cycle_handler& handler)
  : handler_(handler)
  {}

  auto write_boolean(::java::boolean_t v) -> void;
  auto write_byte(::java::byte_t v) -> void;
  auto write_short(::java::short_t v) -> void;
  auto write_int(::java::int_t v) -> void;
  auto write_long(::java::long_t v) -> void;
  auto write_float(::java::float_t v) -> void;
  auto write_double(::java::double_t v) -> void;
  auto write_char(::java::char_t v) -> void;

  auto write_object(::java::lang::Object v) -> void;
  auto write_object_unshared(::java::lang::Object v) -> void;

  auto data() const &
  -> const vector& {
    return data_;
  }

  auto data() &&
  -> vector&& {
    return std::move(data_);
  }

 private:
  auto write_bytes_(const void* b, ::std::size_t n) -> void;

  static const ::std::size_t MAX_BLOCKSIZE;
  cycle_handler& handler_;
  vector data_;
};

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_WRITER_H */
