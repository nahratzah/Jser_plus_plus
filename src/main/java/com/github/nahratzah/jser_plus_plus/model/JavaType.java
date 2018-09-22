package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.lang.reflect.Modifier;
import java.lang.reflect.TypeVariable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

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
    public BoundTemplate.ClassBinding getSuperClass();

    /**
     * Retrieve the interfaces implemented by this type.
     *
     * @return List of interfaces implemented by this type.
     */
    public Collection<BoundTemplate.ClassBinding> getInterfaces();

    /**
     * Retrieve includes requires to use this type.
     *
     * @return List of includes to use this type.
     */
    public default Stream<String> getIncludes() {
        return Stream.empty();
    }

    /**
     * Retrieve additional includes required to implement this type.
     *
     * @param publicOnly If set, only return includes required to satisfy
     * declarations, not implementations.
     * @return Collection of includes.
     */
    public default Stream<String> getImplementationIncludes(boolean publicOnly) {
        return getImplementationIncludes(publicOnly, new HashSet<>());
    }

    /**
     * Retrieve additional includes required to implement this type.
     *
     * @param publicOnly If set, only return includes required to satisfy
     * declarations, not implementations.
     * @param recursionGuard A set that each type adds itself to, to detect
     * recursion. If the type is already in the set, it shall emit an empty
     * collection.
     * @return Collection of includes.
     */
    public Stream<String> getImplementationIncludes(boolean publicOnly, Set<JavaType> recursionGuard);

    /**
     * Get all java types required to forward declare this type.
     *
     * @return All java types required to forward declare this type.
     */
    public Stream<JavaType> getForwardDeclarationJavaTypes();

    /**
     * Get all java types required to declare this type. Only types that must be
     * complete at the declaration type are returned.
     *
     * @return All java types required to declare this type.
     */
    public Stream<JavaType> getDeclarationCompleteJavaTypes();

    /**
     * Get all java types required to declare this type, for which a forward
     * declaration is sufficient.
     *
     * @return All java types required to declare this type.
     */
    public Stream<JavaType> getDeclarationForwardJavaTypes();

    /**
     * Get all java types required to implement functionality of this type.
     *
     * This means also looking inside functions and listing their internal
     * types.
     *
     * @return All java types required to implement this type.
     */
    public Stream<JavaType> getImplementationJavaTypes();

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
     * Post processing of this type.
     *
     * Post processing stage consists of figuring out anything depending on
     * parent types.
     *
     * @param ctx Lookup context.
     */
    public default void postProcess(Context ctx) {
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
     * Retrieve the default variable type for this class.
     *
     * @return The type to use for fields and variables. Returns null if the
     * class type is to be used directly.
     */
    public default Type getVarType() {
        return null;
    }

    /**
     * Test if this is an enum class.
     *
     * @return True if this type is an enum.
     */
    public default boolean isEnum() {
        return false;
    }
}
