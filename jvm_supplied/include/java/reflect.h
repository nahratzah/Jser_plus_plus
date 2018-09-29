#ifndef JAVA_REFLECT_H
#define JAVA_REFLECT_H

#include <java/inline.h>
#include <java/null_error.h>
#include <java/object_intf.h>
#include <java/ref.h>
#include <java/type_traits.h>
#include <java/lang/Class.h>

namespace java {

///\brief Helper for get_class.
///\details This class simply exists, so that object_intf can declare it friend.
class _reflect_ops {
 public:
  template<template<typename> class PtrImpl, typename Type>
  static JSER_INLINE auto get_class(const basic_ref<PtrImpl, Type>& ref)
  -> ::java::lang::Class<::java::G::extends<std::remove_const_t<Type>>> {
    const ::java::object_intf*const objintf = raw_objintf(ref);
    if (objintf == nullptr) throw ::java::null_error();
    return ::java::lang::Class<::java::G::extends<std::remove_const_t<Type>>>(
        ::java::_direct(),
        objintf->__get_class__());
  }

  template<typename Tag, typename... Types>
  static JSER_INLINE auto get_class([[maybe_unused]] ::java::G::is_t<Tag, Types...> type)
  -> ::java::lang::Class<::java::G::is_t<Tag, Types...>> {
    using erased_type = typename Tag::erased_type;

    return ::java::lang::Class<::java::G::is_t<Tag, Types...>>(
        ::java::_direct(),
        erased_type::__class__());
  }

  template<template<typename> class PtrImpl, typename Type>
  static JSER_INLINE auto hash_code(const basic_ref<PtrImpl, Type>& ref, std::size_t max_cascade)
  -> ::std::size_t {
    const ::java::object_intf*const objintf = raw_objintf(ref);
    if (objintf == nullptr) return 0;
    return objintf->__hash_code__(false, max_cascade);
  }
};

///\brief Acquire the class from a type.
///\returns the java::lang::Class implemented by the pointee of the reference.
///\throws ::java::null_error if the reference is a null-reference.
template<template<typename> class PtrImpl, typename Type>
JSER_INLINE auto get_class(const basic_ref<PtrImpl, Type>& ref)
-> ::java::lang::Class<::java::G::extends<std::remove_const_t<Type>>> {
  return _reflect_ops::get_class(ref);
}

///\brief Acquire class type for the given type.
///\returns the java::lang::Class implemented by the type.
///\tparam Type a type. Only exact types may be specified.
template<typename Type>
JSER_INLINE auto get_class()
-> auto {
  return _reflect_ops::get_class(Type());
}

} /* namespace java */

#endif /* JAVA_REFLECT_H */
