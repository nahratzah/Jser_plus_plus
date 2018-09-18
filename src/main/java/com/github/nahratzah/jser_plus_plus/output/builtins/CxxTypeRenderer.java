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
        return (emitConst ? "const " : "")
                + new ST(StCtx.BUILTINS, cxxType.getTemplate()).render(Locale.ROOT);
    }
}
