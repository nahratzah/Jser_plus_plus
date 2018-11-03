#ifndef JAVA_GENERICS_H
#define JAVA_GENERICS_H

///\file
///\brief Types to describe java generics.

#include <cstddef>
#include <type_traits>
#include <java/generics_arity_.h>
#include <java/fwd/java/lang/Object.tagfwd>
#include <java/fwd/java/io/Serializable.tagfwd>
#include <java/fwd/java/lang/Cloneable.tagfwd>

namespace java::type_traits {
namespace {

// Forward declaration of is_java_primitive_ (implemented in java/type_traits.h)
template<typename>
struct is_java_primitive_;

///\brief Type trait that tests if \p T is a generic.
template<typename T>
struct is_generic_
: std::false_type
{};

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
///Invocation is: `is_satisfied_by_<X>::template test<Y>::type`
///
///For example, `List<? extends CharSequence>` is satisfied by `List<String>`.
///
///\attention
///`List<? extends CharSequence>` is not satisfied by `ArrayList<String>`.
///But `? extends List<? extends CharSequence>` is satisfied by `ArrayList<String>`.
template<typename X>
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
using is_satisfied_by = typename is_satisfied_by_<X>::template test<Y>::type;

template<typename X, typename Y>
constexpr bool is_satisfied_by_v = is_satisfied_by<X, Y>::value;

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

// Specialization for array.
template<typename T>
struct extends_t<T*> {
  using type = extends_t;
  using tag = T*;
  using arguments = generics_arguments<>;
};

// Specialization for array.
template<typename T>
struct super_t<T*> {
  using type = super_t;
  using tag = T*;
  using arguments = generics_arguments<>;
};

// Specialization for array.
template<typename T>
struct is_t<T*> {
  using type = is_t;
  using tag = T*;
  using arguments = generics_arguments<>;
};

/**
 * \brief Multiple generics constraints pack.
 * \details Contains zero or more generics.
 *
 * \note Generics are incomplete types, used for tagging purposes only.
 */
template<typename... G>
struct pack_t {
  static_assert(std::conjunction_v<java::type_traits::is_generic<G>...>);
  static_assert(sizeof...(G) != 1u);

  using type = pack_t;
};


namespace detail {

///\brief Hold the result of combine.
template<typename... T>
struct combine_result {
  static_assert(std::conjunction_v<java::type_traits::is_generic<T>...>);
  static_assert(sizeof...(T) != 1u);

  using type = pack_t<T...>;
};

///\brief Hold the result of combine.
template<typename T>
struct combine_result<T> {
  static_assert(java::type_traits::is_generic_v<T>);

  using type = T;
};

///\brief Helper type that computes an argument pack by repeatedly adding elements from Tail.
///\tparam T An argument pack. The pack should be compact.
///\tparam Tail Multiple elements to be added. Each should be a generic or `void`. (`void` elements are dropped.)
template<typename R, typename... Tail>
struct combine;

template<typename Tag, typename... Arguments>
struct make_is_ {
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
      "Incorrect number of generics arguments for type.");
  static_assert(std::conjunction_v<::java::type_traits::is_generic<Arguments>...>,
      "Arguments must be generics types.");

  using type = typename Tag::template is_t<Arguments...>;
};

template<typename Tag, typename... Arguments>
struct make_extends_ {
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
      "Incorrect number of generics arguments for type.");
  static_assert(std::conjunction_v<::java::type_traits::is_generic<Arguments>...>,
      "Arguments must be generics types.");

  using type = typename Tag::template extends_t<Arguments...>;
};

template<typename Tag, typename... Arguments>
struct make_super_ {
  static_assert(sizeof...(Arguments) == detail::generics_arity_<Tag>::value,
      "Incorrect number of generics arguments for type.");
  static_assert(std::conjunction_v<::java::type_traits::is_generic<Arguments>...>,
      "Arguments must be generics types.");

  using type = typename Tag::template super_t<Arguments...>;
};

template<typename Tag, typename... Arguments>
struct make_is_<is_t<Tag, Arguments...>>
: make_is_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_is_<extends_t<Tag, Arguments...>>
: make_extends_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_is_<super_t<Tag, Arguments...>>
: make_super_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_extends_<is_t<Tag, Arguments...>>
: make_extends_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_extends_<extends_t<Tag, Arguments...>>
: make_extends_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_extends_<super_t<Tag, Arguments...>>; // Unresolvable.

template<typename Tag, typename... Arguments>
struct make_super_<is_t<Tag, Arguments...>>
: make_super_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_super_<super_t<Tag, Arguments...>>
: make_super_<Tag, Arguments...>
{};

template<typename Tag, typename... Arguments>
struct make_super_<extends_t<Tag, Arguments...>>; // Unresolvable.

template<typename T>
struct make_is_<T*> {
  static_assert(::java::type_traits::is_generic_v<T*>);

