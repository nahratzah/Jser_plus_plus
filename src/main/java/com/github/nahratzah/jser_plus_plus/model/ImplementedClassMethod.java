package com.github.nahratzah.jser_plus_plus.model;

import java.util.Collection;
import static java.util.Collections.unmodifiableSet;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Models a method that has been implemented on a class.
 *
 * @author ariane
 */
public class ImplementedClassMethod {
    private ImplementedClassMethod(ClassType cls, ClassType tagType, OverrideSelector selector, OverrideSelector erasedSelector, Set<ImplementedClassMethod> overridenMethods) {
        this.cls = requireNonNull(cls);
        this.tagType = tagType;
        this.selector = requireNonNull(selector);
        this.erasedSelector = requireNonNull(erasedSelector);
        this.overridenMethods = requireNonNull(overridenMethods);

        if (!erasedSelector.getUnderlyingMethod().isVirtual() && !this.overridenMethods.isEmpty())
            throw new IllegalArgumentException("non-virtual method overrides methods");

        if (erasedSelector.getUnderlyingMethod().isCovariantReturn()) {
            requireNonNull(tagType);
            if (cls == tagType) {
                // Skip
            } else if (this.overridenMethods.stream().anyMatch(overridenMethod -> overridenMethod.getTagType() == tagType)) {
                // Skip
            } else {
                throw new IllegalArgumentException("tag type must be either from declaring class or one of the overriden classes");
            }
        } else if (tagType != null) {
            throw new IllegalArgumentException("tag must be null for non-covariant-return function");
        }

        this.overridenMethods.forEach(overridenMethod -> {
            if (!Objects.equals(selector.getName(), overridenMethod.getSelector().getName()))
                throw new IllegalArgumentException("mismatched name for override method");
            if (!Objects.equals(selector.isConst(), overridenMethod.getSelector().isConst()))
                throw new IllegalArgumentException("mismatch in const-ness of override method");
            if (erasedSelector.getUnderlyingMethod().isCovariantReturn()
                    != overridenMethod.getErasedSelector().getUnderlyingMethod().isCovariantReturn())
                throw new IllegalArgumentException("mismatch for covariant return option");
        });
    }

    public ImplementedClassMethod(ClassType cls, ClassType tagType, OverrideSelector selector, Stream<ImplementedClassMethod> overridenMethods) {
        this(
                cls, tagType, selector, requireNonNull(selector).getErasedMethod(requireNonNull(cls)),
                unmodifiableSet(overridenMethods
                        .peek(Objects::requireNonNull)
                        .collect(Collectors.toSet())));
    }

    public ImplementedClassMethod(ClassType cls, ClassType tagType, OverrideSelector selector, Collection<ImplementedClassMethod> overridenMethods) {
        this(cls, tagType, selector, overridenMethods.stream());
    }

    public ImplementedClassMethod rebind(Map<String, ? extends BoundTemplate> rebindMap) {
        final Set<ImplementedClassMethod> reboundOverridenMethods = overridenMethods.stream()
                .map(overridenMethod -> overridenMethod.rebind(rebindMap))
                .collect(Collectors.toSet());
        return new ImplementedClassMethod(cls, tagType, selector.rebind(rebindMap), erasedSelector, unmodifiableSet(reboundOverridenMethods));
    }

    public ClassType getCls() {
        return cls;
    }

    public OverrideSelector getSelector() {
        return selector;
    }

    public OverrideSelector getErasedSelector() {
        return erasedSelector;
    }

    public Set<ImplementedClassMethod> getOverridenMethods() {
        return overridenMethods;
    }

    public ClassType getTagType() {
        return tagType;
    }

    public boolean hasChangedTypes(ClassType implementingClass) {
        final Map<String, BoundTemplate> implementingBindings = implementingClass.getErasedTemplateArguments().stream()
                .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
        final OverrideSelector implementingSelector = selector.rebind(implementingBindings);
        return !Objects.equals(implementingSelector, erasedSelector)
                || !Objects.equals(implementingSelector.getReturnType(), erasedSelector.getReturnType());
    }

    /**
     * Retrieve all methods that are overriden by this method.
     *
     * @return Stream of {@link ImplementedClassMethod implemented methods} that
     * are overriden by this {@link ImplementedClassMethod method}.
     */
    public Stream<ImplementedClassMethod> getAllOverridenMethods() {
        return Stream.concat(
                overridenMethods.stream(),
                overridenMethods.stream().flatMap(ImplementedClassMethod::getAllOverridenMethods));
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 41 * hash + Objects.hashCode(this.cls);
        hash = 41 * hash + Objects.hashCode(this.erasedSelector);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final ImplementedClassMethod other = (ImplementedClassMethod) obj;
        if (!Objects.equals(this.cls, other.cls)) return false;
        if (!Objects.equals(this.erasedSelector, other.erasedSelector))
            return false;
        return true;
    }

    @Override
    public String toString() {
        return selector.toString() + " at " + cls.getName();
    }

    private final ClassType cls;
    private final OverrideSelector selector;
    private final OverrideSelector erasedSelector;
    private final Set<ImplementedClassMethod> overridenMethods;
    private final ClassType tagType;
}
