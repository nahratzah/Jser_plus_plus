#include <java/primitives.h>
#include <java/lang/Boolean.h>
#include <java/lang/Byte.h>
#include <java/lang/Short.h>
#include <java/lang/Integer.h>
#include <java/lang/Long.h>
#include <java/lang/Float.h>
#include <java/lang/Double.h>
#include <java/lang/Character.h>

namespace java::serialization {


auto type_def<java::boolean_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::boolean_t>::encode(java::boolean_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Boolean>::encode(v);
}

auto type_def<java::byte_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::byte_t>::encode(java::byte_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Byte>::encode(v);
}

auto type_def<java::char_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::char_t>::encode(java::char_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Character>::encode(v);
}

auto type_def<java::double_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::double_t>::encode(java::double_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Double>::encode(v);
}

auto type_def<java::float_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::float_t>::encode(java::float_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Float>::encode(v);
}

auto type_def<java::int_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::int_t>::encode(java::int_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Integer>::encode(v);
}

auto type_def<java::long_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::long_t>::encode(java::long_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Long>::encode(v);
}

auto type_def<java::short_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}

auto type_def<java::short_t>::encode(java::short_t v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  return type_def<java::lang::Short>::encode(v);
}

auto type_def<java::void_t>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}


} /* namespace java::serialization */
