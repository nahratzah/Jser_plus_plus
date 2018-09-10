package com.github.nahratzah.jser_plus_plus.output.builtins;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import static java.nio.charset.StandardCharsets.UTF_8;
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

    private static final String[] rules = new String[]{
        "cxxString(s) ::= \"$cxxString_.(s)$\"",
        "u8String(s) ::= \"u8$cxxString(s)$s\"",
        "u16String(s) ::= \"u16$cxxString(s)$s\"",
        "u8StringView(s) ::= \"u8$cxxString(s)$sv\"",
        "u16StringView(s) ::= \"u16$cxxString(s)$sv\""
    };
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
    }
}
