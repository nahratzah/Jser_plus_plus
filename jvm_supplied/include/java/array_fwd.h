#ifndef JAVA_ARRAY_FWD_H
#define JAVA_ARRAY_FWD_H

#include <java/ref.h>

namespace java::_erased::java {

template<typename T> class array;
template<typename T> class array_of_array;

} /* namespace java::_erased::java */

namespace java {
namespace {

template<typename Type, std::size_t Dimensions>
struct add_dimensions_ {
  using type = typename add_dimensions_<Type, Dimensions - 1>::type*;
};

template<typename Type>
struct add_dimensions_<Type, 0> {
  using type = Type;
};

} /* namespace java::<unnamed> */

template<typename T, std::size_t Dimensions = 1>
using array_type = basic_ref<
    cycle_ptr::cycle_gptr,
    typename add_dimensions_<typename maybe_unpack_type_<T>::type, Dimensions>::type>;

} /* namespace java */

#endif /* JAVA_ARRAY_FWD_H */
