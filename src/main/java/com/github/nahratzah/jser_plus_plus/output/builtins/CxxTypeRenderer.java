package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.CxxType;
import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Pattern;
import org.stringtemplate.v4.AttributeRenderer;
import org.stringtemplate.v4.ST;

/**
 * Render a {@link CxxType C++ type}.
 *
 * @author ariane
 */
public class CxxTypeRenderer implements AttributeRenderer {
    public static final Class<CxxType> ATTRIBUTE_CLASS = CxxType.class;

    @Override
    public String toString(Object o, String formatString, Locale locale) {
        final CxxType cxxType = (CxxType) o;
        final boolean emitConst = Arrays.stream(formatString.split(Pattern.quote(",")))
                .map(String::trim)
                .anyMatch("const"::equals);
        return (emitConst ? "const " : "")
                + new ST(StCtx.BUILTINS, cxxType.getTemplate()).render(Locale.ROOT);
    }
}
