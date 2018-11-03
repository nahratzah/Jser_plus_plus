#ifndef JAVA_TYPE_TRAITS_H
#define JAVA_TYPE_TRAITS_H

#include <type_traits>
#include <java/generics.h>
#include <java/primitives.h>
#include <java/fwd/java/lang/Object.tagfwd>
#include <java/fwd/java/lang/Cloneable.tagfwd>
#include <java/fwd/java/io/Serializable.tagfwd>

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

template<typename R, typename... T>
struct type_and_supertypes_;

template<typename TargetTag, typename G>
struct is_tagged_;

template<typename R, typename Tag, typename... Types>
struct find_tagged_type_;

template<typename T>
struct find_is_types_only_;

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


/**
 * \brief Resolve type T and all its supertypes.
 * \details
 * Create a \ref ::java::G::type_set_t "type set" containing the type \p T
 * and its supertypes, recursively.
 * Uses breadth-first search to populate the list.
 * \tparam T A java \ref ::java::G::is "is-generic",
 * or a \ref java::G::type_set_t "type set" containing zero or more
 * \ref ::java::G::is "is-generics".
 *
 * Example:
 * `java::util::ArrayList<T>` will yield:
 * \code
 * ::java::G::pack<  // Note: same types are merged by the pack type.
 *   java::util::ArrayList<T>,          // Input type.
 *
 *   java::util::AbstractList<T>,       // 1st parent of ArrayList.
 *   java::util::List<T>,               // 2nd parent of ArrayList.
 *   java::util::RandomAccess,          // 3rd parent of ArrayList.
 *   java::lang::Cloneable,             // 4th parent of ArrayList.
 *   java::io::Serializable<T>,         // 5th parent of ArrayList.
 *
 *   java::util::AbstractCollection<T>, // 1st parent of AbstractList.
 *   java::util::List<T>,               // 2nd parent of AbstractList.
 *
 *   java::util::Collection<T>,         // 1st parent of List.
 *
 *                                      // RandomAccess has no parents.
 *                                      // Cloneable has no parents.
 *                                      // Serializable has no parents.
 *
 *   java::lang::Object,                // 1st parent of AbstractCollection.
 *   java::util::Collection<T>,         // 2nd parent of AbstractCollection.
 *
 *   java::util::Collection<T>,         // 1st parent of List.
 *
 *   java::util::Iterable<T>            // 1st parent of Collection.
 *
 *                                      // Object has no parents.
 *
 *                                      // Iterable has no parents.
 * >
 * \endcode
 */
template<typename T>
using type_and_supertypes = type_and_supertypes_<java::G::type_set_t<>, T>;

///\copydoc type_and_supertypes
template<typename T>
using type_and_supertypes_t = typename type_and_supertypes<T>::type;


/**
 * \brief Test if the given non-pack generic is tagged with the given tag.
 * \details
 * A non-pack generic is one of
 * \ref ::java::G::is_t "is",
 * \ref ::java::G::extends_t "extends",
 * or \ref ::java::G::super_t "super".
 *
 * \note Arrays don't have tags.
 * \tparam TargetTag The tag to look for.
 * \tparam G A non-pack generic.
 */
template<typename TargetTag, typename G>
using is_tagged = typename is_tagged_<TargetTag, G>::type;

///\copydoc is_tagged
template<typename TargetTag, typename G>
constexpr bool is_tagged_v = is_tagged<TargetTag, G>::value;

/**
 * \brief Find a type with the given tag.
 * \details
 * Will contain the type with the given tag.
 * If no type matches the tag, this will hold an empty \ref ::java::G::pack_t "argument pack".
 * If multiple types match the tag, they will be merged.
 *
 * \note The search will ignore \ref ::java::G::super_t "super-generics".
 * It will also not contain `java.lang.Object` unless one of the types derives from it (aka is not an interface).
 *
 * \tparam Tag The tag of the type to find.
 * \tparam Types Zero or more types over which to search.
 * This may contains \ref ::java::G::pack_t "argument packs" and
 * \ref ::java::G::type_set_t "type sets".
 */
template<typename Tag, typename... Types>
using find_tagged_type = find_tagged_type_<::java::G::type_set_t<>, Tag, Types...>;

