package com.github.nahratzah.jser_plus_plus.model;

import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singleton;
import static java.util.Collections.singletonList;
import java.util.List;
import java.util.Objects;
import static java.util.Objects.requireNonNull;

/**
 * A java primitive type.
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

    /**
     * Find the primitive type for the given class.
     *
     * @param c The class for which to find the corresponding primitive type.
     * @return An instance of PrimitiveType, corresponding to the argument
     * class.
     * @throws IllegalStateException if the class is a primitive, but no
     * corresponding specialization of PrimitiveType exists.
     * @throws IllegalArgumentException if the class is not a primitive.
     */
    public static PrimitiveType fromClass(Class<?> c) {
        return Arrays.stream(values())
                .filter(v -> Objects.equals(v.c.getName(), c.getName()))
                .findAny()
                .orElseThrow(() -> {
                    if (c.isPrimitive())
                        return new IllegalStateException("Missing implementation for primitive type " + c);
                    else
                        return new IllegalArgumentException(c + " is not a primitive type");
                });
    }

    @Override
    public String getName() {
        return name;
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

    @Override
    public boolean isInterface() {
        return false;
    }

    @Override
    public boolean isAbstract() {
        return false;
    }

    private final String name;
    private final Class<?> c;
}
