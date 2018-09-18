package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import static java.util.Collections.unmodifiableMap;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * Renders the {@link ConstType}.
 *
 * @author ariane
 */
public class ConstTypeRenderer implements TypeAttributeRenderer {
    public static final Class<ConstType> ATTRIBUTE_CLASS = ConstType.class;

    @Override
    public String toString(Type type, String formatString, Locale locale, boolean emitConst) {
        final Type innerType = ((ConstType) type).getType();
        return RENDERERS.entrySet().stream()
                .filter(entry -> entry.getKey().isInstance(innerType))
                .map(Map.Entry::getValue)
                .findAny()
                .orElseThrow(() -> new IllegalStateException("Missing renderer for " + innerType.getClass() + ": " + innerType))
                .toString(innerType, formatString, locale, true);
    }

    private static final Map<Class<?>, TypeAttributeRenderer> RENDERERS;

    static {
        final Map<Class<?>, TypeAttributeRenderer> renderers = new HashMap<>();
        renderers.put(ConstTypeRenderer.ATTRIBUTE_CLASS, new ConstTypeRenderer());
        renderers.put(CxxTypeRenderer.ATTRIBUTE_CLASS, new CxxTypeRenderer());
        renderers.put(BoundTemplateRenderer.ATTRIBUTE_CLASS, new BoundTemplateRenderer());
        RENDERERS = unmodifiableMap(renderers);
    }
}