///\copydoc find_tagged_type
template<typename Tag, typename... Types>
using find_tagged_type_t = typename find_tagged_type<Tag, Types...>::type;


/**
 * \brief Find the combined base type with a given tag.
 * \details
 * This type basically searches for the base type with the given tag.
 * Any type satisfying the input types will satisfy the returned base type.
 *
 * Will contain the type with the given tag.
 * If no type matches the tag, this will hold an empty \ref ::java::G::pack_t "argument pack".
 * If multiple types match the tag, they will be merged.
 *
 * \note The search will ignore \ref ::java::G::super_t "super-generics".
 * It will also not contain `java.lang.Object` unless one of the types derives from it (aka is not an interface).
 *
 * \tparam Tag The tag of the type to find.
 * \tparam Types Zero or more types over which to search.
 * This may contains \ref ::java::G::pack_t "argument packs" and
 * \ref ::java::G::type_set_t "type sets".
 */
template<typename Tag, typename... Types>
using find_basetype_with_tag = find_tagged_type<Tag, type_and_supertypes_t<Types>...>;

///\copydoc find_basetype_with_tag
template<typename Tag, typename... Types>
using find_basetype_with_tag_t = typename find_basetype_with_tag<Tag, Types...>::type;


/**
 * \brief Test if the given type has a base type with the given tag.
 * \details
 * This type tests if the given type derives from a type identified by tag \p Tag.
 *
 * \note The search will ignore \ref ::java::G::super_t "super-generics".
 * It will also not contain `java.lang.Object` unless one of the types derives from it (aka is not an interface).
 */
template<typename Tag, typename... Types>
using has_basetype_with_tag = std::negation<std::is_same<
    ::java::G::pack_t<>,
    find_basetype_with_tag_t<Tag, Types...>>>;

///\copydoc has_basetype_with_tag
template<typename Tag, typename... Types>
constexpr bool has_basetype_with_tag_v = has_basetype_with_tag<Tag, Types...>::value;


/**
 * \brief Filter the types in this type, such that only \ref ::java::G::is_t "is-types" remain.
 * \tparam T The type to search.
 */
template<typename T>
using find_is_types_only = find_is_types_only_<T>;

///\copydoc find_is_types_only
template<typename T>
using find_is_types_only_t = typename find_is_types_only<T>::type;


namespace {

template<typename G>
struct param_convert_<const G> {
  using type = const typename param_convert_<G>::type;
};

template<typename G>
struct param_convert_<G*> {
  static_assert(is_generic_v<G> || is_java_primitive_v<G>,
      "Array type conversion requires generics or primitives.");

  using type = G*;
};

template<typename... X>
struct param_convert_<java::G::super_t<X...>> {
  using type = java::G::is<X...>;
};

template<typename... X>
struct param_convert_<java::G::pack_t<X...>> {
  using type = java::G::pack<typename param_convert_<X>::type...>;
};


template<typename G>
struct return_type_convert_<const G> {
  using type = const typename return_type_convert_<G>::type;
};

template<typename G>
struct return_type_convert_<G*> {
  static_assert(is_generic_v<G> || is_java_primitive_v<G>,
      "Array type conversion requires generics or primitives.");

