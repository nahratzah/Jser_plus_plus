package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.misc.ListComparator;
import java.util.ArrayList;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singleton;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

/**
 * A template with bound names.
 *
 * @author ariane
 */
public interface BoundTemplate extends Type, Comparable<BoundTemplate> {
    @Override
    public default BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Collection<String> variables) {
        return this;
    }

    @Override
    public BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables);

    @Override
    public Set<String> getUnresolvedTemplateNames();

    /**
     * Rebind unbound template argument names.
     *
     * @param bindings Mapping of name to bound type.
     * @return A copy of this bound template, where each occurance of unresolved
     * template variabe in bindings has been replaced with its mapping.
     */
    public BoundTemplate rebind(Map<String, ? extends BoundTemplate> bindings);

    /**
     * Visit specializations.
     *
     * @param <T> Return type of the visitor.
     * @param v Visitor to apply.
     * @return Result of {@code v.apply(this)}.
     */
    public <T> T visit(Visitor<T> v);

    /**
     * Retrieve a name of the bound template style.
     *
     * @return One of "templateVar", "templateClass", "templateArray", or
     * "templateAny".
     */
    public default String getTemplateStyle() {
        return visit(new Visitor<String>() {
            @Override
            public String apply(VarBinding b) {
                return "templateVar";
            }

            @Override
            public String apply(ClassBinding<?> b) {
                return "templateClass";
            }

            @Override
            public String apply(ArrayBinding b) {
                return "templateArray";
            }

            @Override
            public String apply(Any b) {
                return "templateAny";
            }

            @Override
            public String apply(MultiType b) {
                return "templateMulti";
            }
        });
    }

    @Override
    public default Stream<String> getIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        final Visitor<Stream<String>> includeVisitor = new Visitor<Stream<String>>() {
            @Override
            public Stream<String> apply(VarBinding b) {
                return Stream.empty();
            }

            @Override
            public Stream<String> apply(ClassBinding<?> b) {
                return Stream.concat(
                        b.getType().getIncludes(),
                        b.getBindings().stream().flatMap(t -> t.visit(this)));
            }

            @Override
            public Stream<String> apply(ArrayBinding b) {
                return Stream.concat(
                        Stream.of("java/array.h"),
                        b.getType().visit(this));
            }

            @Override
            public Stream<String> apply(Any b) {
                return Stream.concat(b.getExtendTypes().stream(), b.getSuperTypes().stream())
                        .flatMap(t -> t.visit(this));
            }

            @Override
            public Stream<String> apply(MultiType b) {
                return b.getTypes().stream()
                        .flatMap(type -> type.visit(this));
            }
        };

        return visit(includeVisitor);
    }

    /**
     * Detect if this template refers to a primitive type.
     *
     * @return True of the template renders a primitive type, false otherwise.
     */
    public default boolean isPrimitive() {
        return visit(new Visitor<Boolean>() {
            @Override
            public Boolean apply(VarBinding b) {
                return false;
            }

            @Override
            public Boolean apply(ClassBinding<?> b) {
                return b.getType() instanceof PrimitiveType;
            }

            @Override
            public Boolean apply(ArrayBinding b) {
                return false;
            }

            @Override
            public Boolean apply(Any b) {
                return false;
            }

            @Override
            public Boolean apply(MultiType b) {
                return false;
            }
        });
    }

    /**
     * Visitor to apply the visitor pattern.
     *
     * @param <T> The return type of the visitor.
     */
    public static interface Visitor<T> {
        public T apply(VarBinding b);

        public T apply(ClassBinding<?> b);

        public T apply(ArrayBinding b);

        public T apply(Any b);

        public T apply(MultiType b);
    }

    /**
     * Binding to a template variable.
     */
    public static final class VarBinding implements BoundTemplate {
        public VarBinding() {
        }

        public VarBinding(String name) {
            this.name = name;
        }

        @Override
        public BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
            final BoundTemplate mapped = variables.get(name);
            return (mapped != null ? mapped : this);
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        @Override
        public Stream<JavaType> getAllJavaTypes() {
            return Stream.empty();
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return singleton(name);
        }

        @Override
        public BoundTemplate rebind(Map<String, ? extends BoundTemplate> bindings) {
            final BoundTemplate replacement = bindings.get(getName());
            if (replacement != null) return replacement;
            return new VarBinding(getName());
        }

        /**
         * Visit specialization.
         *
         * @param <T> Return type of the visitor.
         * @param v Visitor to apply.
         * @return Result of
         * {@link Visitor#apply(com.github.nahratzah.jser_plus_plus.config.cplusplus.BoundTemplate.VarBinding) visitor.apply(this)}.
         */
        @Override
        public <T> T visit(Visitor<T> v) {
            return v.apply(this);
        }

        @Override
        public int hashCode() {
            int hash = 5;
            hash = 37 * hash + Objects.hashCode(this.name);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final VarBinding other = (VarBinding) obj;
            if (!Objects.equals(this.name, other.name)) return false;
            return true;
        }

        @Override
        public int compareTo(BoundTemplate o) {
            if (!getClass().isInstance(o))
                return getClass().getName().compareTo(o.getClass().getName());

            return name.compareTo(((VarBinding) o).name);
        }

        @Override
        public String toString() {
            return getName();
        }

        private String name;
    }

    /**
     * Binding to a class.
     *
     * @param <T> The type of {@link JavaType} held by this
     * {@link ClassBinding}.
     */
    public static final class ClassBinding<T extends JavaType> implements BoundTemplate {
        public ClassBinding() {
        }

        public ClassBinding(T type, List<BoundTemplate> bindings) {
            this.type = type;
            this.bindings = bindings;
        }

        @Override
        public ClassBinding<T> prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
            final List<BoundTemplate> rebound = bindings.stream()
                    .map(binding -> binding.prerender(ctx, renderArgs, variables))
                    .collect(Collectors.toList());
            if (Objects.equals(bindings, rebound)) return this;
            return new ClassBinding<>(type, rebound);
        }

        public T getType() {
            return type;
        }

        public void setType(T type) {
            this.type = type;
        }

        public List<BoundTemplate> getBindings() {
            return bindings;
        }

        public void setBindings(List<BoundTemplate> bindings) {
            this.bindings = bindings;
        }

        /**
         * Retrieve a mapping between the types in the class and its bound
         * arguments.
         *
         * @return Mapping from class-template-name to
         * {@link BoundTemplate bound type}.
         */
        public Map<String, BoundTemplate> getBindingsMap() {
            final Map<String, BoundTemplate> bindingsMap = new HashMap<>();

            final Iterator<String> nameIter = getType().getTemplateArgumentNames().iterator();
            final Iterator<BoundTemplate> typeIter = getBindings().iterator();
            while (nameIter.hasNext() && typeIter.hasNext())
                bindingsMap.put(nameIter.next(), typeIter.next());

            if (nameIter.hasNext() || typeIter.hasNext())
                throw new IllegalStateException("Mismatch between accepted and supplied template arguments for " + this);

            return bindingsMap;
        }

        @Override
        public Stream<JavaType> getAllJavaTypes() {
            return Stream.concat(
                    Stream.of(getType()),
                    bindings.stream().flatMap(Type::getAllJavaTypes));
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return bindings.stream()
                    .map(BoundTemplate::getUnresolvedTemplateNames)
                    .flatMap(Collection::stream)
                    .collect(Collectors.toSet());
        }

        @Override
        public ClassBinding<T> rebind(Map<String, ? extends BoundTemplate> bindings) {
            final List<BoundTemplate> newBindings = getBindings().stream()
                    .map(binding -> binding.rebind(bindings))
                    .collect(Collectors.toList());
            return new ClassBinding<>(getType(), newBindings);
        }

        /**
         * Visit specialization.
         *
         * @param <T> Return type of the visitor.
         * @param v Visitor to apply.
         * @return Result of
         * {@link Visitor#apply(com.github.nahratzah.jser_plus_plus.config.cplusplus.BoundTemplate.ClassBinding) visitor.apply(this)}.
         */
        @Override
        public <T> T visit(Visitor<T> v) {
            return v.apply(this);
        }

        @Override
        public int hashCode() {
            int hash = 3;
            hash = 97 * hash + Objects.hashCode(this.type);
            hash = 97 * hash + Objects.hashCode(this.bindings);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final ClassBinding<?> other = (ClassBinding<?>) obj;
            if (!Objects.equals(this.type, other.type)) return false;
            if (!Objects.equals(this.bindings, other.bindings)) return false;
            return true;
        }

        @Override
        public int compareTo(BoundTemplate o) {
            if (!getClass().isInstance(o))
                return getClass().getName().compareTo(o.getClass().getName());

            int cmp = 0;
            if (cmp == 0)
                cmp = type.getName().compareTo(((ClassBinding<?>) o).type.getName());
            if (cmp == 0)
                cmp = new ListComparator<>().compare(bindings, ((ClassBinding<?>) o).bindings);
            return cmp;
        }

        @Override
        public String toString() {
            final String bindingsStr;
            if (bindings == null || bindings.isEmpty())
                bindingsStr = "";
            else
                bindingsStr = bindings.stream().map(Object::toString).collect(Collectors.joining(", ", "<", ">"));
            return type.getName() + bindingsStr;
        }

        private T type;
        private List<BoundTemplate> bindings;
    }

    /**
     * Binding to an array.
     */
    public static final class ArrayBinding implements BoundTemplate {
        public ArrayBinding() {
        }

        public ArrayBinding(BoundTemplate type, int extents) {
            if (type instanceof ArrayBinding) {
                this.type = ((ArrayBinding) type).getType();
                this.extents = ((ArrayBinding) type).getExtents() + extents;
            } else {
                this.type = type;
                this.extents = extents;
            }
        }

        @Override
        public BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
            final BoundTemplate prerenderedType = type.prerender(ctx, renderArgs, variables);
            return (prerenderedType == type ? this : new ArrayBinding(prerenderedType, extents));
        }

        public BoundTemplate getType() {
            return type;
        }

        public void setType(BoundTemplate type) {
            this.type = type;
        }

        public int getExtents() {
            return extents;
        }

        public void setExtents(int extents) {
            this.extents = extents;
        }

        @Override
        public Stream<JavaType> getAllJavaTypes() {
            return getType().getAllJavaTypes();
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return type.getUnresolvedTemplateNames();
        }

        @Override
        public ArrayBinding rebind(Map<String, ? extends BoundTemplate> bindings) {
            return new ArrayBinding(getType().rebind(bindings), getExtents());
        }

        /**
         * Visit specialization.
         *
         * @param <T> Return type of the visitor.
         * @param v Visitor to apply.
         * @return Result of
         * {@link Visitor#apply(com.github.nahratzah.jser_plus_plus.config.cplusplus.BoundTemplate.ArrayBinding) visitor.apply(this)}.
         */
        @Override
        public <T> T visit(Visitor<T> v) {
            return v.apply(this);
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 83 * hash + Objects.hashCode(this.type);
            hash = 83 * hash + this.extents;
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final ArrayBinding other = (ArrayBinding) obj;
            if (this.extents != other.extents) return false;
            if (!Objects.equals(this.type, other.type)) return false;
            return true;
        }

        @Override
        public int compareTo(BoundTemplate o) {
            if (!getClass().isInstance(o))
                return getClass().getName().compareTo(o.getClass().getName());

            int cmp = 0;
            if (cmp == 0) cmp = type.compareTo(((ArrayBinding) o).type);
            if (cmp == 0)
                cmp = Integer.compare(extents, ((ArrayBinding) o).extents);
            return cmp;
        }

        @Override
        public String toString() {
            String typeStr = type.toString();
            if (typeStr.indexOf(' ') != -1) typeStr = "(" + typeStr + ")";
            return typeStr + IntStream.range(0, extents).mapToObj(i -> "[]").collect(Collectors.joining());
        }

        private BoundTemplate type;
        private int extents;
    }

    /**
     * Wildcard binding.
     */
    public static final class Any implements BoundTemplate {
        public Any() {
            this(EMPTY_LIST, EMPTY_LIST);
        }

        public Any(Collection<BoundTemplate> superTypes, Collection<BoundTemplate> extendTypes) {
            this.superTypes = new TreeSet<>(requireNonNull(superTypes));
            this.extendTypes = new TreeSet<>(requireNonNull(extendTypes));
        }

        @Override
        public BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
            final List<BoundTemplate> newSuperTypes = superTypes.stream()
                    .map(type -> type.prerender(ctx, renderArgs, variables))
                    .collect(Collectors.toList());
            final List<BoundTemplate> newExtendTypes = extendTypes.stream()
                    .map(type -> type.prerender(ctx, renderArgs, variables))
                    .collect(Collectors.toList());
            if (Objects.equals(superTypes, newSuperTypes) && Objects.equals(extendTypes, newExtendTypes))
                return this;
            return new Any(newSuperTypes, newExtendTypes);
        }

        public Collection<BoundTemplate> getSuperTypes() {
            return superTypes;
        }

        public void setSuperTypes(Collection<BoundTemplate> superTypes) {
            this.superTypes = new TreeSet<>(superTypes);
        }

        public Collection<BoundTemplate> getExtendTypes() {
            return extendTypes;
        }

        public void setExtendTypes(Collection<BoundTemplate> extendTypes) {
            this.extendTypes = new TreeSet<>(extendTypes);
        }

        @Override
        public Stream<JavaType> getAllJavaTypes() {
            return Stream.concat(getSuperTypes().stream(), getExtendTypes().stream())
                    .flatMap(Type::getAllJavaTypes);
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return Stream.concat(superTypes.stream(), extendTypes.stream())
                    .map(BoundTemplate::getUnresolvedTemplateNames)
                    .flatMap(Collection::stream)
                    .collect(Collectors.toSet());
        }

        @Override
        public Any rebind(Map<String, ? extends BoundTemplate> bindings) {
            final List<BoundTemplate> newSuperTypes = getSuperTypes().stream()
                    .map(type -> type.rebind(bindings))
                    .collect(Collectors.toList());
            final List<BoundTemplate> newExtendTypes = getExtendTypes().stream()
                    .map(type -> type.rebind(bindings))
                    .collect(Collectors.toList());
            return new Any(newSuperTypes, newExtendTypes);
        }

        /**
         * Visit specialization.
         *
         * @param <T> Return type of the visitor.
         * @param v Visitor to apply.
         * @return Result of
         * {@link Visitor#apply(com.github.nahratzah.jser_plus_plus.config.cplusplus.BoundTemplate.Any) visitor.apply(this)}.
         */
        @Override
        public <T> T visit(Visitor<T> v) {
            return v.apply(this);
        }

        @Override
        public int hashCode() {
            int hash = 3;
            hash = 53 * hash + Objects.hashCode(this.superTypes);
            hash = 53 * hash + Objects.hashCode(this.extendTypes);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Any other = (Any) obj;
            if (!Objects.equals(this.superTypes, other.superTypes))
                return false;
            if (!Objects.equals(this.extendTypes, other.extendTypes))
                return false;
            return true;
        }

        @Override
        public int compareTo(BoundTemplate o) {
            if (!getClass().isInstance(o))
                return getClass().getName().compareTo(o.getClass().getName());

            int cmp = 0;
            if (cmp == 0)
                cmp = new ListComparator<>().compare(superTypes, ((Any) o).superTypes);
            if (cmp == 0)
                cmp = new ListComparator<>().compare(extendTypes, ((Any) o).extendTypes);
            return cmp;
        }

        @Override
        public String toString() {
            final StringBuilder spec = new StringBuilder("?");
            if (!superTypes.isEmpty()) {
                spec
                        .append(" super ")
                        .append(superTypes.stream()
                                .map(t -> {
                                    if (t instanceof Any) return "(" + t + ")";
                                    return t.toString();
                                })
                                .collect(Collectors.joining(" & ")));
            }
            if (!extendTypes.isEmpty()) {
                spec
                        .append(" extends ")
                        .append(extendTypes.stream()
                                .map(t -> {
                                    if (t instanceof Any) return "(" + t + ")";
                                    return t.toString();
                                })
                                .collect(Collectors.joining(" & ")));
            }

            return spec.toString();
        }

        private SortedSet<BoundTemplate> superTypes;
        private SortedSet<BoundTemplate> extendTypes;
    }

    public static final class MultiType implements BoundTemplate {
        public static BoundTemplate maybeMakeMultiType(Collection<? extends BoundTemplate> types) {
            if (types.size() == 1) return types.iterator().next();
            return new MultiType(types);
        }

        public MultiType() {
            this(EMPTY_LIST);
        }

        public MultiType(Collection<? extends BoundTemplate> types) {
            this.types = new TreeSet<>(requireNonNull(types));
        }

        public Set<BoundTemplate> getTypes() {
            return types;
        }

        public void setTypes(Collection<? extends BoundTemplate> types) {
            this.types = new TreeSet<>(requireNonNull(types));
        }

        @Override
        public BoundTemplate prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
            final Set<BoundTemplate> prerenderedTypes = types.stream()
                    .map(template -> template.prerender(ctx, renderArgs, variables))
                    .collect(Collectors.toSet());
            if (Objects.equals(types, prerenderedTypes)) return this;
            return new MultiType(prerenderedTypes);
        }

        @Override
        public Stream<JavaType> getAllJavaTypes() {
            return types.stream()
                    .flatMap(Type::getAllJavaTypes);
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return types.stream()
                    .map(BoundTemplate::getUnresolvedTemplateNames)
                    .flatMap(Collection::stream)
                    .collect(Collectors.toSet());
        }

        @Override
        public MultiType rebind(Map<String, ? extends BoundTemplate> bindings) {
            return new MultiType(types.stream()
                    .map(type -> type.rebind(bindings))
                    .collect(Collectors.toList()));
        }

        /**
         * Visit specialization.
         *
         * @param <T> Return type of the visitor.
         * @param v Visitor to apply.
         * @return Result of
         * {@link Visitor#apply(com.github.nahratzah.jser_plus_plus.config.cplusplus.BoundTemplate.Any) visitor.apply(this)}.
         */
        @Override
        public <T> T visit(Visitor<T> v) {
            return v.apply(this);
        }

        @Override
        public int hashCode() {
            int hash = 3;
            hash = 47 * hash + Objects.hashCode(this.types);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final MultiType other = (MultiType) obj;
            if (!Objects.equals(this.types, other.types)) return false;
            return true;
        }

        @Override
        public int compareTo(BoundTemplate o) {
            if (!getClass().isInstance(o))
                return getClass().getName().compareTo(o.getClass().getName());

            int cmp = 0;
            if (cmp == 0)
                cmp = new ListComparator<>().compare(types, ((MultiType) o).types);
            return cmp;
        }

        @Override
        public String toString() {
            return types.stream()
                    .map(Object::toString)
                    .collect(Collectors.joining(" & ", "[", "]"));
        }

        private Set<BoundTemplate> types;
    }

    public static Type fromString(String text, Context ctx, Map<String, ? extends BoundTemplate> variables) {
        return new BoundTemplateParser(ctx, variables).parse(text);
    }
}

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
     * The string on which we are attempting to match.
     */
    private CharSequence s;

    public BoundTemplateParser(Context ctx, Map<String, ? extends BoundTemplate> variables) {
        this.ctx = requireNonNull(ctx);
        this.variables = variables;
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
            final JavaType type = ctx.resolveClass(cls.group().replaceAll("\\s", ""));
            s = s.subSequence(cls.end(), s.length());
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
