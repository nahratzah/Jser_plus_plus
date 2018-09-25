#ifndef JAVA__ACCESSOR_H
#define JAVA__ACCESSOR_H

namespace java {


/**
 * \brief Constructor object.
 * \details Contains zero or more functor methods `operator()` that will
 * construct an instance of the corresponding class.
 */
template<typename Tag, typename... Arguments>
struct _constructor;

/**
 * \brief Accessor for a specific type.
 * \details
 * Each type declares an accessor, which allows access to all public methods
 * and fields of the implementation type.
 * It uses the \p Arguments parameter-pack to provide the correct translation.
 *
 * Accessors must inherit from all base accessor types.
 *
 * An accessor shall define a private, pure virtual function, that it uses to
 * access its referenced type.
 * \code
 * private:
 * virtual auto ref_() const -> typename Tag::erased_type& = 0;
 * \endcode
 *
 * An accessor shall derive non-virtual from the accessor of it's super-object-type,
 * and shall have virtual inheritance of all of accessors of it's interface types.
 *
 * An accessor shall have only protected constructors, destructors, and copy/move assignment.
 *
 * An accessor shall not have member fields.
 * \note Only specializations of this type exist.
 * \tparam Base A type the _accessor must virtually inherit from, that provides a ref_() function.
 * \tparam Tag type identification tag.
 * \tparam Arguments generics arguments for the type.
 */
template<typename Base, typename Tag, typename... Arguments>
class _accessor;


/**
 * \brief Accessor for static fields and values of a specific type.
 * \details
 * Each type declares a static accessor, which allows access to all static
 * methods of the implementation type.
 *
 * A static accessor shall not derive from parent types.
 *
 * A static accessor shall only have protected constructors, destructors, and copy/move assignment.
 *
 * An accessor shall not have member fields.
 * \note Only specializations of this type exist.
 * \tparam Tag type identification tag.
 * \tparam Arguments generics arguments for the type.
 */
template<typename Tag, typename... Arguments>
class _static_accessor;


} /* namespace java */

namespace java::type_traits {
namespace {

/**
 * \brief Helper function that accepts an accessor via a supplied tag.
 * \details
 * To be invoked with only that tag specified:
 * \code
 * find_tag_<java::_tags::java::lang::Object>(some_accessor);
 * \endcode
 *
 * \tparam Tag The tag type for which to find the accessor.
 * \param accessor The accessor on which to find the specified tag type.
 * \returns The \p accessor type with the specified \p Tag.
 * The function has an overload which will return void, if the accessor does
 * not derive from an accessor with the specified \p Tag.
 */
template<typename Tag, typename Base, typename... Arguments>
inline auto find_tag_(const java::_accessor<Base, Tag, Arguments...>& accessor) noexcept
-> const java::_accessor<Base, Tag, Arguments...>& {
  return accessor;
}

// Specialization of the find_tag_, if the \p Tag is not present.
template<typename Tag>
inline auto find_tag_(...) noexcept
-> void {
}

template<typename Tag, typename Accessor>
struct implements_tag_ {
  static_assert(sizeof(Tag) > 0, "Tag must be a complete type.");
  static_assert(sizeof(Accessor) > 0, "Accessor must be a complete type.");

  using type = std::negation<std::is_same<void, decltype(find_tag_<Tag>(std::declval<Accessor>()))>>;
};

template<typename Tag, typename Accessor, bool = implements_tag_<Tag, Accessor>::type::value>
struct accessor_for_tag_ {
  static_assert(sizeof(Tag) > 0, "Tag must be a complete type.");
  static_assert(sizeof(Accessor) > 0, "Accessor must be a complete type.");
  // typedef `type` left undefined for SFINAE purposes.
};

template<typename Tag, typename Accessor>
struct accessor_for_tag_<Tag, Accessor, true> {
  static_assert(sizeof(Tag) > 0, "Tag must be a complete type.");
  static_assert(sizeof(Accessor) > 0, "Accessor must be a complete type.");

  using type = std::remove_cv_t<std::remove_reference_t<decltype(find_tag_<Tag>(std::declval<Accessor>()))>>;
};

} /* namespace java::type_traits::<unnamed> */

template<typename Tag, typename Accessor>
using implements_tag = typename implements_tag_<Tag, Accessor>::type;

template<typename Tag, typename Accessor>
constexpr bool implements_tag_v = implements_tag<Tag, Accessor>::value;

template<typename Tag, typename Accessor>
using accessor_for_tag = accessor_for_tag_<Tag, Accessor>;

template<typename Tag, typename Accessor>
using accessor_for_tag_t = typename accessor_for_tag<Tag, Accessor>::type;

} /* namespace java::type_traits */

#endif /* JAVA__ACCESSOR_H */
