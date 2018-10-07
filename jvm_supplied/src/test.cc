#include <java/util/Collections.h>
#include <java/util/Set.h>
#include <java/util/HashSet.h>
#include <java/lang/String.h>
#include <java/lang/Class.h>
#include <java/generics.h>
#include <java/reflect.h>
#include <java/generics.h>
#include <vector>

auto foobar() -> java::util::Set<java::lang::String> {
  java::util::Set<java::lang::String> q = java::util::HashSet<java::lang::String>(::java::allocate);
  return java::util::Collections$CheckedSet<java::lang::String>(::java::allocate, q, java::get_class<java::lang::String>());
}

auto foobar_types() -> std::vector<::java::lang::Class<::java::G::pack<>>> {
  std::vector<::java::lang::Class<::java::G::pack<>>> result;

  result.push_back(::java::get_class<::java::byte_t>());
  result.push_back(::java::get_class<::java::array_type<::java::byte_t>>());
  result.push_back(::java::get_class<::java::array_type<::java::byte_t, 5>>());

  result.push_back(::java::get_class<::java::lang::Object>());
  result.push_back(::java::get_class<::java::array_type<::java::lang::Object>>());
  result.push_back(::java::get_class<::java::array_type<::java::lang::Object, 5>>());

  return result;
}
