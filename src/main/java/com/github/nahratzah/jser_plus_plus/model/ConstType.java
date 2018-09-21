package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Collection;
import java.util.Map;
import java.util.Set;
import java.util.stream.Stream;

/**
 * Wraps a type so that it'll be const.
 *
 * @author ariane
 */
public class ConstType implements Type {
    public ConstType(Type type) {
        if (type instanceof ConstType)
            this.type = ((ConstType) type).type;
        else
            this.type = type;
    }

    @Override
    public ConstType prerender(Context ctx, Map<String, ?> renderArgs, Collection<String> variables) {
        return new ConstType(type.prerender(ctx, renderArgs, variables));
    }

    @Override
    public Set<String> getUnresolvedTemplateNames() {
        return type.getUnresolvedTemplateNames();
    }

    public Type getType() {
        return type;
    }

    public void setType(Type type) {
        if (type instanceof ConstType)
            this.type = ((ConstType) type).type;
        else
            this.type = type;
    }

    @Override
    public Stream<String> getIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        return type.getIncludes(publicOnly, recursionGuard);
    }

    @Override
    public Stream<JavaType> getAllJavaTypes() {
        return type.getAllJavaTypes();
    }

    private Type type;
}
