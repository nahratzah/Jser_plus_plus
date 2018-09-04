#ifndef JAVA_PTR_H
#define JAVA_PTR_H

#include <type_traits>
#include <utility>
#include <memory>
#include <cycle_ptr/cycle_ptr.h>
#include <java/generics.h>
#include <java/_accessor.h>
#include <java/type_traits.h>
#include <java/null_error.h>

namespace java {
namespace {
template<typename, typename> struct _accessor_for_type_;
} /* namespace java::<unnamed> */

namespace java::_erased::java::lang {
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
  constexpr _accessor_base() = default;

  template<typename Init>
  explicit _accessor_base(Init&& init)
      noexcept(std::is_nothrow_constructible_v<ErasedPtr, std::add_rvalue_reference_t<Init>>)
  : p_(std::forward<Init>(init))
  {}

  _accessor_base(const _accessor_base&) = default;
  _accessor_base(_accessor_base&&) = default;
  _accessor_base& operator=(const _accessor_base&) = default;
  _accessor_base& operator=(_accessor_base&&) = default;
  ~_accessor_base() = default;

  ///\brief Acquire reference to the given type.
  ///\details Performs implicit cast if possible.
  ///Uses dynamic cast if implicit cast is not possible.
  ///\tparam ErasedType requested type.
  ///\returns reference to the requested type.
  ///\throws ::java::null_error if this is null.
  template<typename ErasedType>
  auto ref_() const -> ErasedType& {
    if constexpr(std::is_convertible_v<typename std::pointer_traits<ErasedPtr>::element_type*, ErasedType*>) {
      return ref__();
    } else {
      return dynamic_cast<ErasedType&>(ref__());
    }
  }

 private:
  virtual auto ref__() const -> ErasedType& = 0;
};

template<typename Base, typename T>
using _accessor_for_type = typename _accessor_for_type_<Base, T>::type;

namespace {

template<typename Base, typename Tag, typename... Args>
struct _accessor_for_type_<java::G::is_t<Tag, Args...>> {
  using erased_type = typename Tag::erased_type;
  using type = _accessor<Base, Tag, Args...>>;
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
  using erased_type = typename _accessor_for_type_<T0>::erased_type;

  struct type
  : public virtual typename _accessor_for_type_<Base, T0>::type,
    public virtual typename _accessor_for_type_<Base, T>::type...
  {
   protected:
    type() = default;
    type(const type&) = default;
    type(type&&) = default;
    type& operator=(const type&) = default;
    type& operator=(type&&) = default;
    ~type() = default;
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
      java::type_traits::is_generic<Type>,
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
: public typename _basic_ref_inheritance<PtrImpl, Type>::accessor_type
{
  // Be friends with all our specializations.
  template<typename<class> class, typename> friend class java::basic_ref;

 protected:
  using erased_type = typename _basic_ref_inheritance<PtrImpl, Type>::erased_type;
  using ptr_type = typename _basic_ref_inheritance<PtrImpl, Type>::ptr_type;

  // Validate erased_type
  static_assert(std::is_base_of_v<java::object_intf, erased_type>);

 public:
  constexpr basic_ref() noexcept = default;
  explicit constexpr basic_ref(std::nullptr_t) noexcept : basic_ref() {}

  basic_ref(const basic_ref&)
      noexcept(std::is_nothrow_copy_constructible_v<ptr_type>) = default;
  basic_ref(basic_ref&&)
      noexcept(std::is_nothrow_move_constructible_v<ptr_type>) = default;
  basic_ref& operator=(const basic_ref&) = default;
      noexcept(std::is_nothrow_copy_assignable_v<ptr_type>) = default;
  basic_ref& operator=(basic_ref&&)
      noexcept(std::is_nothrow_move_assignable_v<ptr_type>) = default;

  /**
   * \brief Make this reference a null-reference.
   * \param np nullptr
   * \returns *this
   * \post `*this == nullptr`
   */
  auto operator=(std::nullptr_t np) noexcept -> basic_ref& {
    p_ = nullptr;
    return *this;
  }

  /**
   * \brief Cross-type assignment.
   * \param other A basic reference to assign from.
   * \returns *this
   * \post `*this == other`
   */
  template<typename<class> class OPtrImpl, typename OType>
  auto operator=(const basic_ref<OPtrImpl, OType>& other) noexcept
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
  template<typename<class> class OPtrImpl, typename OType>
  auto operator=(const basic_ref<OPtrImpl, OType>& other) noexcept
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
  template<typename<class> class OPtrImpl, typename OType>
  friend auto swap(const basic_ref& x, const basic_ref<OPtrImpl, OType>& y) noexcept
  -> std::enable_if_t<
      java::type_traits::is_assignable_v<Type, OType>
      && java::type_traits::is_assignable_v<OType, Type>> {
    if constexpr(std::is_swappable_with_v<typename basic_ref<OPtrImpl, OType>::ptr_type&, ptr_type&>) {
      using std::swap;
      swap(x.p_, y.p_);
    } else {
      cycle_ptr::cycle_gptr<erased_type> tmp = std::move(p_);
      p_ = std::move(other.p_);
      other.p_ = std::move(tmp);
    }
  }

  /**
   * \brief Test if this is a valid reference.
   * \details Tests if this is not a null-reference.
   * \returns True if the reference is valid, false if this is a null-reference.
   */
  explicit operator bool() const noexcept {
    return bool(p_);
  }

  /**
   * \brief Test if this is a null-reference.
   * \returns True if this is a null-reference, false otherwise.
   */
  auto operator!() const noexcept -> bool {
    return !p_;
  }

  /**
   * \brief Test if this is a null-reference.
   * \param np nullptr
   * \returns True if this is a null-reference, false otherwise.
   */
  auto operator==(std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  /**
   * \brief Test if this is not a null-reference.
   * \param np nullptr
   * \returns False if this is a null-reference, true otherwise.
   */
  auto operator!=(std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

 private:
  // Provide implementation of method in base_type.
  auto ref__() const -> erased_type& override {
    if (p_ == nullptr) throw ::java::null_error();
    return *p_;
  }

  ///\brief Pointer to the erased type.
  ptr_type p_ = nullptr;
};

template<template<typename> class Base, typename P>
auto operator==(std::nullptr_t np, const basic_ref<Base, P>& b) noexcept
-> bool {
  return b == nullptr;
}

template<template<typename> class Base, typename P>
auto operator!=(std::nullptr_t np, const basic_ref<Base, P>& b) noexcept
-> bool {
  return b != nullptr;
}


template<typename... Types>
using var_ref = basic_ref<cycle_ptr::cycle_gptr, Types...>;

template<typename... Types>
using field_ref = basic_ref<cycle_ptr::cycle_member_ptr, Types...>;


} /* namespace java */

#endif /* JAVA_PTR_H */
