package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.regex.Pattern;

/**
 * An interface for a java type.
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
    public default String getSimpleName() {
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
}
