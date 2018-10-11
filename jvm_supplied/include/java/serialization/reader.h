#ifndef JAVA_SERIALIZATION_READER_H
#define JAVA_SERIALIZATION_READER_H

#include <cstddef>
#include <cycle_ptr/cycle_ptr.h>
#include <java/inline.h>
#include <java/primitives.h>
#include <java/ref.h>
#include <java/serialization/decoder_fwd.h>
#include <java/serialization/encdec.h>
#include <java/lang/Object.h>

namespace java::serialization {

class annotation_reader {
 public:
  annotation_reader(class_decoder_intf& intf, stream::new_object::annotation_vector::const_iterator b, stream::new_object::annotation_vector::const_iterator e)
  : intf_(intf),
    iter_(std::move(b)),
    end_(std::move(e))
  {}

  explicit annotation_reader(class_decoder_intf& intf, const stream::new_object::annotation_vector& data)
  : annotation_reader(intf, data.begin(), data.end())
  {}

  auto read_boolean() -> ::java::boolean_t;
  auto read_byte() -> ::java::byte_t;
  auto read_short() -> ::java::short_t;
  auto read_int() -> ::java::int_t;
  auto read_long() -> ::java::long_t;
  auto read_float() -> ::java::float_t;
  auto read_double() -> ::java::double_t;
  auto read_char() -> ::java::char_t;
  auto at_end() -> bool;

  template<typename T>
  JSER_INLINE auto read_initial_object() -> T {
    return ::java::cast<T>(read_initial_object_());
  }

  template<typename T>
  JSER_INLINE auto read_comparable_object() -> T {
    return ::java::cast<T>(read_comparable_object_());
  }

  template<typename T>
  JSER_INLINE auto read_complete_object() -> T {
    return ::java::cast<T>(read_complete_object_());
  }

 private:
  auto read_object_() -> ::cycle_ptr::cycle_gptr<const stream::stream_element>;
  auto read_initial_object_() -> ::java::lang::Object;
  auto read_comparable_object_() -> ::java::lang::Object;
  auto read_complete_object_() -> ::java::lang::Object;
  auto get_bytes_(void* buf, ::std::size_t n) -> void;

  class_decoder_intf& intf_;
  stream::new_object::annotation_vector::const_iterator iter_;
  stream::new_object::annotation_vector::const_iterator end_;
  ::std::size_t off_ = 0;
};

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_READER_H */