  using type = G*;
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

template<typename X, typename Y>
struct is_assignable_<X*, Y> {
  using type = is_satisfied_by<X*, Y>;
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


// Sentinel, when there are no more remaining types.
// (Nothing to be done, if there is no type, there are no type and supertypes.)
template<typename... Types>
struct type_and_supertypes_<::java::G::type_set_t<Types...>> {
  using type = ::java::G::type_set_t<Types...>;
};

// Case for adding one or more types from a pack.
template<typename... Types, typename... Pack, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::pack_t<Pack...>, Tail...>
: type_and_supertypes_<::java::G::type_set_t<Types...>, Pack..., Tail...>
{};

// Case for adding one or more types from a type set.
template<typename... Types, typename... Pack, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::type_set_t<Pack...>, Tail...>
: type_and_supertypes_<::java::G::type_set_t<Types...>, Pack..., Tail...>
{};

// Case for adding an is_t type.
template<typename... Types, typename Tag, typename... Args, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::is_t<Tag, Args...>, Tail...>
: type_and_supertypes_<
    ::java::G::type_set<Types..., ::java::G::is_t<Tag, Args...>>,
    Tail...,
    typename Tag::template parent_types<Args...>>
{};

// Case for adding an extends_t type.
template<typename... Types, typename Tag, typename... Args, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::extends_t<Tag, Args...>, Tail...>
: type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::is_t<Tag, Args...>, Tail...>
{};

// Case for adding a super_t type (it is dropped).
template<typename... Types, typename Tag, typename... Args, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, ::java::G::super_t<Tag, Args...>, Tail...>
: type_and_supertypes_<::java::G::type_set_t<Types...>, Tail...>
{};

// Case for adding a pointer type.
template<typename... Types, typename T, typename... Tail>
struct type_and_supertypes_<::java::G::type_set_t<Types...>, T*, Tail...>
: type_and_supertypes_<
    ::java::G::type_set<Types..., T*>,
    Tail...,
    ::java::G::is_t<::java::_tags::java::lang::Object>,
    ::java::G::is_t<::java::_tags::java::lang::Cloneable>,
    ::java::G::is_t<::java::_tags::java::io::Serializable>>
{};


template<typename TargetTag, typename Tag, typename... Arguments>
struct is_tagged_<TargetTag, ::java::G::is_t<Tag, Arguments...>>
: std::is_same<TargetTag, Tag>
{};

template<typename TargetTag, typename Tag, typename... Arguments>
struct is_tagged_<TargetTag, ::java::G::extends_t<Tag, Arguments...>>
: std::is_same<TargetTag, Tag>
{};

template<typename TargetTag, typename Tag, typename... Arguments>
struct is_tagged_<TargetTag, ::java::G::super_t<Tag, Arguments...>>
: std::is_same<TargetTag, Tag>
{};


template<typename... R, typename Tag>
struct find_tagged_type_<::java::G::type_set_t<R...>, Tag> {
  using type = ::java::G::pack<R...>;
};

template<typename... R, typename Tag, typename T0, typename... Types>
struct find_tagged_type_<::java::G::type_set_t<R...>, Tag, T0, Types...>
: std::conditional_t<
    is_tagged_<Tag, T0>::value,
    find_tagged_type_<::java::G::type_set_t<R..., T0>, Tag, Types...>,
    find_tagged_type_<::java::G::type_set_t<R...>, Tag, Types...>>
{};

template<typename... R, typename Tag, typename... Pack, typename... Types>
struct find_tagged_type_<::java::G::type_set_t<R...>, Tag, ::java::G::pack_t<Pack...>, Types...>
: find_tagged_type_<::java::G::type_set_t<R...>, Tag, Pack..., Types...>
{};

template<typename... R, typename Tag, typename... Pack, typename... Types>
struct find_tagged_type_<::java::G::type_set_t<R...>, Tag, ::java::G::type_set_t<Pack...>, Types...>
: find_tagged_type_<::java::G::type_set_t<R...>, Tag, Pack..., Types...>
{};


template<typename Tag, typename... Arguments>
struct find_is_types_only_<::java::G::is_t<Tag, Arguments...>> {
  using type = ::java::G::is_t<Tag, Arguments...>;
};

template<typename Tag, typename... Arguments>
struct find_is_types_only_<::java::G::extends_t<Tag, Arguments...>> {
  using type = ::java::G::pack_t<>;
};

template<typename Tag, typename... Arguments>
struct find_is_types_only_<::java::G::super_t<Tag, Arguments...>> {
  using type = ::java::G::pack_t<>;
};

template<typename T>
struct find_is_types_only_<T*>
: std::enable_if<is_generic_v<T*>, T*>
{};

template<typename... T>
struct find_is_types_only_<::java::G::pack_t<T...>> {
  using type = ::java::G::pack<typename find_is_types_only_<T>::type...>;
};

template<typename... T>
struct find_is_types_only_<::java::G::type_set_t<T...>> {
  using type = ::java::G::pack<typename find_is_types_only_<T>::type...>;
};

} /* namespace java::type_traits::<unnamed> */
} /* namespace java::type_traits */

#include <java/fwd/java/lang/Object.tag>
#include <java/fwd/java/lang/Cloneable.tag>
#include <java/fwd/java/io/Serializable.tag>

#endif /* JAVA_TYPE_TRAITS_H */
