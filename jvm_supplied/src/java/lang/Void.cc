#include <java/lang/Void.h>

namespace java::serialization {


auto type_def<java::lang::Void>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}


} /* namespace java::serialization */
