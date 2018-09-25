#ifndef JAVA_PTR_H
#define JAVA_PTR_H

#include <cycle_ptr/cycle_ptr.h>
#include <java/generics.h>
#include <cassert>

namespace java {

struct allocate_t {};
constexpr allocate_t allocate{};

template<template<class> class PtrImpl, typename Type>
class basic_ref;


///\brief Variable reference.
template<typename Type>
using var_ref = basic_ref<cycle_ptr::cycle_gptr, Type>;


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


template<typename Type>
struct maybe_unpack_type_ {
  using type = Type;
};

template<template<class> class PtrImpl, typename Type>
struct maybe_unpack_type_<basic_ref<PtrImpl, Type>> {
  using type = Type;
};


template<typename BasicRef>
struct change_to_const_;

template<template<typename> class PtrType, typename Type>
struct change_to_const_<basic_ref<PtrType, Type>> {
  using type = basic_ref<PtrType, std::add_const_t<Type>>;
};


template<typename BasicRef>
struct change_to_non_const_;

template<template<typename> class PtrType, typename Type>
struct change_to_non_const_<basic_ref<PtrType, Type>> {
  using type = basic_ref<PtrType, std::remove_const_t<Type>>;
};

} /* namespace java::<unnamed> */


///\brief Extract the underlying type from a basic_ref.
template<typename BasicRef>
using type_of = type_of_<BasicRef>;

///\brief Extract the underlying type from a basic_ref.
template<typename BasicRef>
using type_of_t = typename type_of<BasicRef>::type;


///\brief Type declaration.
///\details Types are exposed as references, since that is the most similar way
///for anyone, to reason about the types.
///By similar, we mean that it resembles the Java language most closely.
template<typename Tag, typename... Args>
using type = var_ref<
    std::conditional_t<
        std::is_const_v<Tag>,
        const java::G::is<std::remove_const_t<Tag>, typename maybe_unpack_type_<Args>::type...>,
        java::G::is<std::remove_const_t<Tag>, typename maybe_unpack_type_<Args>::type...>>>;

} /* namespace java */

#include <type_traits>
#include <utility>
#include <memory>
#include <typeinfo>
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
template<typename ErasedRef>
class _accessor_base {
 protected:
  constexpr JSER_INLINE _accessor_base(ErasedRef& ref) noexcept : ref__(&ref) {}

  _accessor_base() = default;
  JSER_INLINE _accessor_base(const _accessor_base&) = default;
  JSER_INLINE _accessor_base(_accessor_base&&) = default;
  _accessor_base& operator=(const _accessor_base&) = delete;
  _accessor_base& operator=(_accessor_base&&) = delete;
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
  JSER_INLINE auto ref_() const -> const ErasedType& {
    assert(ref__ != nullptr);
    if constexpr(std::is_convertible_v<ErasedRef*, ErasedType*>) {
      return *ref__;
    } else {
      return dynamic_cast<const ErasedType&>(*ref__);
    }
  }

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
  JSER_INLINE auto ref_() -> ErasedType& {
    assert(ref__ != nullptr);
    if constexpr(std::is_convertible_v<ErasedRef*, ErasedType*>) {
      return *ref__;
    } else {
      return dynamic_cast<ErasedType&>(*ref__);
    }
  }

 private:
  ErasedRef*const ref__ = nullptr;
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
///\tparam Type java::G::* (generics) type description.
template<typename Type>
struct _basic_ref_inheritance {
  static_assert(
      java::type_traits::is_generic_v<std::remove_const_t<Type>>,
      "Must use generics as template arguments.");

 private:
  // A dummy base used to figure out accessor information prior to us deciding
  // on the actual base type to use.
  using obj_base = ::std::conditional_t<
      ::std::is_const_v<Type>,
      _accessor_base<const java::_erased::java::lang::Object>,
      _accessor_base<java::_erased::java::lang::Object>>;
  ///\brief Type-erased type held by basic_ref, without considering const-ness.
  using erased_type_without_constness = typename _accessor_for_type_<obj_base, ::std::remove_const_t<Type>>::erased_type;

