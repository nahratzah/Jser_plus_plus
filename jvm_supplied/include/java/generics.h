#ifndef JAVA_GENERICS_H
#define JAVA_GENERICS_H

///\file
///\brief Types to describe java generics.

namespace java::_tags::java::lang {
struct Object;
};

#include <cstddef>
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

template<typename T>
struct is_compact_generic_<T*, true>
: is_compact_generic_<T>
{};

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

template<typename X, typename Y>
constexpr bool is_satisfied_by_v = is_satisfied_by<X, Y>::value;

} /* namespace java::type_traits */

///\brief Namespace for generics.
namespace java::G {
namespace detail {

template<typename Tag>
struct generics_arity_
: std::integral_constant<std::size_t, Tag::generics_arity>
{};

template<>
struct generics_arity_<java::_tags::java::lang::Object>
: std::integral_constant<std::size_t, 0u>
{};

} /* namespace java::G::detail */


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
  static_assert(!java::type_traits::is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
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
  static_assert(!java::type_traits::is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
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
  static_assert(!java::type_traits::is_generic_v<Tag>);
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
      "Incorrect number of generics arguments for type.");

  using type = is_t;
  using tag = Tag;
  using arguments = generics_arguments<Arguments...>;
};


namespace detail {

///\brief Helper type that computes an argument pack by repeatedly adding elements from Tail.
///\tparam T An argument pack. The pack should be compact.
///\tparam Tail Multiple elements to be added. Each should be a generic or `void`. (`void` elements are dropped.)
template<typename R, typename... Tail>
struct combine;

} /* namespace java::G::detail */


template<typename Tag, typename... Arguments>
struct extends_t<is_t<Tag, Arguments...>>
: public extends_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct super_t<is_t<Tag, Arguments...>>
: public super_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct is_t<is_t<Tag, Arguments...>>
: public is_t<Tag, Arguments...>
{};


template<typename Tag, typename... Arguments>
struct extends_t<extends_t<Tag, Arguments...>>
: public extends_t<Tag, Arguments...>
{};

#if 0 // Omit: `? super ? extends ...` is not resolvable (static_assert in base case will trip).
template<typename Tag, typename... Arguments>
struct super_t<extends_t<typename Tag, typename... Arguments>>
{};
#endif

template<typename Tag, typename... Arguments>
struct is_t<extends_t<Tag, Arguments...>>
: public extends_t<Tag, Arguments...>
{};


#if 0 // Omit: `? extends ? super ...` is not resolvable (static_assert in base case will trip).
template<typename Tag, typename... Arguments>
struct extends_t<super_t<typename Tag, typename... Arguments>>
{};
#endif

template<typename Tag, typename... Arguments>
struct super_t<super_t<Tag, Arguments...>>
: public super_t<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct is_t<super_t<Tag, Arguments...>>
: public super_t<Tag, Arguments...>
{};


template<typename T>
struct is_t<T*> {
  using type = T*;
};

template<typename T>
struct extends_t<T*>
: is_t<T*> // Arrays are final, thus `? extends T[]` is the same as `? is T[]`.
{};


/**
 * \brief Multiple generics constraints pack.
 * \details Contains zero or more generics.
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename... G>
struct pack_t {
  static_assert(std::conjunction_v<java::type_traits::is_generic<G>...>);

  using type = pack_t<typename G::type...>;
};

// Specialization that unpacks single-element.
template<typename G>
struct pack_t<G> {
  static_assert(java::type_traits::is_generic_v<G>);

  using type = G;
};

template<typename... G>
struct extends_t<pack_t<G...>>
: public pack_t<extends_t<G>...>
{};

template<typename... G>
struct super_t<pack_t<G...>>
: public pack_t<super_t<G>...>
{};

template<typename... G>
struct is_t<pack_t<G...>>
: public pack_t<is_t<G>...>
{};


template<typename... T>
using extends = typename extends_t<T...>::type;

template<typename... T>
using super = typename super_t<T...>::type;

template<typename... T>
using is = typename is_t<T...>::type;

template<typename... T>
using pack = typename detail::combine<pack_t<>, T...>::type::type; // Double type is needed here: first one selects result of combine operation, which is a pack. Second allows pack_t to perform minimizing operation.


} /* namespace java::G */


// Includes to satisfy implementations of type traits.
#include <java/_accessor.h>


namespace java::type_traits {
namespace {

template<typename T>
struct is_generic_<T*>
: is_generic_<T>
{};

template<typename Tag, typename... Args>
struct is_generic_<java::G::is_t<Tag, Args...>> {
  using type = std::true_type;
};

template<typename Tag, typename... Args>
struct is_generic_<java::G::extends_t<Tag, Args...>> {
  using type = std::true_type;
};

template<typename Tag, typename... Args>
struct is_generic_<java::G::super_t<Tag, Args...>> {
  using type = std::true_type;
};

template<typename... G>
struct is_generic_<java::G::pack_t<G...>> {
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
  using type = std::disjunction<is_satisfied_by<G::is<X...>, Y>...>;
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

// Everything extends java.lang.Object.
// Ambiguity case.
template<>
struct is_satisfied_by_<java::G::extends_t<java::_tags::java::lang::Object>, java::G::extends_t<java::_tags::java::lang::Object>> {
  using type = std::true_type;
};

// Everything extends java.lang.Object.
// Ambiguity case.
template<>
struct is_satisfied_by_<java::G::extends_t<java::_tags::java::lang::Object>, java::G::is_t<java::_tags::java::lang::Object>> {
  using type = std::true_type;
};

// Everything extends java.lang.Object.
// Ambiguity case.
template<typename YTag, typename... Y>
struct is_satisfied_by_<java::G::extends_t<java::_tags::java::lang::Object>, java::G::super_t<YTag, Y...>> {
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

namespace java::G::detail {

///\brief Attempt to merge two generics together based on their tag.
template<typename X, typename ToAdd>
struct merge_by_tag_ {
  static_assert(java::type_traits::is_compact_generic_v<X>
      && java::type_traits::is_compact_generic_v<ToAdd>);

