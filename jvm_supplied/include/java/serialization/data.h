#ifndef JAVA_SERIALIZATION_DATA_H
#define JAVA_SERIALIZATION_DATA_H

namespace java::serialization {

/**
 * \brief Implementation of \p Type without generics.
 */
template<typename Type>
class data;

/**
 * \brief Implementation of templated \p Type without generics.
 */
template<template<class...> class Type>
class tmpl_data;

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_DATA_H */
