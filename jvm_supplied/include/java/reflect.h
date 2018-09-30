#ifndef JAVA_REFLECT_H
#define JAVA_REFLECT_H

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <cycle_ptr/cycle_ptr.h>
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
  static JSER_INLINE auto get_class(const object_intf& objintf)
  -> ::java::lang::Class<::java::G::pack<>> {
    return ::java::lang::Class<::java::G::pack<>>(
        ::java::_direct(),
        objintf.__get_class__());
  }

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


///\brief Helper for object equality.
///\details Protects against infinite loops in the data structure,
///at the cost of some memory and performance.
class _equal_helper {
 private:
  using pair = std::tuple<
      cycle_ptr::cycle_gptr<const object_intf>,
      cycle_ptr::cycle_gptr<const object_intf>>;

  struct hasher {
    auto operator()(const pair& x) const noexcept -> std::size_t;
  };

 public:
  ///\brief Mark the equality as failed.
  auto fail() noexcept -> void {
    success_ = false;
  }

  ///\brief Test if the equality still holds.
  auto ok() const noexcept -> bool {
    return success_;
  }

  ///\brief Compare two references and update the state.
  ///\details If the comparison doesn't hold, this function does nothing.
  ///\param x,y Two references to compare.
  ///\returns `*this`
  template<
      template<typename> class XPtr, typename XType,
      template<typename> class YPtr, typename YType>
  auto operator()(const basic_ref<XPtr, XType>& x, const basic_ref<YPtr, YType>& y)
  -> _equal_helper& {
    if (!success_) return *this;
    return eq_(raw_objintf_ptr(x), raw_objintf_ptr(y));
  }

  ///\brief Compare two references and update the state.
  ///\details If the comparison doesn't hold, this function does nothing.
  ///
  ///This function can be used if the comparison is not field based.
  ///\param x A references to compare.
  ///\param y An object_intf to compare against.
  ///\returns `*this`
  template<template<typename> class XPtr, typename XType>
  auto operator()(const basic_ref<XPtr, XType>& x, const object_intf& y)
  -> _equal_helper& {
    if (!success_) return *this;
    return eq_(raw_objintf_ptr(x), y);
  }

  ///\brief Compare two ordinary types.
  ///\details If the comparison doesn't hold, this function does nothing.
  ///
  ///This function only participates in overload resolution,
  ///if neither \p x nor \p y are references.
  ///\param x,y Two values to compare.
  ///\returns `*this`
  template<typename X, typename Y>
  auto operator()(const X& x, const Y& y)
  -> std::enable_if_t<
      std::is_convertible_v<decltype(std::declval<const X&>() == std::declval<const Y&>()), bool>
      && !std::is_convertible_v<const X, const_ref<java::lang::Object>>
      && !std::is_convertible_v<const Y, const_ref<java::lang::Object>>
      && !std::is_convertible_v<const X, ::cycle_ptr::cycle_gptr<const void>>
      && !std::is_convertible_v<const Y, ::cycle_ptr::cycle_gptr<const void>>,
      _equal_helper&> {
    if (!success_) return *this;
    success_ = (x == y);
    return *this;
  }

  ///\brief Compare two references and update the state.
  ///\details If the comparison doesn't hold, this function does nothing.
  ///\param x,y Two references to compare.
  ///\returns `*this`
  auto operator()(::cycle_ptr::cycle_gptr<const object_intf> x,
                  ::cycle_ptr::cycle_gptr<const object_intf> y)
  -> _equal_helper& {
    if (!success_) return *this;
    return eq_(x, y);
  }

  ///\brief Test comparison.
  ///\details Comparison state is restored when this function returns.
  ///\param x,y Two references to compare.
  ///\returns Boolean indicating if the two are compared equal.
  template<
      template<typename> class XPtr, typename XType,
      template<typename> class YPtr, typename YType>
  auto test(const basic_ref<XPtr, XType>& x, const basic_ref<YPtr, YType>& y)
  -> bool {
    return test_(raw_objintf_ptr(x), raw_objintf_ptr(y));
  }

  ///\brief Test comparison.
  ///\details Comparison state is restored when this function returns.
  ///
  ///This function can be used if the comparison is not field based.
  ///\param x A references to compare.
  ///\param y An object_intf to compare against.
  ///\returns Boolean indicating if the two are compared equal.
  template<template<typename> class XPtr, typename XType>
  auto test(const basic_ref<XPtr, XType>& x, const object_intf& y)
  -> bool {
    return test_(raw_objintf_ptr(x), y);
  }

  ///\brief Test comparison.
  ///\details Comparison state is restored when this function returns.
  ///
  ///This function only participates in overload resolution,
  ///if neither \p x nor \p y are references.
  ///\param x,y Two values to compare.
  ///\returns Boolean indicating if the two are compared equal.
  template<typename X, typename Y>
  auto test(const X& x, const Y& y)
  -> std::enable_if_t<
      std::is_convertible_v<decltype(std::declval<const X&>() == std::declval<const Y&>()), bool>
      && !std::is_convertible_v<const X, const_ref<java::lang::Object>>
      && !std::is_convertible_v<const Y, const_ref<java::lang::Object>>
      && !std::is_convertible_v<const X, ::cycle_ptr::cycle_gptr<const void>>
      && !std::is_convertible_v<const Y, ::cycle_ptr::cycle_gptr<const void>>,
      bool> {
    return (x == y);
  }

  ///\brief Test comparison.
  ///\details Comparison state is restored when this function returns.
  ///\param x,y Two references to compare.
  ///\returns Boolean indicating if the two are compared equal.
  auto test(::cycle_ptr::cycle_gptr<const object_intf> x,
            ::cycle_ptr::cycle_gptr<const object_intf> y)
  -> bool {
    return test_(x, y);
  }

 private:
  auto test_(cycle_ptr::cycle_gptr<const object_intf> x,
             cycle_ptr::cycle_gptr<const object_intf> y)
  -> bool;

  auto test_(cycle_ptr::cycle_gptr<const object_intf> x,
             const object_intf& y)
  -> bool;

  auto eq_(cycle_ptr::cycle_gptr<const object_intf> x,
           cycle_ptr::cycle_gptr<const object_intf> y)
  -> _equal_helper&;

  auto eq_(cycle_ptr::cycle_gptr<const object_intf> x,
           const object_intf& y)
  -> _equal_helper&;

  ///\brief Set of visited pairs, for which equality holds.
  std::unordered_set<pair, hasher> visited_;
  ///\brief Internal comparison state.
  bool success_ = true;
};


} /* namespace java */

#endif /* JAVA_REFLECT_H */