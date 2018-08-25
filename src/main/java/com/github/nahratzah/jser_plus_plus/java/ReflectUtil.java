package com.github.nahratzah.jser_plus_plus.java;

import java.lang.reflect.GenericArrayType;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import static java.util.Objects.requireNonNull;

/**
 * Utilities to make reflection usable.
 *
 * @author ariane
 */
public enum ReflectUtil {
    ;

    /**
     * Visit the implementation of a specific {@link java.lang.reflect.Type}.
     * @param <T> Return type of the visit operation.
     * @param t The type to visit.
     * @param visitor Visitor implementation.
     * @return The result of invoking {@code visitor.apply(t)} for the specialization of {@code t}.
     */
    public static <T> T visitType(Type t, Visitor<T> visitor) {
        requireNonNull(t);
        requireNonNull(visitor);

        if (t instanceof ParameterizedType)
            return visitor.apply((ParameterizedType) t);
        if (t instanceof GenericArrayType)
            return visitor.apply((GenericArrayType) t);
        if (t instanceof TypeVariable)
            return visitor.apply((TypeVariable) t);
        if (t instanceof WildcardType)
            return visitor.apply((WildcardType) t);
        if (t instanceof Class)
            return visitor.apply((Class) t);
        throw new IllegalStateException("Missing visitor for type " + t.getClass());
    }

    /**
     * Visitor for {@link java.lang.reflect.Type} specializations.
     *
     * @param <T> The return type of the visit operation.
     */
    public static interface Visitor<T> {
        /**
         * Apply the visitor to a specific {@link java.lang.Class}.
         *
         * @param var The type that is being visited.
         * @return Anything.
         */
        public T apply(Class<?> var);

        /**
         * Apply the visitor to a specific
         * {@link java.lang.reflect.TypeVariable}.
         *
         * @param var The type that is being visited.
         * @return Anything.
         */
        public T apply(TypeVariable<?> var);

        /**
         * Apply the visitor to a specific
         * {@link java.lang.reflect.GenericArrayType}.
         *
         * @param var The type that is being visited.
         * @return Anything.
         */
        public T apply(GenericArrayType var);

        /**
         * Apply the visitor to a specific
         * {@link java.lang.reflect.WildcardType}.
         *
         * @param var The type that is being visited.
         * @return Anything.
         */
        public T apply(WildcardType var);

        /**
         * Apply the visitor to a specific
         * {@link java.lang.reflect.ParameterizedType}.
         *
         * @param var The type that is being visited.
         * @return Anything.
         */
        public T apply(ParameterizedType var);
    }
}
