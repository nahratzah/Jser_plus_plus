package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.lang.reflect.Modifier;
import java.lang.reflect.TypeVariable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import java.util.List;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * An interface for a java type.
 *
 * Note: despite the name (JavaType), these are {@link Type types}. This is
 * because a {@link Type} is emittable in the source code as a whole type,
 * whereas {@link JavaType} models a type declaration; {@link Type} are
 * instantiable.
 *
 * @author ariane
 */
public interface JavaType {
    /**
     * The full java class name for this type.
     *
     * @return String of the form "package.path.JavaClassName".
     */
    public String getName();

    /**
     * The generics arguments for this type.
     *
     * @return List of template arguments for this type.
     */
    public List<ClassTemplateArgument> getTemplateArguments();

    /**
     * The list of template argument names.
     *
     * @return List of template argument names.
     */
    public default List<String> getTemplateArgumentNames() {
        return getTemplateArguments().stream()
                .map(ClassTemplateArgument::getName)
                .collect(Collectors.toList());
    }

    /**
     * The number of generics arguments for this type.
     *
     * This member can be called before initialization completes.
     *
     * @return Number of generics arguments for this type.
     */
    public int getNumTemplateArguments();

    /**
     * Retrieve the name space (java package) of the type.
     *
     * @return List of path elements that make up the name space path.
     */
    public default List<String> getNamespace() {
        final List<String> parts = Arrays.asList(getName().split(Pattern.quote(".")));
        return parts.subList(0, parts.size() - 1);
    }

    /**
     * Retrieve the name of the class, excluding the name space.
     *
     * @return The simple name of the class.
     */
    public default String getClassName() {
        final List<String> parts = Arrays.asList(getName().split(Pattern.quote(".")));
        return parts.get(parts.size() - 1);
    }

    /**
     * Retrieve the super class of this type.
     *
     * May be null.
     *
     * @return The super class of this type, or null if this type has not super
     * class.
     */
    public BoundTemplate getSuperClass();

    /**
     * Retrieve the interfaces implemented by this type.
     *
     * @return List of interfaces implemented by this type.
     */
    public Collection<BoundTemplate> getInterfaces();

    /**
     * Retrieve additional includes required to implement this type.
     *
     * @param publicOnly If set, only return includes required to satisfy
     * declarations, not implementations.
     * @return Collection of includes.
     */
    public Collection<String> getIncludes(boolean publicOnly);

    /**
     * Retrieve the fields of this type.
     *
     * @return The fields of this type.
     */
    public default Collection<FieldType> getFields() {
        return EMPTY_LIST;
    }

    /**
     * Initialization of this type.
     *
     * We use late initialization, so that initializers can recurse type
     * lookups.
     *
     * @param ctx Context for lookups of required types.
     * @param cfg Configuration.
     */
    public default void init(Context ctx, Config cfg) {
    }

    /**
     * Helper that figures out all type parameters for a type.
     *
     * @param c Class for which to figure out all type parameters.
     * @return List of generics arguments. If the class is a non-static member
     * class, the generics arguments of the enclosing class will be in the front
     * of the list.
     */
    public static List<TypeVariable<? extends Class<?>>> getAllTypeParameters(Class<?> c) {
        final List<TypeVariable<? extends Class<?>>> result;
        if (c.isMemberClass() && !Modifier.isStatic(c.getModifiers()))
            result = getAllTypeParameters(c.getEnclosingClass());
        else
            result = new ArrayList<>();

        result.addAll(Arrays.asList(c.getTypeParameters()));
        return result;
    }

    /**
     * Test if this is an interface.
     *
     * @return True if this type is an interface.
     */
    public boolean isInterface();

    /**
     * Test if this is an abstract class, but not an interface.
     *
     * @return True if this type is an abstract class.
     */
    public boolean isAbstract();

    /**
     * Test if this class is final.
     *
     * @return True if this type is final.
     */
    public boolean isFinal();

    /**
     * Test if this is an enum class.
     *
     * @return True if this type is an enum.
     */
    public default boolean isEnum() {
        return false;
    }
}