 public:
  ///\brief Type-erased type held by basic_ref.
  using erased_type = ::std::conditional_t<
      ::std::is_const_v<Type>,
      const erased_type_without_constness,
      erased_type_without_constness>;
  ///\brief Base type that all accessors (ultimately) inherit from.
  using base_type = _accessor_base<erased_type>;
  ///\brief Accessor type.
  using accessor_type = _accessor_for_type<base_type, ::std::remove_const_t<Type>>;

  // Validation
  static_assert(std::is_base_of_v<base_type, accessor_type>,
      "Bad accessor type.");

 private:
  ///\brief Not instantiable.
  _basic_ref_inheritance() noexcept = delete;
};

struct _no_static_accessor_ {};

template<typename T>
struct _static_accessor_for_type_ {
  using type = _no_static_accessor_;
};

template<typename Tag, typename... Args>
struct _static_accessor_for_type_<::java::G::is_t<Tag, Args...>> {
  using type = _static_accessor<Tag, Args...>;
};

template<typename T>
using _static_accessor_for_type = typename _static_accessor_for_type_<T>::type;

template<typename T>
class _accessor_proxy final
: private _basic_ref_inheritance<T>::accessor_type
{
  template<template<typename> class, typename> friend class ::java::basic_ref;

 private:
  explicit JSER_INLINE _accessor_proxy(typename _basic_ref_inheritance<T>::erased_type& ref) noexcept
  : _basic_ref_inheritance<T>::base_type(ref)
  {}

 public:
  JSER_INLINE _accessor_proxy(const _accessor_proxy&) = default;
  JSER_INLINE _accessor_proxy& operator=(const _accessor_proxy&) = delete;

  JSER_INLINE auto operator->() const noexcept
  -> typename _basic_ref_inheritance<T>::accessor_type* {
    return const_cast<_accessor_proxy*>(this);
  }
};

template<typename T>
class _accessor_proxy<const T> final
: private _basic_ref_inheritance<const T>::accessor_type
{
  template<template<typename> class, typename> friend class ::java::basic_ref;

 private:
  explicit JSER_INLINE _accessor_proxy(typename _basic_ref_inheritance<const T>::erased_type& ref) noexcept
  : _basic_ref_inheritance<const T>::base_type(ref)
  {}

 public:
  JSER_INLINE _accessor_proxy(const _accessor_proxy&) = default;
  JSER_INLINE _accessor_proxy& operator=(const _accessor_proxy&) = delete;

  JSER_INLINE auto operator->() const noexcept
  -> const typename _basic_ref_inheritance<const T>::accessor_type* {
    return this;
  }
};

} /* namespace java::<unnamed> */


/**
 * \brief Provide raw pointer access.
 * \tparam Tag The tag of the erased type that you wish to access.
 * \param r Reference on which to access the raw pointer.
 * \returns Raw cycle_gptr of the erased type corresponding to the Tag.
 */
template<typename Tag, template<class> class PtrImpl, typename Type>
JSER_INLINE auto raw_ptr(const basic_ref<PtrImpl, Type>& r)
-> std::enable_if_t<
    ::java::type_traits::implements_tag_v<Tag, typename _basic_ref_inheritance<Type>::accessor_type>,
    cycle_ptr::cycle_gptr<typename Tag::erased_type>> {
  if constexpr(std::is_convertible_v<decltype(r.p_), cycle_ptr::cycle_gptr<typename Tag::erased_type>>) {
    return r.p_;
  } else {
    if (r == nullptr) return nullptr;
    auto result = std::dynamic_pointer_cast<typename Tag::erased_type>(r.p_);
    if (result == nullptr) throw std::bad_cast();
    return result;
  }
}


