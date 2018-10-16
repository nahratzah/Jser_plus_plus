package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.ArrayList;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 *
 * @author ariane
 */
class BoundTemplateParser {
    private static final String IDENTIFIER = "[_a-zA-Z][_a-zA-Z0-9$]*";
    private static final String ANY = Pattern.quote("?");
    private static final String EXTENDS_SPECIFIER = Pattern.quote("extends");
    private static final String SUPER_SPECIFIER = Pattern.quote("super");

    private static Pattern startWith(String pattern) {
        return Pattern.compile("^" + pattern);
    }

    private static final Pattern CLASS_NAME_STARTS = startWith(
            IDENTIFIER
            + "(?:\\s*"
            + Pattern.quote(".")
            + "\\s*"
            + IDENTIFIER
            + ")*");
    private static final Pattern ANY_STARTS = startWith(ANY);
    private static final Pattern EXTENDS_STARTS = startWith(EXTENDS_SPECIFIER + "\\s+");
    private static final Pattern SUPER_STARTS = startWith(SUPER_SPECIFIER + "\\s+");
    private static final Pattern WS = startWith("\\s*");
    private static final Pattern AMPERSANT_START = startWith("\\s*" + Pattern.quote("&") + "\\s*");
    private static final Pattern ARRAY_EXTENT = startWith("\\s*" + Pattern.quote("[") + "\\s*" + Pattern.quote("]"));
    private static final Pattern ROUND_BRACKET_OPEN = startWith("\\s*" + Pattern.quote("("));
    private static final Pattern CONST_STARTS = startWith("\\s*" + Pattern.quote("const") + "\\s+");
    private final Pattern VARIABLES_STARTS;

    /**
     * Context for class resolution.
     */
    private final Context ctx;
    /**
     * Mapping for variables.
     */
    final Map<String, ? extends BoundTemplate> variables;
    /**
     * Current type, for `this`.
     */
    final BoundTemplate.ClassBinding<?> thisType;
    /**
     * The string on which we are attempting to match.
     */
    private CharSequence s;

    public BoundTemplateParser(Context ctx, Map<String, ? extends BoundTemplate> variables, BoundTemplate.ClassBinding<?> thisType) {
        this.ctx = requireNonNull(ctx);
        this.variables = variables;
        this.thisType = thisType;
        if (variables.isEmpty())
            VARIABLES_STARTS = null;
        else
            VARIABLES_STARTS = startWith(variables.keySet().stream()
                    .map(Pattern::quote)
                    .collect(Collectors.joining("|", "(?:", ")")));
    }

    public Type parse(CharSequence text) {
        final boolean wrapConst;
        this.s = requireNonNull(text);

        // Eat the const keyword at the beginning.
        {
            final Matcher constMatcher = CONST_STARTS.matcher(s);
            wrapConst = constMatcher.find();
            if (wrapConst) s = s.subSequence(constMatcher.end(), s.length());
        }

        final BoundTemplate type = parse_();
        eatWs_();
        if (s.length() != 0)
            throw new IllegalArgumentException("Text remaining after parsing.");
        return (wrapConst
                ? new ConstType(type)
                : type);
    }

    private void eatWs_() {
        final Matcher ws = WS.matcher(s);
        if (ws.find()) s = s.subSequence(ws.end(), s.length());
    }

    private BoundTemplate parse0_() {
        eatWs_();

        final Matcher any = ANY_STARTS.matcher(s);
        final Matcher var = (VARIABLES_STARTS == null ? null : VARIABLES_STARTS.matcher(s));
        final Matcher cls = CLASS_NAME_STARTS.matcher(s);

        if (any.find()) {
            s = s.subSequence(any.end(), s.length());
            return parseAny_();
        } else if (var != null && var.find()) {
            final String varName = var.group();
            s = s.subSequence(var.end(), s.length());
            return Objects.requireNonNull(variables.get(varName));
        } else if (cls.find()) {
            final String clsGroup = cls.group().replaceAll("\\s", "");
            s = s.subSequence(cls.end(), s.length());

            if ("this".equals(clsGroup)) return thisType;

            final JavaType type = ctx.resolveClass(clsGroup);
            return new BoundTemplate.ClassBinding<>(type, maybeParseTemplate_());
        } else {
            throw new IllegalArgumentException("Expected variable, class, or wildcard.");
        }
    }

