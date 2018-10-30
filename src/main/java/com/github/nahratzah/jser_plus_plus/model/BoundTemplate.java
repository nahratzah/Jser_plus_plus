package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.misc.ListComparator;
import com.google.common.collect.Streams;
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
import java.util.function.Function;
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
    public default boolean isJavaType() {
        return !isPrimitive();
    }

    @Override
    public Set<String> getUnresolvedTemplateNames();

    /**
     * Rebind unbound template argument names.
     *
     * @param bindings Mapping of name to bound type.
     * @return A copy of this bound template, where each occurance of unresolved
     * template variabe in bindings has been replaced with its mapping.
     */
    @Override
    public BoundTemplate rebind(Map<String, ? extends BoundTemplate> bindings);

    /**
     * Compute a selector to read a type from this template.
     *
     * @param varName The name of a variable to look up.
     * @return A stream of selectors that resolves the type of that variable.
     */
    public Stream<TemplateSelector> findSelectorFor(String varName);

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

        @Override
        public Stream<TemplateSelector> findSelectorFor(String varName) {
            if (Objects.equals(varName, getName()))
                return Stream.of(new TemplateSelector.Type());
            return Stream.empty();
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

        @Override
        public Stream<TemplateSelector> findSelectorFor(String varName) {
            return Streams.mapWithIndex(
                    bindings.stream(),
                    (binding, idx) -> {
                        if (idx > Integer.MAX_VALUE)
                            throw new IndexOutOfBoundsException("Too many bindings to resolve a path.");
                        final Stream<TemplateSelector> selectors = binding.findSelectorFor(varName)
                                .map(selector -> new TemplateSelector.Binding<>(getType(), (int) idx, selector));
                        return selectors;
                    })
                    .flatMap(Function.identity());
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

        @Override
        public Stream<TemplateSelector> findSelectorFor(String varName) {
            return getType().findSelectorFor(varName)
                    .map(selector -> new TemplateSelector.Array(extents, selector));
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

        @Override
        public Stream<TemplateSelector> findSelectorFor(String varName) {
            return Stream.empty();
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

        @Override
        public Stream<TemplateSelector> findSelectorFor(String varName) {
            return types.stream()
                    .flatMap(type -> type.findSelectorFor(varName));
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

    public static Type fromString(String text, Context ctx, Map<String, ? extends BoundTemplate> variables, BoundTemplate.ClassBinding<?> thisType) {
        return new BoundTemplateParser(ctx, variables, thisType).parse(text);
    }
}
