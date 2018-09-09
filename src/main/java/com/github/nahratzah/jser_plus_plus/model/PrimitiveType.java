package com.github.nahratzah.jser_plus_plus.model;

import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singleton;
import static java.util.Collections.singletonList;
import java.util.List;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;

/**
 *
 * @author ariane
 */
public enum PrimitiveType implements JavaType {
    BOOLEAN("boolean", Boolean.TYPE),
    BYTE("byte", Byte.TYPE),
    SHORT("short", Short.TYPE),
    INT("int", Integer.TYPE),
    LONG("long", Long.TYPE),
    FLOAT("float", Float.TYPE),
    DOUBLE("double", Double.TYPE),
    CHAR("char", Character.TYPE),
    VOID("void", Void.TYPE);

    private PrimitiveType(String name, Class<?> c) {
        this.name = requireNonNull(name);
        this.c = requireNonNull(c);
    }

    public static Optional<PrimitiveType> fromClass(Class<?> c) {
        return Arrays.stream(values())
                .filter(v -> Objects.equals(v.c.getName(), c.getName()))
                .findAny();
    }

    @Override
    public String getName() {
        return String.join(".", getNamespace()) + "." + name;
    }

    @Override
    public List<String> getNamespace() {
        return singletonList("java");
    }

    @Override
    public String getSimpleName() {
        return name + "_t";
    }

    @Override
    public List<ClassTemplateArgument> getTemplateArguments() {
        return EMPTY_LIST;
    }

    @Override
    public int getNumTemplateArguments() {
        return 0;
    }

    @Override
    public BoundTemplate getSuperClass() {
        return null;
    }

    @Override
    public Collection<BoundTemplate> getInterfaces() {
        return EMPTY_LIST;
    }

    @Override
    public Collection<String> getIncludes(boolean publicOnly) {
        return singleton("java/primitives.h");
    }

    private final String name;
    private final Class<?> c;
}