  using type = T*;
};

// Arrays can not be a base class, so is_t and extends_t are equivalent.
template<typename T>
struct make_extends_<T*>
: make_is_<T*>
{};

template<typename... G>
struct make_pack_
: detail::combine<detail::combine_result<>, G...>::type // a combine_result
{};

template<typename G>
struct make_pack_<G> {
  static_assert(java::type_traits::is_generic_v<G>);

  using type = G;
};

template<>
struct make_pack_<> {
  using type = ::java::G::pack_t<>;
};

template<typename... T>
struct make_is_<pack_t<T...>>
: make_pack_<typename make_is_<T>::type...>
{};

template<typename... T>
struct make_extends_<pack_t<T...>>
: make_pack_<typename make_extends_<T>::type...>
{};

template<typename... T>
struct make_super_<pack_t<T...>>
: make_pack_<typename make_super_<T>::type...>
{};

template<typename... T>
struct make_pack_<pack_t<T...>>
: make_pack_<T...>
{};

} /* namespace java::G::detail */


template<typename... T>
using extends = typename ::java::G::detail::make_extends_<T...>::type;

template<typename... T>
using super = typename ::java::G::detail::make_super_<T...>::type;

template<typename... T>
using is = typename ::java::G::detail::make_is_<T...>::type;

template<typename... T>
using pack = typename ::java::G::detail::make_pack_<T...>::type;


} /* namespace java::G */


// Includes to satisfy implementations of type traits.
#include <java/_accessor.h>
#include <java/fwd/java/lang/Object.tag>
#include <java/fwd/java/io/Serializable.tag>
#include <java/fwd/java/lang/Cloneable.tag>


namespace java::type_traits {
namespace {

template<typename T>
struct is_generic_<T*>
: std::disjunction<typename is_generic_<T>::type, typename is_java_primitive_<T>::type>
{};

template<typename Tag, typename... Args>
struct is_generic_<java::G::is_t<Tag, Args...>>
: std::true_type
{};

template<typename Tag, typename... Args>
struct is_generic_<java::G::extends_t<Tag, Args...>>
: std::true_type
{};

template<typename Tag, typename... Args>
struct is_generic_<java::G::super_t<Tag, Args...>>
: std::true_type
{};

template<typename... G>
struct is_generic_<java::G::pack_t<G...>>
: std::true_type
{};


template<typename X, typename... XArgs>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>> {
  template<typename Y>
  struct test;
};

template<typename X, typename... XArgs>
template<typename Y, typename... YArgs>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<::java::G::is_t<Y, YArgs...>>
: std::false_type
{};

template<typename X, typename... XArgs>
template<typename... YArgs>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<::java::G::is_t<X, YArgs...>>
: std::conjunction<is_satisfied_by<XArgs, YArgs>...>
{};

template<typename X, typename... XArgs>
template<typename... Y>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<::java::G::extends_t<Y...>>
: std::false_type
{};

template<typename X, typename... XArgs>
template<typename... Y>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<::java::G::super_t<Y...>>
: std::false_type
{};

template<typename X, typename... XArgs>
template<typename... Y>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<::java::G::pack_t<Y...>>
: std::disjunction<typename test<Y>::type...>
{};

template<typename X, typename... XArgs>
template<typename Y>
struct is_satisfied_by_<::java::G::is_t<X, XArgs...>>::test<Y*>
: std::false_type
{};


template<typename X, typename... XArgs>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>> {
  template<typename Y>
  struct test;
};

template<typename X, typename... XArgs>
template<typename Y, typename... YArgs>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>>::test<::java::G::is_t<Y, YArgs...>>
: std::disjunction<
    typename is_satisfied_by_<::java::G::is_t<X, XArgs...>>
        ::template test<::java::G::is_t<Y, YArgs...>>::type,
    typename test<typename Y::template parent_types<YArgs...>>::type>
{};

template<typename X, typename... XArgs>
template<typename Y, typename... YArgs>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>>::test<::java::G::extends_t<Y, YArgs...>>
: test<::java::G::is_t<Y, YArgs...>>
{};

template<typename X, typename... XArgs>
template<typename Y, typename... YArgs>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>>::test<::java::G::super_t<Y, YArgs...>>
: test<::java::_tags::java::lang::Object::is_t<>>
{};

template<typename X, typename... XArgs>
template<typename... Y>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>>::test<::java::G::pack_t<Y...>>
: std::disjunction<typename test<Y>::type...>
{};

template<typename X, typename... XArgs>
template<typename Y>
struct is_satisfied_by_<::java::G::extends_t<X, XArgs...>>::test<Y*>
: std::enable_if_t<
    is_generic_v<Y*>,
    std::disjunction<
        std::is_same<::java::_tags::java::lang::Object, X>,
        std::is_same<::java::_tags::java::io::Serializable, X>,
        std::is_same<::java::_tags::java::lang::Cloneable, X>>>
{};


// Object is extended by everything.
// (Curiously, it is by interfaces too, don't ask me how.)
template<>
struct is_satisfied_by_<::java::_tags::java::lang::Object::extends_t<>> {
  template<typename Y>
  using test = std::enable_if<is_generic_v<Y>, std::true_type>;
};


template<typename X, typename... XArgs>
struct is_satisfied_by_<::java::G::super_t<X, XArgs...>> {
  template<typename Y>
  struct test_;

