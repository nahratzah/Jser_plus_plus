package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import static java.nio.charset.StandardCharsets.UTF_8;
import java.util.Arrays;
import java.util.Map;
import java.util.function.Consumer;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import org.stringtemplate.v4.STGroup;
import org.stringtemplate.v4.STGroupString;

/**
 * String Template context.
 *
 * @author ariane
 */
public class StCtx {
    private StCtx() {
    }

    public static final STGroup BUILTINS;

    /**
     * Create a group that contains the 'java' dictionary.
     *
     * The java dictionary performs type lookups.
     *
     * @param context The resolver context to use to find classes.
     * @param variables A list of type variables that is known in this context.
     * @param thisType The type for the `this` keyword.
     * @param registry A registry that listens for all types resolved using this
     * context.
     * @return A STGroup that imports {@link #BUILTINS} and contains the 'java'
     * dictionary.
     */
    public static final STGroup contextGroup(Context context, Map<String, ? extends BoundTemplate> variables, BoundTemplate.ClassBinding<?> thisType, Consumer<? super BoundTemplate> registry) {
        final STGroup contextGroup = new STGroupString("<context>", "", '$', '$');
        contextGroup.defineDictionary(
                "java",
                new FunctionAttrMap(text -> {
                    final Type type = BoundTemplate.fromString(text, context, variables, thisType);

                    // XXX figure out a prettier way to do this.
                    final BoundTemplate template;
                    if (type instanceof ConstType)
                        template = (BoundTemplate) ((ConstType) type).getType();
                    else
                        template = (BoundTemplate) type;

                    registry.accept(template);
                    return type;
                }));
        contextGroup.importTemplates(BUILTINS);
        return contextGroup;
    }

    static {
        try (Reader accessorFile = new InputStreamReader(StCtx.class.getResourceAsStream("rules.stg"), UTF_8)) {
            final char[] cbuf = new char[1024];
            StringBuilder sb = new StringBuilder();
            for (int rlen = accessorFile.read(cbuf); rlen != -1;
                 rlen = accessorFile.read(cbuf))
                sb.append(cbuf, 0, rlen);
            BUILTINS = new STGroupString("rules.stg", sb.toString(), '$', '$');
        } catch (IOException ex) {
            throw new IllegalStateException("unable to load accessor template", ex);
        }

        BUILTINS.defineDictionary("cxxString_", new FunctionAttrMap(StCtx::plainStringLiteral));
        BUILTINS.defineDictionary("renderDocString_", new FunctionAttrMap(StCtx::renderDocString));
        BUILTINS.defineDictionary("argNameIsMaybeUnused_", new FunctionAttrMap(StCtx::argNameIsMaybeUnused));
        BUILTINS.registerRenderer(BoundTemplateRenderer.ATTRIBUTE_CLASS, new BoundTemplateRenderer());
        BUILTINS.registerRenderer(CxxTypeRenderer.ATTRIBUTE_CLASS, new CxxTypeRenderer());
        BUILTINS.registerRenderer(ConstTypeRenderer.ATTRIBUTE_CLASS, new ConstTypeRenderer());
    }

    private static String renderDocString(String docString) {
        // Remove trailing new lines.
        while (docString.endsWith("\n"))
            docString = docString.substring(0, docString.length() - 1);

        final String[] lines = docString.split(Pattern.quote("\n"));
        if (lines.length == 0) return null;
        if (lines.length == 1) return "///" + lines[0];

        return Arrays.stream(lines)
                .map(line -> line.isEmpty() ? " *" : " * " + line)
                .collect(Collectors.joining("\n", "/**\n", "\n */"));
    }

    private static Boolean argNameIsMaybeUnused(String name) {
        return name == null || (name.startsWith("_") && name.endsWith("_"));
    }

    private static String plainStringLiteral(String s) {
        if (s.length() < 4 || !s.startsWith("__") || !s.endsWith("__"))
            throw new IllegalArgumentException("We're expecting two underscores at the begin and end, to evade member methods of the property map.");

        return CxxUtil.plainStringLiteral(s.substring(2, s.length() - 2));
    }
}
