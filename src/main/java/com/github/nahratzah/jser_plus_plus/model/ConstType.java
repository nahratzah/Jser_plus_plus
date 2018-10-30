package com.github.nahratzah.jser_plus_plus.model;

import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Stream;

/**
 * Wraps a type so that it'll be const.
 *
 * @author ariane
 */
public final class ConstType implements Type {
    public ConstType(Type type) {
        if (type instanceof ConstType)
            this.type = ((ConstType) type).type;
        else
            this.type = type;
    }

    @Override
    public ConstType rebind(Map<String, ? extends BoundTemplate> bindings) {
        return new ConstType(type.rebind(bindings));
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

    @Override
    public final boolean isJavaType() {
        return type.isJavaType();
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 23 * hash + Objects.hashCode(this.type);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final ConstType other = (ConstType) obj;
        if (!Objects.equals(this.type, other.type)) return false;
        return true;
    }

    @Override
    public String toString() {
        return "const " + type;
    }

    private Type type;
}
