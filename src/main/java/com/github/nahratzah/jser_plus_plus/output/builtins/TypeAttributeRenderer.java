package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.Type;
import java.util.Locale;
import org.stringtemplate.v4.AttributeRenderer;

/**
 * Attribute renderer for types.
 *
 * @author ariane
 */
public interface TypeAttributeRenderer extends AttributeRenderer {
    @Override
    public default String toString(Object o, String formatString, Locale locale) {
        return toString((Type) o, formatString, locale, false);
    }

    public String toString(Type type, String formatString, Locale locale, boolean emitConst);
}
