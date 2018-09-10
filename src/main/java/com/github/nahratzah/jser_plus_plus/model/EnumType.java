package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Arrays;
import java.util.Locale;
import java.util.Map;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * An enum type.
 *
 * @author ariane
 */
public class EnumType extends ClassType {
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
    public boolean isEnum() {
        return true;
    }

    /**
     * Retrieve all the enum values that this enum contains.
     *
     * @return Map containing as key the original name of the enum constant,
     * mapped to the remapped name.
     */
    public Map<String, String> getEnumValues() {
        return Arrays.stream(c.getEnumConstants())
                .map(Enum.class::cast)
                .map(e -> e.name())
                .collect(Collectors.toMap(Function.identity(), EnumType::remapEnumConstant));
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
}
