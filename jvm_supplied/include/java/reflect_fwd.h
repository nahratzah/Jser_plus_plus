#ifndef JAVA_REFLECT_FWD_H
#define JAVA_REFLECT_FWD_H

#include <java/ref.h>
#include <java/fwd/java/lang/Class.h>

namespace java {

///\brief Acquire the class from a type.
///\returns the java::lang::Class implemented by the pointee of the reference.
///\throws ::java::null_error if the reference is a null-reference.
template<template<typename> class PtrImpl, typename Type>
JSER_INLINE auto get_class(const basic_ref<PtrImpl, Type>& ref)
-> ::java::lang::Class<::java::G::extends<std::remove_const_t<Type>>>;

///\brief Acquire class type for the given type.
///\returns the java::lang::Class implemented by the type.
///\tparam Type a type. Only exact types may be specified.
template<typename Type>
JSER_INLINE auto get_class()
-> auto;

} /* namespace java */

#endif /* JAVA_REFLECT_FWD_H */