  template<typename Y>
  struct test;
};

template<typename X, typename... XArgs>
template<typename Y>
struct is_satisfied_by_<::java::G::super_t<X, XArgs...>>::test_
: std::disjunction<
    typename is_satisfied_by_<::java::G::is_t<X, XArgs...>>
        ::template test<Y>::type,
    std::conjunction<
        std::negation<std::is_same<::java::G::pack_t<>, typename X::template parent_types<XArgs...>>>,
        typename is_satisfied_by_<typename X::template parent_types<XArgs...>>
            ::template test<Y>::type>>
{};

template<typename X, typename... XArgs>
template<typename Y, typename... YArgs>
struct is_satisfied_by_<::java::G::super_t<X, XArgs...>>::test_<::java::G::super_t<Y, YArgs...>>
: test_<::java::G::is_t<Y, YArgs...>>
{};

template<typename X, typename... XArgs>
template<typename Y>
struct is_satisfied_by_<::java::G::super_t<X, XArgs...>>::test
: std::disjunction<
    std::is_same<::java::_tags::java::lang::Object::is_t<>, Y>,
    std::is_same<::java::_tags::java::lang::Object::super_t<>, Y>,
    typename test_<Y>::type>
{};


template<typename... X>
struct is_satisfied_by_<::java::G::pack_t<X...>> {
  template<typename Y>
  using test = std::conjunction<typename is_satisfied_by_<X>::template test<Y>::type...>;
};


template<typename X>
struct is_satisfied_by_<X*> {
  struct primitive_test_ {
    template<typename Y>
    using type = typename ::std::is_same<X, Y>;
  };

  struct type_test_ {
    template<typename Y>
    using type = typename is_satisfied_by_<X>::template test<Y>::type;
  };

  template<typename Y>
  using test_ = typename ::std::conditional_t<
      is_java_primitive_<X>::value || is_java_primitive_<Y>::value,
      primitive_test_,
      type_test_>::template type<Y>;

  template<typename Y>
  struct test;
};

template<typename X>
template<typename Y>
struct is_satisfied_by_<X*>::test<Y*>
: test_<Y>::type
{};

template<typename X>
template<typename Y>
struct is_satisfied_by_<X*>::test
: std::enable_if_t<is_generic_v<Y>, std::false_type>
{};

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
struct add<combine_result<X...>, void> {
  using type = combine_result<X...>;
};

// When adding a pack, unpack it first.
template<typename... X, typename... Y>
struct add<combine_result<X...>, pack_t<Y...>>
: combine<combine_result<X...>, Y...>
{};

// Simple addition, append Y to pack_t X.
template<typename X, typename Y>
struct add0_;

template<typename... X, typename Y>
struct add0_<combine_result<X...>, Y> {
  using type = combine_result<X..., Y>;
};

// Add a type Y to pack_ X.
// If Y is already satisfied by X, do nothing.
// Otherwise: merge any X with Y for which tags match.
// Otherwise: eliminate any X satisfied by Y, then perform simple addition.
template<typename... X, typename Y>
struct add<combine_result<X...>, Y> {
  // std::true_type if Y is satisfied.
  using already_satisfied = std::disjunction<java::type_traits::is_satisfied_by<Y, X>...>;

  // Use combine to compute lesser pack.
  using elim_satisfied = combine<
      combine_result<>,
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
          combine_result<X...>,
          // else:
          std::conditional_t<
              // If we can merge...
              std::disjunction_v<typename merge_by_tag_<X, Y>::success...>,
              // Merge same-tag X and Y into single entry.
              combine_result<typename merge_by_tag_<X, Y>::merged_type...>,
              // Else: liminate any X satisfied by Y, then append Y.
              typename add0_<typename elim_satisfied::type, Y>::type>>;
};


// Base case where there is nothing more to be added.
template<typename... R>
struct combine<combine_result<R...>> {
  using type = combine_result<R...>;
};

// Default addition case: use `add` to add a single element,
// and use recursion for the remaining elements.
template<typename... R, typename In, typename... Tail>
struct combine<combine_result<R...>, In, Tail...>
: combine<typename add<combine_result<R...>, In>::type, Tail...>
{};

} /* namespace java::G::detail */

#endif /* JAVA_GENERICS_H */
