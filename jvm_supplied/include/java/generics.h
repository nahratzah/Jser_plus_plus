#ifndef JAVA_GENERICS_H
#define JAVA_GENERICS_H

///\file
///\brief Types to describe java generics.

#include <type_traits>

namespace java::type_traits {
namespace {

///\brief Type trait that tests if \p T is a generic.
template<typename T>
struct is_generic_ {
  using type = std::false_type;
};

///\brief Type trait that tests if \p T is a compacted generic.
template<typename T, bool = is_generic_<T>::type::value>
struct is_compact_generic_ {
  using type = std::false_type;
};

template<typename T>
struct is_compact_generic_<T, true> {
  using type = std::is_same<T, typename T::type>;
};

///\brief Type trait that tests if X is satisfied by Y.
///\details Used for types.
///
///For example, `List<? extends CharSequence>` is satisfied by `List<String>`.
///
///\attention
///`List<? extends CharSequence>` is not satisfied by `ArrayList<String>`.
///But `? extends List<? extends CharSequence>` is satisfied by `ArrayList<String>`.
template<typename X, typename Y>
struct is_satisfied_by_;

} /* namespace java::type_traits::<unnamed> */

template<typename T>
using is_generic = typename is_generic_<T>::type;

template<typename T>
constexpr bool is_generic_v = is_generic<T>::value;

template<typename T>
using is_compact_generic = typename is_compact_generic_<T>::type;

template<typename T>
constexpr bool is_compact_generic_v = is_compact_generic<T>::value;

template<typename X, typename Y>
using is_satisfied_by = typename is_satisfied_by_<typename X::type, typename Y::type>::type;

template<typename T>
constexpr bool is_satisfied_by_v = is_satisfied_by<T>::value;

} /* namespace java::type_traits */

///\brief Namespace for generics.
namespace java::G {


///\brief Template that holds multiple type-arguments to a type.
///\details
///Holds all arguments to a generics type.
///
///For example, for a type `Map<String, ? extends Integer>`,
///this would hold `String, ? extends Integer`.
template<typename... Arg>
struct generics_arguments {
  static_assert(std::conjunction_v<java::type_traits::is_generic<Arg>...>);
};


/**
 * \brief Generic declaration.
 * \detail
 * This generics declaration models the relation:
 * \code[.java]
 * ? extends Tag<Arguments>
 * \endcode
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename Tag, typename... Arguments>
struct extends_t {
  static_assert(!is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == Tag::generics_arity,
      "Incorrect number of generics arguments for type.");

  using type = extends_t;
  using tag = Tag;
  using arguments = generics_arguments<Arguments...>;
};

/**
 * \brief Generic declaration.
 * \detail
 * This generics declaration models the relation:
 * \code[.java]
 * ? super Tag<Arguments>
 * \endcode
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename Tag, typename... Arguments>
struct super_t {
  static_assert(!is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == Tag::generics_arity,
      "Incorrect number of generics arguments for type.");

  using type = super_t;
  using tag = Tag;
  using arguments = generics_arguments<Arguments...>;
};

/**
 * \brief Generic declaration.
 * \detail
 * This generics declaration models the relation:
 * \code[.java]
 * Tag<Arguments>
 * \endcode
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename Tag, typename... Arguments>
struct is_t {
  static_assert(!is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == Tag::generics_arity,
      "Incorrect number of generics arguments for type.");

  using type = is_t;
  using tag = Tag;
  using arguments = generics_arguments<Arguments...>;
};


template<typename Tag, typename... Arguments>
struct extends_t<is_t<typename Tag, typename... Arguments>>
: public extends_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct super_t<is_t<typename Tag, typename... Arguments>>
: public super_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct is_t<is_t<typename Tag, typename... Arguments>>
: public is_t<Tag, Arguments...>
{};


template<typename Tag, typename... Arguments>
struct extends_t<extends_t<typename Tag, typename... Arguments>>
: public extends_t<Tag, Arguments...>
{};

#if 0 // Omit: `? super ? extends ...` is not resolvable (static_assert in base case will trip).
template<typename Tag, typename... Arguments>
struct super_t<extends_t<typename Tag, typename... Arguments>>
{};
#endif

template<typename Tag, typename... Arguments>
struct is_t<extends_t<typename Tag, typename... Arguments>>
: public extends_t<Tag, Arguments...>
{};


#if 0 // Omit: `? extends ? super ...` is not resolvable (static_assert in base case will trip).
template<typename Tag, typename... Arguments>
struct extends_t<super_t<typename Tag, typename... Arguments>>
{};
#endif

template<typename Tag, typename... Arguments>
struct super_t<super_t<typename Tag, typename... Arguments>>
: public super_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct is_t<super_t<typename Tag, typename... Arguments>>
: public super_t<Tag, Arguments...>
{};


/**
 * \brief Multiple generics constraints pack.
 * \details Contains zero or more generics.
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename... G>
struct pack {
  static_assert(std::conjunction_v<is_generic<G>...>);

  using type = pack_t<typename G::type...>;
};

template<typename... G>
struct extends_t<pack_t<G...>>
: using pack_t<extends_t<G>...>
{};

template<typename... G>
struct super_t<pack_t<G...>>
: using pack_t<super_t<G>...>
{};

template<typename... G>
struct is_t<pack_t<G...>>
: using pack_t<is_t<G>...>
{};


template<typename... T>
using extends = typename extends_t<T...>::type;

template<typename... T>
using super = typename super_t<T...>::type;

template<typename... T>
using is = typename is_t<T...>::type;

template<typename... T>
using pack = typename pack_t<T...>::type;


} /* namespace java::G */


// Includes to satisfy implementations of type traits.
#include <java/lang/Object.h>
#include <java/_accessor.h>


namespace java::type_traits {
namespace {

template<typename Tag, typename... Args>
struct is_generic_<java::G::is_t<Tag, Args...> {
  using type = std::true_type;
};

template<typename Tag, typename... Args>
struct is_generic_<java::G::extends_t<Tag, Args...> {
  using type = std::true_type;
};

template<typename Tag, typename... Args>
struct is_generic_<java::G::super_t<Tag, Args...> {
  using type = std::true_type;
};

template<typename... G>
struct is_generic_<java::G::pack_t<G...> {
  using type = std::true_type;
};


// Comparison with argument pack on the left.
template<typename... X, typename Y>
struct is_satisfied_by_<java::G::pack_t<X...>, Y> {
  static_assert(is_generic_v<Y>);

