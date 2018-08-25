#include <java/lang/Class.h>

namespace java::serialization {


auto template_type_def<java::lang::Class>::get_class()
-> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> {
  static const cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> cls = get_class_();
  return cls;
}

auto template_type_def<java::lang::Class>::get_class_()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  using namespace std::string_view_literals;
  using namespace java::serialization::stream;

  const auto result = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  result->class_name = field_descriptor(java_class_name());
  result->serial_version_uid = 0x2c7e5503d9bf9553ULL;
  result->info.flags = SC_SERIALIZABLE;
  result->info.fields = {};
  result->info.annotation = {};
  result->info.super = nullptr;
  return result;
}



} /* namespace java::serialization */

