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
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * An enum type.
 *
 * @author ariane
 */
public class EnumType extends ClassType {
    private static final List<String> EXTRA_PUBLIC_INCLUDES = unmodifiableList(Arrays.asList("string_view"));

    public EnumType(Class<?> c) {
        super(c);
        if (!c.isEnum())
            throw new IllegalArgumentException("EnumType requires an enum class");
    }

    @Override
    public void init(Context ctx, Config cfg) {
        super.init(ctx, cfg); //To change body of generated methods, choose Tools | Templates.
    }

    @Override
    public Collection<String> getIncludes(boolean publicOnly) {
        return Stream.concat(super.getIncludes(publicOnly).stream(), EXTRA_PUBLIC_INCLUDES.stream())
                .collect(Collectors.toList());
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
        return name.toLowerCase(Locale.ROOT);
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
