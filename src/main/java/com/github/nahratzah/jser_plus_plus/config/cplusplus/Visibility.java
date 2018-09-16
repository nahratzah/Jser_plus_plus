package com.github.nahratzah.jser_plus_plus.config.cplusplus;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import java.util.Locale;

/**
 * C++ visibility.
 *
 * @author ariane
 */
public enum Visibility {
    /**
     * Public visibility.
     */
    PUBLIC,
    /**
     * Protected visibility.
     */
    PROTECTED,
    /**
     * Private visibility.
     */
    PRIVATE;

    /**
     * The text representation of the visibility.
     *
     * @return One of "public", "protected", or "private".
     */
    @Override
    public String toString() {
        return name().toLowerCase(Locale.ROOT);
    }

    @JsonCreator
    private static Visibility fromJson(String s) {
        return Visibility.valueOf(s.toUpperCase(Locale.ROOT));
    }

    @JsonValue
    private String toJson() {
        return name().toLowerCase(Locale.ROOT);
    }
}
