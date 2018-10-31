package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;

/**
 * Model selecting a template variable.
 *
 * @author ariane
 */
public interface TemplateSelector {
    /**
     * The header file that needs to be included to the selector logic.
     */
    public static final String HEADER = "java/_template_selector.h";

    /**
     * Retrieve the subselector for resolution.
     *
     * The subselector is the type invocation in
     * {@literal ::java::_template_selector} that is to be invoked to resolve
     * the type. See also {@literal java/_template_selector.h}.
     *
     * You probably should invoke
     * {@link #selector(java.lang.String, boolean) selector()} instead of this.
     *
     * @param classSelector If set, return a class selector instead of a
     * generics selector.
     * @return A subselector string.
     */
    public String subSelector(boolean classSelector);

    /**
     * Render the selector.
     *
     * @param inputType A string representing the input type of the selector.
     * @param preselectedType A string representing the preselected input type
     * of the selector. If this type is not void, the selector will simply
     * ignore any instructions and return this type.
     * @param classSelector If set, return a class selector, otherwise return a
     * generics selector.
     * @return Selector statement for the given input type. Note that the
     * returned type if not prefixed with {@literal typename}.
     */
    public default String selector(String inputType, String preselectedType, boolean classSelector) {
        return String.format("::java::_template_selector<%s, %s>%s", inputType, preselectedType, subSelector(classSelector));
    }

    /**
     * Selector that simply selects the current type.
     *
     * This is the last selector in any selection chain.
     */
    public static final class Type implements TemplateSelector {
        @Override
        public String subSelector(boolean classSelector) {
            return (classSelector ? "::var_type" : "::type");
        }

        @Override
        public int hashCode() {
            return 0;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Type other = (Type) obj;
            return true;
        }
    }

    /**
     * Selector that takes an array type and removes a number of extents from
     * the array.
     */
    public static final class Array implements TemplateSelector {
        public Array(int removeExtents, TemplateSelector next) {
            this.removeExtents = removeExtents;
            this.next = requireNonNull(next);

            if (this.removeExtents < 0)
                throw new IndexOutOfBoundsException("Negative extent count for selector array.");
        }

        public int getRemoveExtents() {
            return removeExtents;
        }

        public TemplateSelector getNext() {
            return next;
        }

        @Override
        public String subSelector(boolean classSelector) {
            return String.format("::template array<%d>%s", removeExtents, next.subSelector(classSelector));
        }

        @Override
        public int hashCode() {
            int hash = 3;
            hash = 73 * hash + this.removeExtents;
            hash = 73 * hash + Objects.hashCode(this.next);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Array other = (Array) obj;
            if (this.removeExtents != other.removeExtents) return false;
            if (!Objects.equals(this.next, other.next)) return false;
            return true;
        }

        private final int removeExtents;
        private final TemplateSelector next;
    }

    /**
     * Selector that takes a template type and resolves a bound generic of that
     * type.
     *
     * @param <T> Type of the base class.
     */
    public static final class Binding<T extends JavaType> implements TemplateSelector {
        public Binding(T baseClass, int index, TemplateSelector next) {
            this.baseClass = requireNonNull(baseClass);
            this.index = index;
            this.next = requireNonNull(next);

            if (this.index < 0)
                throw new IndexOutOfBoundsException("Negative index for selector binding.");
            if (this.index >= this.baseClass.getNumTemplateArguments())
                throw new IndexOutOfBoundsException("Template selector has binding index (" + this.index + ") that exceeds number of template arguments to type " + this.baseClass.getName());
        }

        public T getBaseClass() {
            return baseClass;
        }

        public int getIndex() {
            return index;
        }

        public TemplateSelector getNext() {
            return next;
        }

        @Override
        public String subSelector(boolean classSelector) {
            final String tagType = StCtx.BUILTINS.getInstanceOf("tagType")
                    .add("model", baseClass)
                    .render(Locale.ROOT);
            return String.format("::template binding<%s, %d>%s", tagType, index, next.subSelector(classSelector));
        }

        @Override
        public int hashCode() {
            int hash = 5;
            hash = 89 * hash + Objects.hashCode(this.baseClass);
            hash = 89 * hash + this.index;
            hash = 89 * hash + Objects.hashCode(this.next);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Binding<?> other = (Binding<?>) obj;
            if (this.index != other.index) return false;
            if (!Objects.equals(this.baseClass, other.baseClass)) return false;
            if (!Objects.equals(this.next, other.next)) return false;
            return true;
        }

        private final T baseClass;
        private final int index;
        private final TemplateSelector next;
    }
}
