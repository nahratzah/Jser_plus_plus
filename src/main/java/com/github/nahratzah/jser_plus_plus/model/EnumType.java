package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.unmodifiableList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * An enum type.
 *
 * @author ariane
 */
public class EnumType extends ClassType {
    private static final List<String> EXTRA_PUBLIC_INCLUDES = unmodifiableList(Arrays.asList("string_view", "java/primitives.h"));

    public EnumType(Class<?> c) {
        super(c);
        if (!c.isEnum())
            throw new IllegalArgumentException("EnumType requires an enum class");
    }

    @Override
    public void init(Context ctx, Config cfg) {
        super.init(ctx, cfg);
    }

    @Override
    public Stream<String> getImplementationIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        if (!recursionGuard.add(this)) return Stream.empty();
        return Stream.concat(
                super.getImplementationIncludes(publicOnly, recursionGuard),
                EXTRA_PUBLIC_INCLUDES.stream());
    }

    @Override
    public boolean isAbstract() {
        // Enums are never abstract, but the way enums are compiled,
        // they'll appear to be from a java.lang.Class perspective.
        // Override explicitly to remedy this trait.
        return false;
    }

    @Override
    public boolean isEnum() {
        return true;
    }

    /**
     * Retrieve all the enum values that this enum contains.
     *
     * @return Map containing as key the original name of the enum constant,
     * mapped to the remapped name.
     */
    public Collection<EnumConstant> getEnumValues() {
        return Arrays.stream(c.getEnumConstants())
                .map(Enum.class::cast)
                .map(e -> e.name())
                .map(EnumConstant::new)
                .collect(Collectors.toList());
    }

    /**
     * Mangle enum constant names, such that they won't clash with defines.
     *
     * @param name The name of the enum constant.
     * @return Mangled name of the enum constant.
     */
    private static String remapEnumConstant(String name) {
        return name.substring(0, 1).toUpperCase(Locale.ROOT) + name.substring(1).toLowerCase(Locale.ROOT);
    }

    /**
     * Track enum names and their remapped name.
     */
    public static final class EnumConstant {
        public EnumConstant(String name) {
            this.name = requireNonNull(name);
        }

        public String getName() {
            return name;
        }

        public String getRemappedName() {
            return remapEnumConstant(name);
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 37 * hash + Objects.hashCode(this.name);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final EnumConstant other = (EnumConstant) obj;
            if (!Objects.equals(this.name, other.name)) return false;
            return true;
        }

        private final String name;
    }
}
