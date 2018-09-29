#include <java/object_intf.h>
#include <java/hash.h>

namespace java {


object_intf::~object_intf() noexcept = default;

auto object_intf::__hash_code__(bool specialized, std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();
  if (specialized) return nonce;

  return hash_combiner(max_cascade)
      << nonce
      << this;
}


} /* namespace java */

namespace std {

auto hash<::java::object_intf*>::operator()(const ::java::object_intf* ptr) const noexcept
-> std::size_t {
  static const std::size_t null_nonce = ::java::__hash_nonce();
  return (ptr == nullptr ? null_nonce : ptr->__hash_code__(false, 128));
}

} /* namespace std */
