package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Stream;

/**
 * All types derive from type.
 *
 * @author ariane
 */
public interface Type {
    /**
     * Pre-render a type, allowing for auto detection of java types.
     *
     * @param ctx Lookup context for finding java types.
     * @param renderArgs Map of items to supply as arguments to the renderer.
     * @param variables List of type variables that are in scope.
     * @return
     */
    public Type prerender(Context ctx, Map<String, ?> renderArgs, Collection<String> variables);

    /**
     * Retrieve the list of parameter names that are not bound.
     *
     * @return List of parameter names that are not bound.
     */
    public Set<String> getUnresolvedTemplateNames();

    /**
     * Retrieve additional includes required to implement this type.
     *
     * @param publicOnly If set, only return includes required to satisfy
     * declarations, not implementations.
     * @return Collection of includes.
     */
    public default Stream<String> getIncludes(boolean publicOnly) {
        return getIncludes(publicOnly, new HashSet<>());
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
    public Stream<String> getIncludes(boolean publicOnly, Set<JavaType> recursionGuard);

    /**
     * Retrieve all {@link JavaType java types} that make up this type.
     *
     * @return All java types that make up this type.
     */
    public Stream<JavaType> getAllJavaTypes();

    /**
     * Create a {@link Type} from a {@link CfgType configuration type}.
     *
     * @param cfgType A type in the configuration.
     * @param ctx Class lookup context.
     * @param variables Type variables that are declared in the context.
     * @return Instance of {@link CxxType} or {@link BoundTemplate}
     * corresponding to the configuration type.
     */
    public static Type typeFromCfgType(CfgType cfgType, Context ctx, Collection<String> variables) {
        if (cfgType.getCxxType() == null && cfgType.getJavaType() == null)
            throw new NullPointerException("no types");

        if (cfgType.getCxxType() != null)
            return new CxxType(cfgType.getCxxType(), cfgType.getIncludes());
        else if (cfgType.getJavaType() != null)
            return BoundTemplate.fromString(cfgType.getJavaType(), ctx, variables);

        throw new IllegalStateException("may only specify one of \"cxx\" or \"java\" for return type");
    }
}
