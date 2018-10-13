package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import static java.util.Collections.singletonMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.stream.Collectors;

/**
 * The significant portion of a method for override comparisons.
 *
 * Its {@link OverrideSelector#hashCode() hashCode()} and
 * {@link OverrideSelector#equals(java.lang.Object) equals()} methods are keyed
 * on the function name, modifiers, and arguments.
 *
 * @author ariane
 */
public class OverrideSelector {
    public OverrideSelector(Context ctx, MethodModel method) {
        this.ctx = requireNonNull(ctx);
        this.method = requireNonNull(method);
        this.declaringType = new BoundTemplate.ClassBinding<>(
                method.getDeclaringClass(),
                method.getDeclaringClass().getTemplateArgumentNames().stream()
                        .map(BoundTemplate.VarBinding::new)
                        .collect(Collectors.toList()));
        this.arguments = requireNonNull(method.getArgumentTypes()).stream()
                .map(type -> {
                    return type.prerender(
                            this.ctx,
                            singletonMap("model", requireNonNull(this.declaringType.getType())),
                            this.declaringType.getBindingsMap());
                })
                .collect(Collectors.toList());
        this.returnType = method.getReturnType()
                .prerender(this.ctx, singletonMap("model", this.declaringType.getType()), this.declaringType.getBindingsMap());
    }

    private OverrideSelector(OverrideSelector parent, Map<String, ? extends BoundTemplate> rebindMap) {
        this.ctx = requireNonNull(parent.ctx);
        this.method = requireNonNull(parent.method);
        this.declaringType = parent.declaringType.rebind(rebindMap);
        this.arguments = requireNonNull(method.getArgumentTypes()).stream()
                .map(type -> {
                    return type.prerender(
                            this.ctx,
                            singletonMap("model", requireNonNull(this.declaringType.getType())),
                            this.declaringType.getBindingsMap());
                })
                .collect(Collectors.toList());
        this.returnType = method.getReturnType()
                .prerender(this.ctx, singletonMap("model", this.declaringType.getType()), this.declaringType.getBindingsMap());
    }

    public OverrideSelector rebind(Map<String, ? extends BoundTemplate> rebindMap) {
        return new OverrideSelector(this, rebindMap);
    }

    /**
     * Retrieve the method with erasure applied.
     *
     * @return The underlying method with type erasure applied.
     */
    public OverrideSelector getErasedMethod() {
        final OverrideSelector erasedBaseCase = method.getOverrideSelector(ctx).orElseThrow(IllegalStateException::new);
        final BoundTemplate.ClassBinding<ClassType> erasedBaseClassType = new BoundTemplate.ClassBinding<>(
                erasedBaseCase.getDeclaringClass(),
                erasedBaseCase.getDeclaringClass().getErasedTemplateArguments().stream()
                        .map(Map.Entry::getValue)
                        .collect(Collectors.toList()));
        return erasedBaseCase.rebind(erasedBaseClassType.getBindingsMap());
    }

    /**
     * Test if the method had its arguments altered.
     *
     * @return True of the override has altered arguments, relative to the
     * erased base type.
     */
    public boolean hasChangedArguments() {
        return !Objects.equals(arguments, getErasedMethod().arguments);
    }

    /**
     * Test if the method had its return type altered.
     *
     * @return True if the override has an altered return type, relative to the
     * erased base type.
     */
    public boolean hasChangedReturnType() {
        return !Objects.equals(returnType, getErasedMethod().returnType);
    }

    /**
     * Test if any of the types of this method was changed from the original
     * method.
     *
     * @return True if any of the types of this method was changed, relative to
     * the base type.
     */
    public boolean hasAlteredTypes() {
        return hasChangedReturnType() || hasChangedArguments();
    }

    /**
     * The name of the method.
     *
     * This is significant in determining override mapping.
     *
     * @return The name of the method.
     */
    public String getName() {
        return method.getName();
    }

    /**
     * Const qualification of the method.
     *
     * This is significant in determining override mapping.
     *
     * @return True if the method is const qualified.
     */
    public boolean isConst() {
        return method.isConst();
    }

    /**
     * List of method arguments.
     *
     * This is significant in determining override mapping.
     *
     * @return The arguments of the method.
     */
    public List<Type> getArguments() {
        return arguments;
    }

    /**
     * Return type of the method.
     *
     * @return The return type of the method.
     */
    public Type getReturnType() {
        return returnType;
    }

    /**
     * Retrieve the declaring class of the method.
     *
     * @return Declaring class of the method.
     */
    public ClassType getDeclaringClass() {
        return getDeclaringType().getType();
    }

    /**
     * Retrieves the bound type of the declaring class.
     *
     * @return Declaring class with any variable substitutions applied.
     */
    public BoundTemplate.ClassBinding<ClassType> getDeclaringType() {
        return declaringType;
    }

    /**
     * Retrieve the method from which this
     * {@link OverrideSelector override selector} derives.
     *
     * @return The underlying method.
     */
    public MethodModel getUnderlyingMethod() {
        return method;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 97 * hash + Objects.hashCode(this.getName());
        hash = 97 * hash + (this.isConst() ? 1 : 0);
        hash = 97 * hash + Objects.hashCode(this.getArguments());
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final OverrideSelector other = (OverrideSelector) obj;
        if (this.isConst() != other.isConst()) return false;
        if (!Objects.equals(this.getName(), other.getName())) return false;
        if (!Objects.equals(this.getArguments(), other.getArguments()))
            return false;
        return true;
    }

    @Override
    public String toString() {
        return "auto "
                + getDeclaringType()
                + "::"
                + getName()
                + getArguments().stream()
                        .map(Object::toString)
                        .collect(Collectors.joining(", ", "(", ")"))
                + (isConst() ? " const" : "")
                + " -> "
                + getReturnType();
    }

    private final Context ctx; // Not significant for equality.
    private final BoundTemplate.ClassBinding<ClassType> declaringType; //Not significant for equality.
    private final MethodModel method; // Not significant for equality.
    private final List<Type> arguments; // Significant for equality.
    private final Type returnType; // Significant for equality.
}
