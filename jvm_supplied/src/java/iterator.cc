#include <java/iterator.h>
#include <memory>

namespace java {

forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface::~interface() noexcept = default;


forward_iterator<type_of_t<java::lang::Object>>::interface::~interface() noexcept = default;

auto forward_iterator<type_of_t<java::lang::Object>>::interface::cderef() const
-> const_ref<java::lang::Object> {
  return deref();
}


bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::interface::~interface() noexcept = default;


bidirectional_iterator<type_of_t<java::lang::Object>>::interface::~interface() noexcept = default;

} /* namespace java */