  using type = std::conjunction<is_satisfied_by<X, Y>...>;
};

// Comparison with argument pack on the right.
template<typename... X, typename... Y>
struct is_satisfied_by_<java::G::is_t<X...>, java::G::pack_t<Y...>> {
  using type = std::disjunction<is_satisfied_by<is<X...>, Y>...>;
};

// Comparison with argument pack on the right.
template<typename... X, typename... Y>
struct is_satisfied_by_<java::G::extends_t<X...>, java::G::pack_t<Y...>> {
  using type = std::disjunction<is_satisfied_by<java::G::extends_t<X...>, Y>...>;
};

// Comparison with argument pack on the right.
template<typename... X, typename... Y>
struct is_satisfied_by_<java::G::super_t<X...>, java::G::pack_t<Y...>> {
  using type = std::disjunction<is_satisfied_by<java::G::super_t<X...>, Y>...>;
};

// Everything is a java.lang.Object.
template<typename... Y>
struct is_satisfied_by_<java::G::is_t<java::_tags::java::lang::Object>, java::G::pack_t<Y...>> {
  using type = std::true_type;
};

// Everything is a java.lang.Object.
template<typename Y>
struct is_satisfied_by_<java::G::is_t<java::_tags::java::lang::Object>, Y> {
  static_assert(is_generic_v<Y>);

  using type = std::true_type;
};

// Everything extends java.lang.Object.
template<typename... Y>
struct is_satisfied_by_<java::G::extends_t<java::_tags::java::lang::Object>, java::G::pack_t<Y...>> {
  using type = std::true_type;
};

// Everything extends java.lang.Object.
template<typename Y>
struct is_satisfied_by_<java::G::extends_t<java::_tags::java::lang::Object>, Y> {
  static_assert(is_generic_v<Y>);

