package com.github.nahratzah.jser_plus_plus.model;

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

    private Type type;
}
