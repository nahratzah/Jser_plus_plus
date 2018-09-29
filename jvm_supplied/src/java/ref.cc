#include <java/ref.h>
#include <java/reflect.h>

namespace java {

auto ref_eq_(::cycle_ptr::cycle_gptr<const object_intf> x,
             ::cycle_ptr::cycle_gptr<const object_intf> y)
-> bool {
  return _equal_helper()(std::move(x), std::move(y)).ok();
}

} /* namespace java */
