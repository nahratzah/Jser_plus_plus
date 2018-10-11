package com.github.nahratzah.jser_plus_plus.config.cplusplus;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import java.util.Locale;

/**
 *
 * @author ariane
 */
public enum DecodeStage {
    INITIAL,
    COMPARABLE,
    COMPLETE;

    public boolean isAtInitialStage() {
        return this == INITIAL;
    }

    public boolean isAtComparableStage() {
        return this == COMPARABLE;
    }

    public boolean isAtCompleteStage() {
        return this == COMPLETE;
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