///\brief Internally used tag type, to support casting.
struct _cast {};
///\brief Internally used tag type, to support direct assignment.
struct _direct {};

/**
 * \brief Perform a type cast.
 * \tparam The type to cast to.
 * \param r Reference that is to be casted.
 * \returns Casted pointer.
 */
template<typename RefType, template<class> class PtrImpl, typename Type>
JSER_INLINE auto cast(const basic_ref<PtrImpl, Type>& r)
-> var_ref<type_of_t<RefType>> {
  if constexpr(std::is_convertible_v<basic_ref<PtrImpl, Type>, var_ref<type_of_t<RefType>>>) {
    return r;
  } else {
    return var_ref<type_of_t<RefType>>(_cast(), r);
  }
}


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
: public _static_accessor_for_type<Type>
{
  // Be friends with all our specializations.
  template<template<class> class, typename> friend class java::basic_ref;

  // Be friend the raw_ptr function.
  template<typename FnTag, template<class> class FnPtrImpl, typename FnType>
  friend auto raw_ptr(const basic_ref<FnPtrImpl, FnType>&)
  -> std::enable_if_t<
      ::java::type_traits::implements_tag_v<FnTag, typename _basic_ref_inheritance<FnType>::accessor_type>,
      cycle_ptr::cycle_gptr<typename FnTag::erased_type>>;

  // Be friend the cast function.
  template<typename FnRefType, template<class> class FnPtrImpl, typename FnType>
  friend auto cast(const basic_ref<FnPtrImpl, FnType>&)
  -> var_ref<type_of_t<FnRefType>>;

 protected:
  using erased_type = typename _basic_ref_inheritance<Type>::erased_type;
  using ptr_type = PtrImpl<erased_type>;

 public:
  JSER_INLINE basic_ref() = default;
  JSER_INLINE constexpr basic_ref(std::nullptr_t) : basic_ref() {}

  JSER_INLINE basic_ref(const basic_ref&)
      noexcept(std::is_nothrow_copy_constructible_v<ptr_type>) = default;
  JSER_INLINE basic_ref(basic_ref&&)
      noexcept(std::is_nothrow_move_constructible_v<ptr_type>) = default;

  template<typename... Args, typename = std::void_t<decltype(std::declval<::java::_constructor<Type>>()(std::declval<Args>()...))>>
  explicit JSER_INLINE basic_ref(allocate_t a, Args&&... args)
  : basic_ref(_direct(), ::java::_constructor<Type>()(std::forward<Args>(args)...))
  {}

 private:
  template<typename ErasedType, typename X, typename Y>
  static auto is_instance_of_(X* x, Y* y) noexcept
  -> bool {
    if constexpr(std::is_convertible_v<X*, const ErasedType*>
        || std::is_convertible_v<Y*, const ErasedType*>) {
      return true;
    } else {
      if (x != nullptr && dynamic_cast<const ErasedType*>(x) == nullptr)
        return false;
      if (y != nullptr && dynamic_cast<const ErasedType*>(y) == nullptr)
        return false;
      return true;
    }
  }

  template<typename X>
  JSER_INLINE basic_ref([[maybe_unused]] _direct d, const cycle_ptr::cycle_gptr<X>& p, java::G::pack_t<> my_type) {
    if constexpr(std::is_convertible_v<cycle_ptr::cycle_gptr<X>, ptr_type>) {
      p_ = p; // Implicit cast.
    } else {
      p_ = std::dynamic_pointer_cast<erased_type>(p); // Explicit cast.

      // Error out if the cast failed.
      if (p_ == nullptr && p != nullptr)
        throw std::bad_cast();
    }
  }

  template<typename X, typename MyType0, typename... MyTypes>
  JSER_INLINE basic_ref([[maybe_unused]] _direct d, const cycle_ptr::cycle_gptr<X>& p, [[maybe_unused]] MyType0 my_type0, [[maybe_unused]] MyTypes... my_types) {
    if constexpr(std::is_convertible_v<cycle_ptr::cycle_gptr<X>, ptr_type>) {
      p_ = p; // Implicit cast.
    } else {
      p_ = std::dynamic_pointer_cast<erased_type>(p); // Explicit cast.

      // Error out if the cast failed.
      if (p_ == nullptr && p != nullptr)
        throw std::bad_cast();
    }

    // Validate that all the casts are correct.
    for (bool b : /* initializer list */ {
        basic_ref::is_instance_of_<typename MyType0::tag::erased_type>(p_.get(), p.get()),
        basic_ref::is_instance_of_<typename MyTypes::tag::erased_type>(p_.get(), p.get())...
        }) {
      if (!b) throw std::bad_cast();
    }
  }

  template<typename X, typename... MyTypes>
  JSER_INLINE basic_ref(_direct d, const cycle_ptr::cycle_gptr<X>& p, [[maybe_unused]] java::G::pack_t<MyTypes...> my_type)
  : basic_ref(d, p, MyTypes()...)
  {}

 public:
  ///\brief Direct assignment.
  ///\details Assigns the pointer directly, bypassing any generics checks.
  ///\throws std::bad_cast if the pointer element does not implement all erased types held by this basic_ref.
  template<typename X>
  JSER_INLINE basic_ref(_direct d, const cycle_ptr::cycle_gptr<X>& p)
  : basic_ref(d, p, Type())
  {}

 private:
  template<template<class> class XImpl, typename XType>
  JSER_INLINE basic_ref([[maybe_unused]] _cast c, const basic_ref<XImpl, XType>& x, java::G::pack_t<> my_type)
  : p_(std::dynamic_pointer_cast<erased_type>(x.p_))
  {
    // Error out if the cast failed.
    if (p_ == nullptr && x.p_ != nullptr)
      throw std::bad_cast();
  }

  template<template<class> class XImpl, typename XType, typename MyType0, typename... MyTypes>
  JSER_INLINE basic_ref([[maybe_unused]] _cast c, const basic_ref<XImpl, XType>& x, [[maybe_unused]] MyType0 my_type0, [[maybe_unused]] MyTypes... my_types)
  : p_(std::dynamic_pointer_cast<erased_type>(x.p_))
  {
    // Error out if the cast failed.
    if (p_ == nullptr && x.p_ != nullptr)
      throw std::bad_cast();

    // Validate that all the casts are correct.
    for (bool b : /* initializer list */ {
        basic_ref::is_instance_of_<typename MyType0::tag::erased_type>(p_.get(), x.p_.get()),
        basic_ref::is_instance_of_<typename MyTypes::tag::erased_type>(p_.get(), x.p_.get())...
        }) {
      if (!b) throw std::bad_cast();
    }
  }

  template<template<class> class XImpl, typename XType, typename... MyTypes>
  JSER_INLINE basic_ref(_cast c, const basic_ref<XImpl, XType>& x, [[maybe_unused]] java::G::pack_t<MyTypes...> my_type)
  : basic_ref(c, x, MyTypes()...)
  {}

  template<template<class> class XImpl, typename XType>
  JSER_INLINE basic_ref(_cast c, const basic_ref<XImpl, XType>& x)
  : basic_ref(c, x, Type())
  {}

 public:
  ///\brief Copy assignment.
  ///\details
  ///Copy assignment is only provided if this type is self-assignable.
  ///Types of the form \ref java::G::extends "extends"
  ///or \ref java::G::super "super"
  ///are not self assignable.
  ///
  ///\note Even if the type is not self-assignable, it will still be
  ///copy/move constructible.
  template<bool Enable = java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<Type>>>
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
  template<bool Enable = java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<Type>>>
  JSER_INLINE auto operator=(basic_ref&& other)
      noexcept(std::is_nothrow_move_assignable_v<ptr_type>)
  -> std::enable_if_t<Enable, basic_ref&> {
    p_ = std::move(other.p_);
    return *this;
  }

  ///\brief Cross type copy assignment.
  template<template<class> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<OType>>
          && (::std::is_const_v<Type> || !::std::is_const_v<OType>)>>
  JSER_INLINE basic_ref(const basic_ref<OPtrImpl, OType>& other) noexcept {
    if constexpr(std::is_convertible_v<typename basic_ref<OPtrImpl, OType>::ptr_type, ptr_type>) {
      // Use implicit cast if possible.
      p_ = other.p_;
    } else {
      // Use explicit cast only if required.
      p_ = std::dynamic_pointer_cast<erased_type>(other.p_);
    }
  }

  ///\brief Cross type move assignment.
  template<template<class> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<OType>>
          && (::std::is_const_v<Type> || !::std::is_const_v<OType>)>>
  JSER_INLINE basic_ref(basic_ref<OPtrImpl, OType>&& other) noexcept {
    if constexpr(std::is_convertible_v<typename basic_ref<OPtrImpl, OType>::ptr_type, ptr_type>) {
      // Use implicit cast if possible.
      p_ = std::move(other.p_);
    } else {
      // Use explicit cast only if required.
      p_ = std::dynamic_pointer_cast<erased_type>(other.p_);
      other.p_ = nullptr;
    }
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
      java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<OType>>
      && (::std::is_const_v<Type> || !::std::is_const_v<OType>),
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
      java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<OType>>
      && (::std::is_const_v<Type> || !::std::is_const_v<OType>),
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
      java::type_traits::is_assignable_v<::std::remove_const_t<Type>, ::std::remove_const_t<OType>>
      && java::type_traits::is_assignable_v<::std::remove_const_t<OType>, ::std::remove_const_t<Type>>
      && (::std::is_const_v<Type> == ::std::is_const_v<OType>)> {
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

  /**
   * \brief Dereference this reference.
   * \details
   * Dereferences this reference using a proxy,
   * granting access to the instance methods of the pointed-to object.
   * \throws ::java::null_error if the reference is a null reference.
   */
  JSER_INLINE auto operator->() const
  -> _accessor_proxy<Type> {
    if (!*this) throw ::java::null_error();
    return _accessor_proxy<Type>(*p_);
  }

 private:
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
using param_t = type<java::type_traits::parameter_type_for_t<type_of_t<BasicRef>>>;

///\brief Convert basic_ref to a return type reference.
///\details Implements type erase, such that `? extends X` will be `X`.
///\sa java::type_traits::return_type_for
template<typename BasicRef>
using return_t = type<java::type_traits::return_type_for_t<type_of_t<BasicRef>>>;

///\brief Convert basic_ref to a const-reference.
template<typename BasicRef>
using const_ref = typename change_to_const_<BasicRef>::type;

///\brief Convert basic_ref to a non-const-reference.
template<typename BasicRef>
using non_const_ref = typename change_to_non_const_<BasicRef>::type;


} /* namespace java */

namespace std {

///\brief Specialize std::exchange for basic_ref.
template<template<typename> class PtrImpl, typename Type, typename U>
auto exchange(::java::basic_ref<PtrImpl, Type>& obj, U&& new_value)
noexcept(
    is_nothrow_constructible_v<
        ::java::var_t<::java::basic_ref<PtrImpl, Type>>,
        ::java::basic_ref<PtrImpl, Type>>
    &&
    is_nothrow_assignable_v<::java::basic_ref<PtrImpl, Type>&, U&&>)
-> std::enable_if_t<
    is_assignable_v<::java::basic_ref<PtrImpl, Type>&, U&&>,
    ::java::var_t<::java::basic_ref<PtrImpl, Type>>> {
  ::java::var_t<::java::basic_ref<PtrImpl, Type>> result = std::move(obj);
  obj = std::forward<U>(new_value);
  return result;
}

} /* namespace std */

#endif /* JAVA_PTR_H */
