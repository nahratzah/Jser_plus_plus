package com.github.nahratzah.jser_plus_plus.model;

import java.util.Collection;
import static java.util.Collections.emptySet;
import static java.util.Collections.singleton;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * A template with bound names.
 *
 * @author ariane
 */
public interface BoundTemplate {
    /**
     * Retrieve the list of parameter names that are not bound.
     *
     * @return List of parameter names that are not bound.
     */
    public Set<String> getUnresolvedTemplateNames();

    /**
     * Visit specializations.
     *
     * @param <T> Return type of the visitor.
     * @param v Visitor to apply.
     * @return Result of {@code v.apply(this)}.
     */
    public <T> T visit(Visitor<T> v);

    /**
     * Visitor to apply the visitor pattern.
     *
     * @param <T> The return type of the visitor.
     */
    public static interface Visitor<T> {
        public T apply(VarBinding b);

        public T apply(ClassBinding b);

        public T apply(ArrayBinding b);

        public T apply(Any b);
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
        public Set<String> getUnresolvedTemplateNames() {
            return singleton(name);
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

        private String name;
    }

    /**
     * Binding to a class.
     */
    public static final class ClassBinding implements BoundTemplate {
        public ClassBinding() {
        }

        public ClassBinding(JavaType type, List<BoundTemplate> bindings) {
            this.type = type;
            this.bindings = bindings;
        }

        public JavaType getType() {
            return type;
        }

        public void setType(JavaType type) {
            this.type = type;
        }

        public List<BoundTemplate> getBindings() {
            return bindings;
        }

        public void setBindings(List<BoundTemplate> bindings) {
            this.bindings = bindings;
        }

        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return bindings.stream()
                    .map(BoundTemplate::getUnresolvedTemplateNames)
                    .flatMap(Collection::stream)
                    .collect(Collectors.toSet());
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

        private JavaType type;
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
        public Set<String> getUnresolvedTemplateNames() {
            return type.getUnresolvedTemplateNames();
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

        private BoundTemplate type;
        private int extents;
    }

    /**
     * Wildcard binding.
     */
    public static class Any implements BoundTemplate {
        @Override
        public Set<String> getUnresolvedTemplateNames() {
            return emptySet();
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
    }
}