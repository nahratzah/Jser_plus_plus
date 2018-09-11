#ifndef JAVA_TYPE_TRAITS_H
#define JAVA_TYPE_TRAITS_H

#include <type_traits>
#include <java/generics.h>
#include <java/primitives.h>

///\brief Namespace for java type_traits.
namespace java::type_traits {
namespace {

///\brief Converter for type, that changes a type for use in parameters.
///\details For any generic, replaces super<X> with is<X>.
///\note Only the leaf type is converted.
template<typename G>
struct param_convert_ {
  static_assert(is_generic_v<G>, "Parameter conversion requires generics.");

  using type = G;
};

///\brief Converter for type, that changes a type for use as return type.
///\details For any generic, replaces extends<X> with is<X>.
///\note Only the leaf type is converted.
template<typename G>
struct return_type_convert_ {
  static_assert(is_generic_v<G>, "Result type conversion requires generics.");

  using type = G;
};


///\brief Test if type is assignable to other type.
template<typename, typename>
struct is_assignable_;


///\brief Test if type is a java primitive type.
template<typename>
struct is_java_primitive_
: std::false_type
{};

} /* namespace java::type_traits::<unnamed> */


/**
 * \brief Figure out the parameter type for a generics type.
 * \details
 * Consider a `class MyClass<T>`, with a method `void MyMethod(T v)`.
 * If this class is evaluated via a pointer `MyClass<? super CharSequence>`,
 * the method can accept anything that derives from `CharSequence`.
 *
 * Normally, types with \ref java::G::super_t "super" in the name, do not
 * accept anything (because `extends<super<...>>` is never a valid capture).
 * But in the case of an argument, it should.
 *
 * In order to assist with this, the type is converted, such that all
 * \ref java::G::super_t "super" generics are converted to
 * \ref java::G::is_t "is" generics.
 * This conversion allows the is_assignable trait to function.
 *
 * In addition, in the implementation of the method, the cast to the underlying
 * type can succeed once again using an implicit cast.
 *
 * \note We can't change to \ref java::G::extends_t "extends" generics,
 * as those are not assignable from right-hand-side.
 *
 * \tparam G a java generics type.
 */
template<typename G>
using parameter_type_for = param_convert_<G>;

///\copydoc parameter_type_for
template<typename G>
using parameter_type_for_t = typename parameter_type_for<G>::type;


/**
 * \brief Figure out the return type of a generics type.
 * \details
 * Consider a `class MyClass<T>`, with a method `T myMethod()`.
 * If this class is evaluated via a pointer `MyClass<? extends CharSequence>`,
 * the method can return anything that derives from `CharSequence`.
 *
 * Normally, types with \ref java::G::extends_t "extends" in the name,
 * would not be assignable to its own type, since there is no way of knowing
 * wether the captures would equal.
 *
 * In order to assist with this, the type is converted, such that all
 * \ref java::G::extends_t "extends" generics are converted to
 * \ref java::G::is_t "is" generics.
 * This conversion allows assignment to function properly.
 *
 * \tparam G a java generics type.
 */
template<typename G>
using return_type_for = return_type_convert_<G>;

///\copydoc return_type_for
template<typename G>
using return_type_for_t = typename return_type_for<G>::type;


/**
 * \brief Test if type X is assignable from type Y.
 * \details
 * Any type \p Y is assignable to \p X, if it extends \p X.
 *
 * \tparam X,Y a java generics type.
 */
template<typename X, typename Y>
using is_assignable = typename is_assignable_<X, Y>::type;

///\copydoc is_assignable
template<typename X, typename Y>
constexpr bool is_assignable_v = is_assignable<X, Y>::value;


/**
 * \brief Test if type T is a java primitive.
 * \tparam T The type to test for.
 */
template<typename T>
using is_java_primitive = typename is_java_primitive_<T>::type;

///\copydoc is_java_primitive
template<typename T>
constexpr bool is_java_primitive_v = is_java_primitive<T>::value;


namespace {

template<typename... X>
struct param_convert_<java::G::super_t<X...>> {
  using type = java::G::is<X...>;
};

template<typename... X>
struct param_convert_<java::G::pack_t<X...>> {
  using type = java::G::pack<typename param_convert_<X>::type...>;
};


template<typename... X>
struct return_type_convert_<java::G::extends_t<X...>> {
  using type = java::G::is<X...>;
};

template<typename... X>
struct return_type_convert_<java::G::pack_t<X...>> {
  using type = java::G::pack<typename return_type_convert_<X>::type...>;
};


// Anything is assignable to java.lang.Object.
template<typename Y>
struct is_assignable_<java::G::is_t<java::_tags::java::lang::Object>, Y> {
  using type = std::true_type;
};

template<typename... X, typename Y>
struct is_assignable_<java::G::is_t<X...>, Y> {
  using type = is_satisfied_by<java::G::extends<X...>, Y>;
};

// `? extends T` can not be assigned to, as we don't know the actual type of `?`.
template<typename... X, typename Y>
struct is_assignable_<java::G::extends_t<X...>, Y> {
  static_assert(is_generic_v<Y>);

  using type = std::false_type;
};

// `? super T` can be assigned to, if it can be assigned to `T`.
// Otherwise, we can not be certain as we don't know the actual type of `?`.
template<typename... X, typename Y>
struct is_assignable_<java::G::super_t<X...>, Y>
: is_assignable<java::G::is<X...>, Y>
{};

template<typename... X, typename Y>
struct is_assignable_<java::G::pack_t<X...>, Y> {
  using type = typename std::conjunction<is_assignable<X, Y>...>;
};


template<>
struct is_java_primitive_<java::boolean_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::byte_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::short_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::int_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::long_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::float_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::double_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::char_t>
: std::true_type
{};

template<>
struct is_java_primitive_<java::void_t>
: std::true_type
{};

} /* namespace java::type_traits::<unnamed> */
} /* namespace java::type_traits */

#endif /* JAVA_TYPE_TRAITS_H */
