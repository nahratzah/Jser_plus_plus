#ifndef JAVA__TEMPLATE_SELECTOR_H
#define JAVA__TEMPLATE_SELECTOR_H

#include <cstddef>
#include <type_traits>
#include <java/_accessor.h>
#include <java/generics.h>
#include <java/ref.h>
#include <java/fwd/java/lang/Object.h>

namespace java {
namespace {

template<typename T, ::std::size_t Extents>
struct _template_selector_array_t;

template<typename T>
struct _template_selector_array_t<T, 0u> {
  using type = T;
};

template<typename T, ::std::size_t Extents>
struct _template_selector_array_t<T*, Extents>
: _template_selector_array_t<T, Extents - 1u>
{};


template<typename GenericsArguments, ::std::size_t ArgumentIndex>
struct _template_selector_argidx_t;

template<typename T0, typename... T>
struct _template_selector_argidx_t<::java::G::generics_arguments<T0, T...>, 0u> {
  using type = T0;
};

template<typename T0, typename... T, ::std::size_t ArgumentIndex>
struct _template_selector_argidx_t<::java::G::generics_arguments<T0, T...>, ArgumentIndex>
: _template_selector_argidx_t<::java::G::generics_arguments<T...>, ArgumentIndex - 1u>
{};


template<typename Tag, ::std::size_t ArgumentIndex>
struct _template_selector_tmpl_t {
  static_assert(ArgumentIndex < ::java::G::detail::generics_arity_<Tag>::value,
      "Can only select arguments up to the argument length of the type.");

 private:
  template<typename G>
  struct accessor_for_;

  template<typename G>
  using accessor_for = typename accessor_for_<G>::type;

  template<typename G, bool Implemented = ::java::type_traits::implements_tag_v<Tag, accessor_for<G>>>
  struct select0_;

  template<typename G>
  struct select_;

 public:
  template<typename G>
  using type = typename _template_selector_argidx_t<typename select_<G>::type::arguments, ArgumentIndex>::type;
};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename... X>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::accessor_for_<::java::G::is_t<X...>> {
  // We borrow dummy_accessor_base_ from java/generics.h
  using type = ::java::_accessor<::java::type_traits::_dummy_accessor_base, X...>;
};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename... X>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::accessor_for_<::java::G::extends_t<X...>>
: accessor_for_<::java::G::is<X...>>
{};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename... X>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::accessor_for_<::java::G::super_t<X...>>
: accessor_for_<::java::G::is<::java::_tags::java::lang::Object>>
{};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename G>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::select0_<G, false> {
  using type_or_pack = ::java::G::pack<>;
};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename G>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::select0_<G, true> {
  using type = typename ::java::type_traits::accessor_type_<accessor_for<G>>::is_type;
  using type_or_pack = type;
};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename G>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::select_
: select0_<G>
{};

template<typename Tag, ::std::size_t ArgumentIndex>
template<typename... G>
struct _template_selector_tmpl_t<Tag, ArgumentIndex>::select_<::java::G::pack_t<G...>>
: ::std::enable_if<
    ::std::disjunction_v<::java::type_traits::implements_tag<Tag, accessor_for<G>>...>,
    ::java::G::pack<typename select0_<G>::type_or_pack...>>
{};


template<typename T>
struct _template_selector_ {
  ///\brief Select the generics for the resolved type.
  using type = T;
  ///\brief Select the variable for the resolved type.
  using var_type = var_ref<type>;

  ///\brief Perform array resolution.
  ///\tparam Extents Number of extents to remove from the array.
  template<::std::size_t Extents>
  using array = _template_selector_<typename _template_selector_array_t<type, Extents>::type>;

  ///\brief Perform class argument resolution.
  ///\tparam Tag Indentifier for the type on which to resolve the binding.
  ///\tparam ArgumentIndex Generics argument index (zero based) for the type.
  template<typename Tag, ::std::size_t ArgumentIndex>
  using binding = _template_selector_<typename _template_selector_tmpl_t<Tag, ArgumentIndex>::template type<type>>;
};

} /* namespace java::<unnamed> */

template<typename T>
using _template_selector = _template_selector_<::std::remove_const_t<typename maybe_unpack_type_<T>::type>>;

} /* namespace java */

#endif /* JAVA__TEMPLATE_SELECTOR_H */
