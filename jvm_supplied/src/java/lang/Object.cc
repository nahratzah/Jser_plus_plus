#include <java/lang/Object.h>

namespace java::lang {


Object::~Object() noexcept {}


} /* namespace java::lang */

namespace java::serialization {


auto type_def<java::lang::Object>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  return get_non_serializable_class(java_class_name());
}


} /* namespace java::serialization */
