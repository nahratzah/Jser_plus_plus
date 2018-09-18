package com.github.nahratzah.jser_plus_plus.output.builtins;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import static java.nio.charset.StandardCharsets.UTF_8;
import java.util.Arrays;
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

        BUILTINS.defineDictionary("cxxString_", new FunctionAttrMap(CxxUtil::plainStringLiteral));
        BUILTINS.defineDictionary("renderDocString_", new FunctionAttrMap(StCtx::renderDocString));
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
}