  ///\brief Indicates if merge is possible.
  using success = std::false_type;
  ///\brief Holds the first type for failed merges, or the merge result for successful merges.
  using merged_type = X;
};

template<typename Tag, typename... LeftArgs, typename... RightArgs>
struct merge_by_tag_<is_t<Tag, LeftArgs...>, is_t<Tag, RightArgs...>> {
  using success = std::true_type;
  using merged_type = is_t<Tag, pack<LeftArgs, RightArgs>...>;
};

template<typename Tag, typename... LeftArgs, typename... RightArgs>
struct merge_by_tag_<extends_t<Tag, LeftArgs...>, extends_t<Tag, RightArgs...>> {
  using success = std::true_type;
  using merged_type = extends_t<Tag, pack<LeftArgs, RightArgs>...>;
};

template<typename Tag, typename... LeftArgs, typename... RightArgs>
struct merge_by_tag_<super_t<Tag, LeftArgs...>, super_t<Tag, RightArgs...>> {
  using success = std::true_type;
  using merged_type = super_t<Tag, pack<LeftArgs, RightArgs>...>;
};


// Add a single type to the pack.
// Performs all the complicated stuff, like merging, deduplication.
template<typename Pack, typename T>
struct add;

// Skip addition of void.
template<typename... X>
struct add<pack_t<X...>, void> {
  using type = pack_t<X...>;
};

// When adding a pack, unpack it first.
template<typename... X, typename... Y>
struct add<pack_t<X...>, pack_t<Y...>>
: combine<pack_t<X...>, Y...>
{};

// Simple addition, append Y to pack_t X.
template<typename X, typename Y>
struct add0_;

template<typename... X, typename Y>
struct add0_<pack_t<X...>, Y> {
  using type = pack_t<X..., Y>;
};

// Add a type Y to pack_ X.
// If Y is already satisfied by X, do nothing.
// Otherwise: merge any X with Y for which tags match.
// Otherwise: eliminate any X satisfied by Y, then perform simple addition.
template<typename... X, typename Y>
struct add<pack_t<X...>, Y> {
  // std::true_type if Y is satisfied.
  using already_satisfied = std::disjunction<java::type_traits::is_satisfied_by<Y, X>...>;

  // Use combine to compute lesser pack.
  using elim_satisfied = combine<
      pack_t<>,
      std::conditional_t<
          java::type_traits::is_satisfied_by_v<X, Y>,
          void,
          X>...>;

  // Result of addition.
  // The result is a pack_t.
  using type =
      std::conditional_t<
          // If Y is already satisfied by constraints in X...
          already_satisfied::value,
          // Then don't add Y.
          pack_t<X...>,
          // else:
          std::conditional_t<
              // If we can merge...
              std::disjunction_v<typename merge_by_tag_<X, Y>::success...>,
              // Merge same-tag X and Y into single entry.
              pack_t<typename merge_by_tag_<X, Y>::merged_type...>,
              // Else: liminate any X satisfied by Y, then append Y.
              typename add0_<typename elim_satisfied::type, Y>::type>>;
};


// Base case where there is nothing more to be added.
template<typename... R>
struct combine<pack_t<R...>> {
  using type = pack_t<R...>;
};

// Default addition case: use `add` to add a single element,
// and use recursion for the remaining elements.
template<typename... R, typename In, typename... Tail>
struct combine<pack_t<R...>, In, Tail...>
: combine<typename add<pack_t<R...>, In>::type, Tail...>
{};

} /* namespace java::G::detail */

#endif /* JAVA_GENERICS_H */
