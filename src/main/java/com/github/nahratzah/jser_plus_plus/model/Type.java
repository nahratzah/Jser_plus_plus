package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Collection;

/**
 * All types derive from type.
 *
 * @author ariane
 */
public interface Type {
    /**
     * Retrieve additional includes required to implement this type.
     *
     * @param publicOnly If set, only return includes required to satisfy
     * declarations, not implementations.
     * @return Collection of includes.
     */
    public Collection<String> getIncludes(boolean publicOnly);

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
