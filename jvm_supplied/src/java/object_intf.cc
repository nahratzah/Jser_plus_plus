#include <java/object_intf.h>
#include <java/hash.h>
#include <java/lang/Object.h>

namespace java {


object_intf::~object_intf() noexcept = default;

auto object_intf::__get_class__() const
-> cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return dynamic_cast<const ::java::_erased::java::lang::Object&>(*this).__get_class__();
}

auto object_intf::__hash_code__(bool specialized, std::size_t max_cascade) const noexcept
-> std::size_t {
  return dynamic_cast<const ::java::_erased::java::lang::Object&>(*this).__hash_code__(specialized, max_cascade);
}

auto object_intf::__equal__(bool specialized, _equal_helper& eq, const object_intf& other) const
-> void {
  return dynamic_cast<const ::java::_erased::java::lang::Object&>(*this).__equal__(specialized, eq, other);
}


} /* namespace java */

namespace std {

auto hash<::java::object_intf*>::operator()(const ::java::object_intf* ptr) const noexcept
-> std::size_t {
  static const std::size_t null_nonce = ::java::__hash_nonce();
  return (ptr == nullptr ? null_nonce : ptr->__hash_code__(false, 128));
}

} /* namespace std */
