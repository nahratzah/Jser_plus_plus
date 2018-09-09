#ifndef JAVA_PTR_H
#define JAVA_PTR_H

#include <cycle_ptr/cycle_ptr.h>
#include <java/generics.h>

namespace java {

template<template<class> class PtrImpl, typename Type>
class basic_ref;


///\brief Variable reference.
template<typename Type>
using var_ref = basic_ref<cycle_ptr::cycle_gptr, Type>;

///\brief Type declaration.
///\details Types are exposed as references, since that is the most similar way
///for anyone, to reason about the types.
///By similar, we mean that it resembles the Java language most closely.
template<typename Tag, typename... Args>
using type = var_ref<java::G::is<Tag, Args...>>;

} /* namespace java */

#include <type_traits>
#include <utility>
#include <memory>
#include <java/_accessor.h>
#include <java/type_traits.h>
#include <java/object_intf.h>
#include <java/null_error.h>
#include <java/inline.h>

namespace java {
namespace {
template<typename, typename> struct _accessor_for_type_;
} /* namespace java::<unnamed> */

namespace _erased::java::lang {
class Object;
} /* namespace java::_erased::java::lang */

/**
 * \brief Base type for accessors.
 * \details Provides access to the type-erased object held by the accessor.
 * \tparam ErasedPtr pointer implementation.
 */
template<typename ErasedPtr>
class _accessor_base {
 protected:
  constexpr JSER_INLINE _accessor_base() = default;

  JSER_INLINE _accessor_base(const _accessor_base&) = default;
  JSER_INLINE _accessor_base(_accessor_base&&) = default;
  JSER_INLINE _accessor_base& operator=(const _accessor_base&) = default;
  JSER_INLINE _accessor_base& operator=(_accessor_base&&) = default;
  JSER_INLINE ~_accessor_base() = default;

  /**
   * \brief Acquire reference to the given type.
   * \details
   * Performs implicit cast if possible.
   * Uses dynamic cast if implicit cast is not possible.
   * \tparam ErasedType requested type.
   * \returns reference to the requested type.
   * \throws ::java::null_error if this is null.
   */
  template<typename ErasedType>
  JSER_INLINE auto ref_() const -> ErasedType& {
    if constexpr(std::is_convertible_v<typename std::pointer_traits<ErasedPtr>::element_type*, ErasedType*>) {
      return ref__();
    } else {
      return dynamic_cast<ErasedType&>(ref__());
    }
  }

 private:
  virtual auto ref__() const -> typename std::pointer_traits<ErasedPtr>::element_type& = 0;
};

template<typename Base, typename T>
using _accessor_for_type = typename _accessor_for_type_<Base, T>::type;

namespace {

template<typename Base, typename Tag, typename... Args>
struct _accessor_for_type_<Base, java::G::is_t<Tag, Args...>> {
  using erased_type = typename Tag::erased_type;
  using type = _accessor<Base, Tag, Args...>;
};

template<typename Base, typename Tag, typename... Args>
struct _accessor_for_type_<Base, java::G::extends_t<Tag, Args...>>
: public _accessor_for_type_<Base, java::G::is<Tag, Args...>>
{};

template<typename Base, typename Tag, typename... Args>
struct _accessor_for_type_<Base, java::G::super_t<Tag, Args...>>
: public _accessor_for_type_<Base, java::G::is<java::_tags::java::lang::Object>>
{};

template<typename Base, typename T0, typename... T>
struct _accessor_for_type_<Base, java::G::pack_t<T0, T...>> {
  using erased_type = typename _accessor_for_type_<Base, T0>::erased_type;

  struct type
  : public virtual _accessor_for_type_<Base, T0>::type,
    public virtual _accessor_for_type_<Base, T>::type...
  {
   protected:
    JSER_INLINE type() = default;
    JSER_INLINE type(const type&) = default;
    JSER_INLINE type(type&&) = default;
    JSER_INLINE type& operator=(const type&) = default;
    JSER_INLINE type& operator=(type&&) = default;
    JSER_INLINE ~type() = default;
  };
};

template<typename Base>
struct _accessor_for_type_<Base, java::G::pack_t<>>
: public _accessor_for_type_<Base, java::G::is<java::_tags::java::lang::Object>>
{};


///\brief Helper type to figure out types to use in basic_ref.
///\tparam PtrType template of the pointer class to use, typically one of `cycle_ptr::cycle_gptr` or `cycle_ptr::cycle_member_ptr`.
///\tparam Type java::G::* (generics) type description.
template<template<typename> class PtrType, typename Type>
struct _basic_ref_inheritance {
  static_assert(
      java::type_traits::is_generic_v<Type>,
      "Must use generics as template arguments.");

 private:
  // A dummy base used to figure out accessor information prior to us deciding
  // on the actual base type to use.
  using obj_base = _accessor_base<cycle_ptr::cycle_gptr<java::_erased::java::lang::Object>>;

