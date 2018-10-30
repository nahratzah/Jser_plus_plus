package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import java.util.Locale;

/**
 * Render a {@link CxxType C++ type}.
 *
 * @author ariane
 */
public class CxxTypeRenderer implements TypeAttributeRenderer {
    public static final Class<CxxType> ATTRIBUTE_CLASS = CxxType.class;

    @Override
    public String toString(Type type, String formatString, Locale locale, boolean emitConst) {
        final CxxType cxxType = (CxxType) type;

        final StringBuilder result = new StringBuilder();
        if (emitConst) result.append("const ");

        assert cxxType.getPreRendered() != null;
        result.append(cxxType.getPreRendered());

        return result.toString();
    }
}
