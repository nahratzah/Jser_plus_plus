package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import java.util.Locale;
import org.stringtemplate.v4.ST;

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

        if (cxxType.getPreRendered() != null)
            result.append(cxxType.getPreRendered());
        else
            result.append(new ST(StCtx.BUILTINS, cxxType.getTemplate()).render(Locale.ROOT));

        return result.toString();
    }
}