  using type = std::true_type;
};

// G::is only works if tags match on both sides (non-matching case).
// Example: List<CharSequence> is not satisfied by List<String>, nor List<Object>
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::is_t<XTag, X...>, java::G::is_t<YTag, Y...>> {
  using type = std::false_type;
};

// G::is only works if tags match on both sides.
// Example: List<CharSequence> is not satisfied by List<String>
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::is_t<XTag, X...>, java::G::extends_t<YTag, Y...>> {
  using type = std::false_type;
};

// G::is only works if tags match on both sides.
// Example: List<CharSequence> is not satisfied by List<Object>
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::is_t<XTag, X...>, java::G::super_t<YTag, Y...>> {
  using type = std::false_type;
};

// G::is only works if tags match on both sides (matching case).
// Example: List<CharSequence> is satisfied by List<CharSequence>
template<typename Tag, typename... X, typename... Y>
struct is_satisfied_by_<java::G::is_t<Tag, X...>, java::G::is_t<Tag, Y...>> {
  using type = std::conjunction<is_satisfied_by<X, Y>...>;
};

///\brief Helper for is_satisfied_by_ with G::extends on the left hand side.
template<typename X, bool Implemented>
struct is_satisfied_by__aft_ {
  template<typename Accessor>
  using test = std::false_type;
};

///\brief Given an accessor, extract the G::is, G::extends, and G::super types.
template<typename Accessor>
struct accessor_type_;

///\brief Helper for is_satisfied_by, that uses `X::tag` to figure out the matching type from an accessor, then invokes is_satisfied.
template<typename X>
struct is_satisfied_by__aft_<X, true> {
  template<typename Accessor>
  using test = is_satisfied_by<X, typename accessor_type_<accessor_for_tag_t<typename X::tag, Accessor>>::is_type>;
};

///\brief Minimalistic base type for accessors.
struct _dummy_accessor_base {
  template<typename T> auto ref_() const -> T&;
};

template<typename Base, typename Tag, typename... Args>
struct accessor_type_<java::_accessor<Base, Tag, Args...>> {
  using is_type = java::G::is<Tag, Args...>;
  using super_type = java::G::super<Tag, Args...>;
  using extends_type = java::G::extends<Tag, Args...>;
};

// G::extends<X...> is satisfied by an G::is<Y...>, that has the same super type
// and where that super type satisfies G::is<X...>
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::extends_t<XTag, X...>, java::G::is_t<YTag, Y...>> {
  using x_accessor = java::_accessor<_dummy_accessor_base, XTag, X...>;
  using y_accessor = java::_accessor<_dummy_accessor_base, YTag, Y...>;
  static_assert(sizeof(x_accessor) > 0 && sizeof(y_accessor) > 0, "Accessors must be complete types.");

  using type = typename is_satisfied_by__aft_<java::G::is<XTag, X...>, implements_tag_v<XTag, y_accessor>>
      ::template test<y_accessor>;
};

// G::extends<X...> is satisfied by any G::extends<Y...>, for which it is satisfied by G::is<Y...>.
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::extends_t<XTag, X...>, java::G::extends_t<YTag, Y...>>
: public is_satisfied_by_<java::G::extends<XTag, X...>, java::G::is<YTag, Y...>>
{};

// G::extends can never be satisfied by G::super.
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::extends_t<XTag, X...>, java::G::super_t<YTag, Y...>> {
  using type = std::false_type;
};

// G::super<X...> is satisfied by G::is<Y...>, if G::extends<Y...> is satisfied by G::is<X...>
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::super_t<XTag, X...>, java::G::is_t<YTag, Y...>>
: public is_satisfied_by_<java::G::extends<YTag, Y...>, java::G::is<XTag, X...>>
{};

// G::super<X...> is satisfied by G::super<Y...>, for which it is satisfied by G::is<Y...>.
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::super_t<XTag, X...>, java::G::super_t<YTag, Y...>>
: public is_satisfied_by_<java::G::super<XTag, X...>, java::G::is<YTag, Y...>>
{};

// G::super can never be satisfied by G::extends.
template<typename XTag, typename... X, typename YTag, typename... Y>
struct is_satisfied_by_<java::G::super_t<XTag, X...>, java::G::extends_t<YTag, Y...>> {
  using type = std::false_type;
};

} /* namespace java::type_traits::<unnamed> */
} /* namespace java::type_traits */

#endif /* JAVA_GENERICS_H */