    private BoundTemplate parse_() {
        final BoundTemplate type;

        final Matcher bracketMatcher = ROUND_BRACKET_OPEN.matcher(s);
        if (bracketMatcher.find()) {
            type = parse_();

            eatWs_();
            if (s.length() == 0 || s.charAt(0) != ')')
                throw new IllegalArgumentException("Expected closing bracket.");
            s = s.subSequence(1, s.length());
        } else {
            type = parse0_();
        }

        for (int extents = 0; true; ++extents) {
            final Matcher arrayMatcher = ARRAY_EXTENT.matcher(s);
            if (!arrayMatcher.find()) {
                if (extents == 0) return type;
                return new BoundTemplate.ArrayBinding(type, extents);
            }
            s = s.subSequence(arrayMatcher.end(), s.length());
        }
    }

    private BoundTemplate parseAny_() {
        final List<BoundTemplate> superTypes = new ArrayList<>();
        final List<BoundTemplate> extendTypes = new ArrayList<>();

        for (;;) {
            eatWs_();

            final Matcher extendsMatcher = EXTENDS_STARTS.matcher(s);
            final Matcher superMatcher = SUPER_STARTS.matcher(s);
            if (extendsMatcher.find()) {
                s = s.subSequence(extendsMatcher.end(), s.length());
                extendTypes.addAll(parseMultipleTypes_());
            } else if (superMatcher.find()) {
                s = s.subSequence(superMatcher.end(), s.length());
                superTypes.addAll(parseMultipleTypes_());
            } else {
                return new BoundTemplate.Any(superTypes, extendTypes);
            }
        }
    }

    private Collection<BoundTemplate> parseMultipleTypes_() {
        final Collection<BoundTemplate> types = new ArrayList<>();

        eatWs_();

        for (;;) {
            final Matcher varName = (VARIABLES_STARTS == null ? null : VARIABLES_STARTS.matcher(s));
            final Matcher clsName = CLASS_NAME_STARTS.matcher(s);
            if (s.length() >= 1 && s.charAt(0) == '(') {
                types.add(parse_());
            } else if (varName != null && varName.find()) {
                types.add(Objects.requireNonNull(variables.get(varName.group())));
                s = s.subSequence(varName.end(), s.length());
            } else if (clsName.find()) {
                final JavaType cls = ctx.resolveClass(clsName.group().replaceAll("\\s", ""));
                types.add(new BoundTemplate.ClassBinding<>(cls, maybeParseTemplate_()));
            } else {
                throw new IllegalArgumentException("Expected variable or type.");
            }

            final Matcher ampersant = AMPERSANT_START.matcher(s);
            if (!ampersant.find()) return types;
            s = s.subSequence(ampersant.end(), s.length());
        }
    }

    private List<BoundTemplate> maybeParseTemplate_() {
        final List<BoundTemplate> result = new ArrayList<>();

        eatWs_();
        if (s.length() == 0 || s.charAt(0) != '<') return EMPTY_LIST;
        s = s.subSequence(1, s.length());

        for (;;) {
            result.add(parse_());
            eatWs_();

            if (s.length() == 0)
                throw new IllegalArgumentException("Unterminated template.");
            if (s.charAt(0) == '>') {
                s = s.subSequence(1, s.length());
                return result;
            }
            if (s.charAt(0) != ',')
                throw new IllegalArgumentException("Template parameters must be separated by commas.");
            s = s.subSequence(1, s.length());
        }
    }
}
