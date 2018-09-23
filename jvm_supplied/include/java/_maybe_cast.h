#ifndef _JAVA__MAYBE_CAST_H
#define _JAVA__MAYBE_CAST_H

#include <type_traits>
#include <java/ref.h>
#include <java/inline.h>
#include <java/lang/Object.h>

namespace java {


///\brief Automatic cast conversion support class.
///\details Used to cast arguments during generics specializations.
template<typename Type>
class _maybe_cast_t {
 public:
  _maybe_cast_t() = delete;
  _maybe_cast_t(const _maybe_cast_t&) = delete;
  _maybe_cast_t(_maybe_cast_t&&) = default;

  JSER_INLINE explicit _maybe_cast_t(var_ref<Type>&& ref) noexcept
  : ref_(std::move(ref))
  {}

  JSER_INLINE explicit _maybe_cast_t(const var_ref<Type>& ref) noexcept
  : ref_(std::move(ref))
  {}

  template<typename OType>
  JSER_INLINE operator var_ref<OType>() {
    if constexpr(std::is_convertible_v<var_ref<Type>, var_ref<OType>>) {
      return var_ref<OType>(std::move(ref_));
    } else {
      return ::java::cast<var_ref<OType>>(std::move(ref_));
    }
  }

 private:
  var_ref<Type> ref_;
};


///\brief Helper that allows cross type assignment.
///\details
///This method yields an object wrapping the reference.
///The object allows for conversion to any reference,
///either by implicit conversion or by explicit cast.
template<typename Ref>
JSER_INLINE auto _maybe_cast(Ref&& ref) noexcept
-> std::enable_if_t<
    std::is_convertible_v<Ref, const_ref<::java::lang::Object>>,
    _maybe_cast_t<type_of_t<std::remove_cv_t<std::remove_reference_t<Ref>>>>> {
  return _maybe_cast_t<type_of_t<std::remove_cv_t<std::remove_reference_t<Ref>>>>(std::forward<Ref>(ref));
}

///\brief Helper for non-object types.
///\details Simply passes the argument through as is.
template<typename T>
JSER_INLINE auto _maybe_cast(T&& v) noexcept
-> std::enable_if_t<
    !std::is_convertible_v<T, const_ref<::java::lang::Object>>,
    std::add_rvalue_reference_t<T>> {
  return std::forward<T>(v);
}


} /* namespace java */

#endif /* _JAVA__MAYBE_CAST_H */
