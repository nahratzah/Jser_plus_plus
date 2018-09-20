#include <java/serialization/type_def.h>

namespace java::serialization {


auto get_non_serializable_class(std::u16string_view class_name)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  using namespace java::serialization::stream;

  return cycle_ptr::make_cycle<new_class_desc__class_desc>(
      field_descriptor(class_name),
      0);
}

auto cycle_handler::serializable_do_encode_(const java::_tags::java::io::Serializable::erased_type& s)
-> cycle_ptr::cycle_gptr<const stream::stream_element> {
  return s.do_encode_(*this);
}


} /* namespace java::serialization */
