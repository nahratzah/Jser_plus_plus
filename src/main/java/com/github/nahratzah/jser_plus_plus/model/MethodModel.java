package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.List;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Stream;

/**
 * Interface that is implemented by class methods.
 *
 * Used to keep all the class methods in sync.
 *
 * @author ariane
 */
public interface MethodModel {
    /**
     * The class declaring the method.
     *
     * @return Class owning this method.
     */
    public ClassType getDeclaringClass();

    /**
     * Method name.
     *
     * @return The name of the method.
     */
    public String getName();

    /**
     * Includes required to declare and implement the method.
     *
     * Includes that can be deduced from
     * {@link #getDeclarationTypes() declaration types} or
     * {@link #getImplementationTypes() implementation types} may be omitted.
     *
     * @return Includes for the method.
     */
    public Includes getIncludes();

    /**
     * Retrieve all includes from the types mentioned for declaration of the
     * method.
     *
     * @return List of includes at declaration time.
     */
    public default Stream<String> getDeclarationIncludes() {
        return Stream.of(
                getDeclarationTypes().flatMap(type -> type.getIncludes(true)),
                getIncludes().getDeclarationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)),
                getReturnType().getIncludes(true))
                .flatMap(Function.identity());
    }

    /**
     * Retrieve all includes from the types mentioned for implementation of the
     * method.
     *
     * @return List of includes at implementation time.
     */
    public default Stream<String> getImplementationIncludes() {
        return Stream.of(
                getDeclarationTypes().flatMap(type -> type.getIncludes(false)),
                getImplementationTypes().flatMap(type -> type.getIncludes(false)),
                getIncludes().getImplementationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(false)),
                getReturnType().getIncludes(false))
                .flatMap(Function.identity());
    }

    /**
     * Return all types required to create the method declaration.
     *
     * @return All types required to create the declaration.
     */
    public Stream<Type> getDeclarationTypes();

    /**
     * Return all types required to create the method implementation.
     *
     * @return All types required to create the implementation.
     */
    public Stream<Type> getImplementationTypes();

    /**
     * The type returned by the method.
     *
     * @return The result type of the method.
     */
    public Type getReturnType();

    /**
     * Types of the arguments.
     *
     * There must always be the same number of
     * {@link #getArgumentTypes() argument types} as there are
     * {@link #getArgumentNames() argument names}. s
     *
     *
     * @return List of argument types.
     */
    public List<Type> getArgumentTypes();

    /**
     * Names of the arguments.
     *
     * There must always be the same number of
     * {@link #getArgumentTypes() argument types} as there are
     * {@link #getArgumentNames() argument names}.
     *
     * @return List of argument names.
     */
    public List<String> getArgumentNames();

    /**
     * Retrieve the body of the method.
     *
     * @return String containing the body of the method.
     */
    public String getBody();

    /**
     * Test if the method is static.
     *
     * @return True if the method is static.
     */
    public boolean isStatic();

    /**
     * Test if the method is virtual.
     *
     * @return True if the method is a virtual method.
     */
    public boolean isVirtual();

    /**
     * Test if the method is a pure virtual method.
     *
     * Pure virtual methods only have a declaration, but no implementation.
     *
     * @return True if the method is a pure virtual function.
     */
    public boolean isPureVirtual();

    /**
     * Test if the method is marked as override.
     *
     * A method marked override must override a similar function from its base
     * class.
     *
     * @return True if the method is an override method.
     */
    public boolean isOverride();

    /**
     * Test if the method is a const method.
     *
     * Const methods may not mutate their class instance.
     *
     * @return True if the method is a const method.
     */
    public boolean isConst();

    /**
     * Test if the method is marked as final.
     *
     * A final method may not be overriden in derived classes.
     *
     * @return True if the method is final.
     */
    public boolean isFinal();

    /**
     * Return the noexcept specification of the function.
     *
     * If the noexcept specification is null, there is no noexcept
     * specification.
     *
     * @return An object that is either a {@link String string} or a
     * {@link org.stringtemplate.v4.ST string template}.
     */
    public Object getNoexcept();

    /**
     * Method {@link Visibility visibility}.
     *
     * @return The {@link Visibility visibility} of the method.
     */
    public Visibility getVisibility();

    /**
     * The documentation of the function.
     *
     * @return Doxygen documentation of the method.
     */
    public String getDocString();

    /**
     * Retrieve the override selector of the method.
     *
     * @param ctx Lookup context to resolve classes.
     * @return An optional containing an override selector.
     */
    public Optional<ClassMemberModel.OverrideSelector> getOverrideSelector(Context ctx);
}