 public:
  ///\brief Type-erased type held by basic_ref.
  using erased_type = typename _accessor_for_type<obj_base, Type>::erased_type;
  ///\brief Pointer-type held by basic_ref.
  using ptr_type = PtrType<erased_type>;
  ///\brief Base type that all accessors (ultimately) inherit from.
  using base_type = _accessor_base<ptr_type>;
  ///\brief Accessor type.
  using accessor_type = _accessor_for_type<base_type, Type>;

  // Validation
  static_assert(std::is_base_of_v<base_type, accessor_type>,
      "Bad accessor type.");

 private:
  ///\brief Not instantiable.
  _basic_ref_inheritance() noexcept = delete;
};

} /* namespace java::<unnamed> */


/**
 * \brief Basic java reference.
 * \details
 * A java reference is a PIMPL implementation to a type-erased java Object.
 *
 * \tparam PtrImpl Template specifying the style of pointer to use.
 * Must be one of `cycle_ptr::cycle_gptr` or `cycle_ptr::cycle_member_ptr`.
 * \tparam PtrImpl Template pointer type.
 * \tparam Type Type of the reference.
 */
template<template<class> class PtrImpl, typename Type>
class basic_ref final
: public _basic_ref_inheritance<PtrImpl, Type>::accessor_type
{
  // Be friends with all our specializations.
  template<template<class> class, typename> friend class java::basic_ref;

 protected:
  using erased_type = typename _basic_ref_inheritance<PtrImpl, Type>::erased_type;
  using ptr_type = typename _basic_ref_inheritance<PtrImpl, Type>::ptr_type;

  // Validate erased_type
  static_assert(std::is_base_of_v<java::object_intf, erased_type>);

 public:
  JSER_INLINE constexpr basic_ref() noexcept = default;
  explicit JSER_INLINE constexpr basic_ref(std::nullptr_t) noexcept : basic_ref() {}

  JSER_INLINE basic_ref(const basic_ref&)
      noexcept(std::is_nothrow_copy_constructible_v<ptr_type>) = default;
  JSER_INLINE basic_ref(basic_ref&&)
      noexcept(std::is_nothrow_move_constructible_v<ptr_type>) = default;

  ///\brief Copy assignment.
  ///\details
  ///Copy assignment is only provided if this type is self-assignable.
  ///Types of the form \ref java::G::extends "extends"
  ///or \ref java::G::super "super"
  ///are not self assignable.
  ///
  ///\note Even if the type is not self-assignable, it will still be
  ///copy/move constructible.
  template<bool Enable = java::type_traits::is_assignable_v<Type, Type>>
  JSER_INLINE auto operator=(const basic_ref& other)
      noexcept(std::is_nothrow_copy_assignable_v<ptr_type>)
  -> std::enable_if_t<Enable, basic_ref&> {
    p_ = other.p_;
    return *this;
  }

  ///\brief Move assignment.
  ///\details
  ///Copy assignment is only provided if this type is self-assignable.
  ///Types of the form \ref java::G::extends "extends"
  ///or \ref java::G::super "super"
  ///are not self assignable.
  ///
  ///\note Even if the type is not self-assignable, it will still be
  ///copy/move constructible.
  template<bool Enable = java::type_traits::is_assignable_v<Type, Type>>
  JSER_INLINE auto operator=(basic_ref&& other)
      noexcept(std::is_nothrow_move_assignable_v<ptr_type>)
  -> std::enable_if_t<Enable, basic_ref&> {
    p_ = std::move(other.p_);
    return *this;
  }

  /**
   * \brief Make this reference a null-reference.
   * \param np nullptr
   * \returns *this
   * \post `*this == nullptr`
   */
  JSER_INLINE auto operator=(std::nullptr_t np) noexcept -> basic_ref& {
    p_ = nullptr;
    return *this;
  }

  /**
   * \brief Cross-type assignment.
   * \param other A basic reference to assign from.
   * \returns *this
   * \post `*this == other`
   */
  template<template<class> class OPtrImpl, typename OType>
  JSER_INLINE auto operator=(const basic_ref<OPtrImpl, OType>& other) noexcept
  -> std::enable_if_t<
      java::type_traits::is_assignable_v<Type, OType>,
      basic_ref&> {
    if constexpr(std::is_convertible_v<typename basic_ref<OPtrImpl, OType>::ptr_type, ptr_type>) {
      // Use implicit cast if possible.
      p_ = other.p_;
    } else {
      // Use explicit cast only if required.
      p_ = std::dynamic_pointer_cast<erased_type>(other.p_);
    }
    return *this;
  }

  /**
   * \brief Cross-type move assignment.
   * \param other A basic reference to assign from.
   * \returns *this
   * \post `*this == original-value-of-other && other == nullptr`
   */
  template<template<class> class OPtrImpl, typename OType>
  JSER_INLINE auto operator=(basic_ref<OPtrImpl, OType>&& other) noexcept
  -> std::enable_if_t<
      java::type_traits::is_assignable_v<Type, OType>,
      basic_ref&> {
    if constexpr(std::is_convertible_v<typename basic_ref<OPtrImpl, OType>::ptr_type, ptr_type>) {
      // Use implicit cast if possible.
      p_ = std::move(other.p_);
    } else {
      // Use explicit cast only if required.
      p_ = std::dynamic_pointer_cast<erased_type>(other.p_);
      other.p_ = nullptr;
    }
    return *this;
  }

  /**
   * \brief Swap two references.
   * \details This method only exists for reference pairs which are assignable to each other.
   * \note swap can only be found using ADL.
   */
  template<template<class> class OPtrImpl, typename OType>
  friend JSER_INLINE auto swap(const basic_ref& x, const basic_ref<OPtrImpl, OType>& y) noexcept
  -> std::enable_if_t<
      java::type_traits::is_assignable_v<Type, OType>
      && java::type_traits::is_assignable_v<OType, Type>> {
    if constexpr(std::is_swappable_with_v<typename basic_ref<OPtrImpl, OType>::ptr_type&, ptr_type&>) {
      using std::swap;
      swap(x.p_, y.p_);
    } else {
      cycle_ptr::cycle_gptr<erased_type> tmp = std::move(x.p_);
      x.p_ = std::move(y.p_);
      y.p_ = std::move(tmp);
    }
  }

  /**
   * \brief Test if this is a valid reference.
   * \details Tests if this is not a null-reference.
   * \returns True if the reference is valid, false if this is a null-reference.
   */
  explicit JSER_INLINE operator bool() const noexcept {
    return bool(p_);
  }

  /**
   * \brief Test if this is a null-reference.
   * \returns True if this is a null-reference, false otherwise.
   */
  JSER_INLINE auto operator!() const noexcept -> bool {
    return !p_;
  }

  /**
   * \brief Test if this is a null-reference.
   * \param np nullptr
   * \returns True if this is a null-reference, false otherwise.
   */
  JSER_INLINE auto operator==(std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  /**
   * \brief Test if this is not a null-reference.
   * \param np nullptr
   * \returns False if this is a null-reference, true otherwise.
   */
  JSER_INLINE auto operator!=(std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

 private:
  // Provide implementation of method in base_type.
  JSER_INLINE auto ref__() const -> erased_type& override {
    if (p_ == nullptr) throw ::java::null_error();
    return *p_;
  }

  ///\brief Pointer to the erased type.
  ptr_type p_ = nullptr;
};

template<template<typename> class Base, typename P>
JSER_INLINE auto operator==(std::nullptr_t np, const basic_ref<Base, P>& b) noexcept
-> bool {
  return b == nullptr;
}

template<template<typename> class Base, typename P>
JSER_INLINE auto operator!=(std::nullptr_t np, const basic_ref<Base, P>& b) noexcept
-> bool {
  return b != nullptr;
}


namespace {

template<template<typename> class PtrType, typename P>
struct change_ptr_type_;

template<template<typename> class PtrType, template<typename> class OldPtrType, typename Type>
struct change_ptr_type_<PtrType, basic_ref<OldPtrType, Type>> {
  using type = basic_ref<PtrType, Type>;
};

template<typename BasicRef>
struct type_of_;

template<template<typename> class PtrType, typename Type>
struct type_of_<basic_ref<PtrType, Type>> {
  using type = Type;
};

} /* namespace java::<unnamed> */


///\brief Extract the underlying type from a basic_ref.
template<typename BasicRef>
using type_of = type_of_<BasicRef>;

///\brief Extract the underlying type from a basic_ref.
template<typename BasicRef>
using type_of_t = typename type_of<BasicRef>::type;


///\brief Convert basic_ref to a variable reference.
template<typename BasicRef>
using var_t = typename change_ptr_type_<cycle_ptr::cycle_gptr, BasicRef>::type;

///\brief Convert basic_ref to a field reference.
///\bug This should be a raw pointer with a method that yields the field, using the raw pointer as reference.
template<typename BasicRef>
using field_t = typename change_ptr_type_<cycle_ptr::cycle_member_ptr, BasicRef>::type;

///\brief Convert basic_ref to a parameter reference.
///\details Implements type erase, such that `? super X` will be `X`.
///\sa java::type_traits::parameter_type_for
template<typename BasicRef>
using param_t = var_t<java::type_traits::parameter_type_for_t<type_of_t<BasicRef>>>;

///\brief Convert basic_ref to a return type reference.
///\details Implements type erase, such that `? extends X` will be `X`.
///\sa java::type_traits::return_type_for
template<typename BasicRef>
using return_t = var_t<java::type_traits::return_type_for_t<type_of_t<BasicRef>>>;


} /* namespace java */

#endif /* JAVA_PTR_H */
