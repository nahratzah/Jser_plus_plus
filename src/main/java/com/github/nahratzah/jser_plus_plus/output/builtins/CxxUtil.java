package com.github.nahratzah.jser_plus_plus.output.builtins;

import java.util.Locale;
import java.util.stream.Collectors;

/**
 *
 * @author ariane
 */
public class CxxUtil {
    private CxxUtil() {
    }

    /**
     * Create a string literal for C++.
     *
     * @param s The string for which to generate a literal.
     * @return The literal, with proper escapes and surrounded by double quotes.
     */
    public static String plainStringLiteral(String s) {
        return s.codePoints()
                .mapToObj(cp -> escCodePoint(cp))
                .collect(Collectors.joining("", "\"", "\""));
    }

    private static StringBuilder escCodePoint(int cp) {
        if (cp == 0) // null character
            return new StringBuilder("\\0");
        if (cp == 0x5c) // backslash
            return new StringBuilder("\\\\");
        if (cp == 0x22) // double quote
            return new StringBuilder("\\\"");
        if (cp == 0x0a) // new line
            return new StringBuilder("\\n");

        if (cp >= 0x20 && cp < 0x7f) { // Printable ascii.
            final StringBuilder result = new StringBuilder().append(Character.toChars(cp));
            if (result.toString().equals("\\")) result.append("\\");
            return result;
        } else { // Hexadecimal escape code.
            final StringBuilder result = new StringBuilder(Integer.toUnsignedString(cp, 16).toLowerCase(Locale.ROOT));
            final String prefix;
            final int len;

            if (result.length() <= 2 && cp < 0x20) { // Use 8 bit escape for low character.
                prefix = "\\x";
                len = 2;
            } else if (result.length() <= 4) { // Use 16 bit escape when possible.
                prefix = "\\u";
                len = 4;
            } else { // Use 32 bit escape if needed.
                prefix = "\\U";
                len = 8;
            }

            while (result.length() < len)
                result.insert(0, '0');
            result.insert(0, prefix);
            return result;
        }
    }
}
