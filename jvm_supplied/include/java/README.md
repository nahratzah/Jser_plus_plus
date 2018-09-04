# Java Types

A java type internally consists of a family of types.

1. The type erased object.
2. The reference to the object, with generics parameters.
3. The field reference to the object (due to how cycle\_ptr keeps track of object member references).

In order to bind these all together with minimal namespace polution,
we use tagging.

## Tagging

Each java type has a tag in namespace `java::_tags`.

Example: given a java type `java.lang.Class<T>`,
a tag will be defined:

    namespace java::_tags::java::lang {
    struct Class;
    }

## Type Erased Object

In order to implement the type, we need to erase its generics parameters.
The resulting type is similar to how java compiles classes.

Example: given a java type `java.lang.Class<T>`,
an type-erased instance will be defined:

    namespace java::_erased::java::lang {

    class Class
    // Inheritance omitted for clarity.
    {
    public:
      Class() = default; // All types must be default constructible.

      // Example method: construct a new instance of T, for Class<T>.
      auto newInstance() -> java::lang::Object;

      // Class body omitted for clarity.
    };

    }

## Reference Types

In order to build a reference, `java.lang.Class<java.lang.Object>`,
we need to re-expose its methods with the correct generics-parameter specializations.
In the Java compiler, this is done automatically, by performing casts where needed.
In C++, we use templates to mimic this behaviour.

    namespace java::lang {

    template<typename T>
    using Class = java::var_ref<java::_type<java::_tags::java::lang::Class, T>>;

    }

The `java::var_ref` template is a PIMPL implementation that wraps
the type-erased `java::_erased::java::lang::Class`,
using accessor specializations based on the type.

    namespace java {

    template<template<class> class PtrImpl, typename... Types>
    class basic_ref
    : public java::_accessor<Types>...
    {
      static_assert(sizeof...(Types) >= 1);
    };

    // Reference type for variables, except member-variables.
    template<typename... Types>
    using var_ref = basic_ref<cycle_ptr::cycle_gptr, Types...>;

    // Reference type for member-variables.
    template<typename... Types>
    using field_ref = basic_ref<cycle_ptr::cycle_member_ptr, Types...>;

    }

An accessor is declared for `java.lang.Class`:

    namespace java {

    template<typename Derived, typename Tag, typename... TemplateArgs>
    class _accessor;

    template<typename Derived, typename T>
    class _accessor<Derived, java::_tags::java::lang::Class, T>
    : public _accessor<Derived, java::_tags::java::lang::Object>
      // Interface inheritance omitted for clarity.
    {
      // Method implementation, where generics parameters are specialized
      // appropriately.
      auto newInstance() const -> java::G::generics_to_ref<T> {
        // Acquire reference to implementation.
        // Throws if the PIMPL holds a nullptr.
        auto& instance = static_cast<Derived&>(*this)
            .template ref<java::_erased::java::lang::Class>();

        // Invoke method on type-erased version.
        java::lang::Object result = instance.newInstance();

        // Convert returned type to correct reference based on
        // actual generics parameters.
        return java::cast_<T>(result);
      }
    };

    }

## Fields

The `var_ref` template is a specialization of `basic_ref`,
taking a non-member-pointer as its template type.

The fields use the `field_ref` specialization,
which can be trivially deduced by replacing the first argument in `basic_ref`.

    namespace java {

    template<typename> struct field_for_; // Incomplete type.

    template<template<class> class Ptr, typename... Types>
    field_for_<java::basic_ref<Ptr, Types...>> {
      using type = field_ref<Types...>;
    };

    template<typename T>
    using field_for = typename field_for_<T>::type;

    }
