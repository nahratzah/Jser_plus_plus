package com.github.nahratzah.jser_plus_plus.config.cplusplus;

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
}
