package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * All types derive from type.
 *
 * @author ariane
 */
public interface Type {
    /**
     * Rebind unbound template argument names.
     *
     * @param bindings Mapping of name to bound type.
     * @return A copy of this bound template, where each occurance of unresolved
     * template variabe in bindings has been replaced with its mapping.
     */
    public Type rebind(Map<String, ? extends BoundTemplate> bindings);

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
     * Test if the type is a java type.
     *
     * @return True if the type is a java type, false otherwise.
     */
    public boolean isJavaType();

    /**
     * Create a {@link Type} from a {@link CfgType configuration type}.
     *
     * @param cfgType A type in the configuration.
     * @param ctx Class lookup context.
     * @param renderArgs Arguments passed to the renderer for
     * {@link CxxType C++ types}.
     * @param variables Type variables that are declared in the context.
     * @param thisType Type when encountering the `this` keyword.
     * @return Instance of {@link CxxType} or {@link BoundTemplate}
     * corresponding to the configuration type.
     */
    public static Type typeFromCfgType(CfgType cfgType, Context ctx, Map<String, ?> renderArgs, Collection<String> variables, BoundTemplate.ClassBinding<?> thisType) {
        final Map<String, BoundTemplate.VarBinding> variablesMap = variables.stream()
                .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));
        return typeFromCfgType(cfgType, ctx, renderArgs, variablesMap, thisType);
    }

    /**
     * Create a {@link Type} from a {@link CfgType configuration type}.
     *
     * @param cfgType A type in the configuration.
     * @param ctx Class lookup context.
     * @param renderArgs Arguments passed to the renderer for
     * {@link CxxType C++ types}.
     * @param variablesMap Type variables that are declared in the context.
     * @param thisType Type when encountering the `this` keyword.
     * @return Instance of {@link CxxType} or {@link BoundTemplate}
     * corresponding to the configuration type.
     */
    public static Type typeFromCfgType(CfgType cfgType, Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variablesMap, BoundTemplate.ClassBinding<?> thisType) {
        if (cfgType.getCxxType() != null && cfgType.getJavaType() != null)
            throw new IllegalStateException("may only specify one of \"cxx\" or \"java\" for return type");

        if (cfgType.getCxxType() != null) {
            return new CxxType(ctx, cfgType.getCxxType(), cfgType.getIncludes(), renderArgs, variablesMap, thisType);
        } else if (cfgType.getJavaType() != null) {
            return BoundTemplate.fromString(cfgType.getJavaType(), ctx, variablesMap, thisType);
        }

        // Neither cfgType.getCxxType(), nor cfgType.getJavaType() was specified.
        throw new NullPointerException("no types");
    }
}
