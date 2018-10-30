package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.TemplateSelector;
import com.github.nahratzah.jser_plus_plus.model.Type;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Streams;
import java.util.List;
import static java.util.Objects.requireNonNull;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Accessor constructor.
 *
 * @author ariane
 */
public interface AccessorConstructor {
    /**
     * Type model.
     *
     * @return The model on which this method is emitted.
     */
    public ClassType getModel();

    /**
     * Name of the method.
     *
     * @return A method name.
     */
    public default String getName() {
        return "operator()";
    }

    /**
     * Return type of the method. Null if the method does not have a return
     * type.
     *
     * @return Return type of the method, or null.
     */
    public default ST getReturnType() {
        return new ST(StCtx.BUILTINS, "::cycle_ptr::cycle_gptr<$erasedType(m.model)$>")
                .add("m", this);
    }

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
        return Streams.concat(
                Stream.of("java/inline.h", "cycle_ptr/cycle_ptr.h"),
                (getFunctionGenericsNames().isEmpty() ? Stream.empty() : Stream.of(TemplateSelector.HEADER)),
                getDeclarationTypes().flatMap(type -> type.getIncludes(true)),
                getIncludes().getDeclarationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)));
    }

    /**
     * Retrieve all includes from the types mentioned for implementation of the
     * method.
     *
     * @return List of includes at implementation time.
     */
    public default Stream<String> getImplementationIncludes() {
        return Streams.concat(
                getDeclarationTypes().flatMap(type -> type.getIncludes(true)),
                getImplementationTypes().flatMap(type -> type.getIncludes(true)),
                getIncludes().getImplementationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)));
    }

    /**
     * Return all types required to create the method declaration.
     *
     * @return All types required to create the declaration.
     */
    public default Stream<Type> getDeclarationTypes() {
        return getArgumentTypes().stream();
    }

    /**
     * Return all types required to create the method implementation.
     *
     * @return All types required to create the implementation.
     */
    public default Stream<Type> getImplementationTypes() {
        return Stream.empty();
    }

    /**
     * Accessor method body.
     *
     * @return Method body.
     */
    public default ST getBody() {
        return new ST(StCtx.BUILTINS, "return ::cycle_ptr::make_cycle<$erasedType(m.model)$>($m.argumentNames: {name | ::std::forward<decltype($name$)>($name$)}; anchor, wrap = \"\\n\", separator = \", \"$);")
                .add("m", this);
    }

    /**
     * Indicator if this is a static method.
     *
     * @return False.
     */
    public default boolean isStatic() {
        return false;
    }

    /**
     * Test if the method is a const method.
     *
     * Const methods may not mutate their class instance.
     *
     * @return True.
     */
    public default boolean isConst() {
        return true;
    }

    /**
     * Return the noexcept specification of the function.
     *
     * If the noexcept specification is null, there is no noexcept
     * specification.
     *
     * @return null.
     */
    public default Object getNoexcept() {
        return null;
    }

    /**
     * Method {@link Visibility visibility}.
     *
     * @return The {@link Visibility visibility} of the method.
     */
    public Visibility getVisibility();

    /**
     * The documentation of the function.
     *
     * @return Doxygen documentation of the method, or null.
     */
    public String getDocString();

    /**
     * Get the inline declaration of this method.
     *
     * @return A string containing the inline declaration of this method. Or
     * null if this method isn't inlined.
     */
    public default String getInline() {
        return "JSER_INLINE";
    }

    /**
     * Retrieve the the names of generics specification local to the function.
     *
     * @return Ordered list of names.
     */
    public List<String> getFunctionGenericsNames();

    /**
     * Retrieve the defaults for the generics specification, local to the
     * function.
     *
     * @return Ordered list of defaults, by name.
     */
    public List<String> getFunctionGenericsDefault();

    public static class Impl implements AccessorConstructor {
        private final ClassType model;
        private final List<Type> argumentTypes;
        private final List<String> argumentNames;
        private final Includes includes;
        private final Visibility visibility;
        private final String docString;
        private final List<String> functionGenericsNames;
        private final List<String> functionGenericsDefault;

        public Impl(ClassType model, List<Type> argumentTypes, List<String> argumentNames, Includes includes, Visibility visibility, String docString, List<String> functionGenericsNames, List<String> functionGenericsDefault) {
            this.model = requireNonNull(model);
            this.argumentTypes = requireNonNull(argumentTypes);
            this.argumentNames = requireNonNull(argumentNames);
            this.includes = requireNonNull(includes);
            this.visibility = requireNonNull(visibility);
            this.docString = docString;
            this.functionGenericsNames = requireNonNull(functionGenericsNames);
            this.functionGenericsDefault = requireNonNull(functionGenericsDefault);
        }

        @Override
        public ClassType getModel() {
            return model;
        }

        @Override
        public List<Type> getArgumentTypes() {
            return argumentTypes;
        }

        @Override
        public List<String> getArgumentNames() {
            return argumentNames;
        }

        @Override
        public Includes getIncludes() {
            return includes;
        }

        @Override
        public Visibility getVisibility() {
            return visibility;
        }

        @Override
        public String getDocString() {
            return docString;
        }

        @Override
        public List<String> getFunctionGenericsNames() {
            return functionGenericsNames;
        }

        @Override
        public List<String> getFunctionGenericsDefault() {
            return functionGenericsDefault;
        }
    }
}
