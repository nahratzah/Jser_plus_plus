package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.JavaType;
import com.github.nahratzah.jser_plus_plus.model.MethodGenerics;
import com.github.nahratzah.jser_plus_plus.model.TemplateSelector;
import com.github.nahratzah.jser_plus_plus.model.Type;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Streams;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Interface for accessor methods.
 *
 * @author ariane
 */
public interface AccessorMethod {
    /**
     * Type model.
     *
     * @return The model on which this method is emitted.
     */
    public JavaType getModel();

    /**
     * Name of the method.
     *
     * @return A method name.
     */
    public String getName();

    /**
     * Return type of the method. Null if the method does not have a return
     * type.
     *
     * @return Return type of the method, or null.
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
                Stream.of("java/inline.h"),
                (getFunctionGenericsNames().isEmpty() ? Stream.empty() : Stream.of(TemplateSelector.HEADER)),
                getDeclarationTypes().flatMap(type -> type.getIncludes(true)),
                getIncludes().getDeclarationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)),
                getReturnType().getIncludes(true));
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
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)),
                getReturnType().getIncludes(false));
    }

    /**
     * Return all types required to create the method declaration.
     *
     * @return All types required to create the declaration.
     */
    public default Stream<Type> getDeclarationTypes() {
        return Stream.concat(
                Stream.of(getReturnType()),
                getArgumentTypes().stream());
    }

    /**
     * Return all types required to create the method implementation.
     *
     * @return All types required to create the implementation.
     */
    public default Stream<Type> getImplementationTypes() {
        return Stream.of(getReturnType());
    }

    /**
     * Test if this method is one that returns void.
     *
     * @return True if the method return type evaluates to void.
     */
    public default boolean isVoidReturnType() {
        Type type = getReturnType();
        if (type == null) return false;
        if (type instanceof ConstType)
            type = ((ConstType) type).getType();
        if (type instanceof CxxType)
            return "void".equals(((CxxType) type).getPreRendered().trim());
        return false;
    }

    /**
     * Test if we need to cast the return type.
     *
     * @return True if the return type needs to be casted. False otherwise.
     */
    public default boolean isReturnTypeCastRequired() {
        final Type returnType = getReturnType();
        if (getReturnType() == null || isVoidReturnType()) return false;

        final Set<String> unresolvedTemplateNames = returnType.getUnresolvedTemplateNames();
        if (unresolvedTemplateNames.isEmpty()) return false;
        return unresolvedTemplateNames.stream()
                .anyMatch(new HashSet<>(getModel().getTemplateArgumentNames())::contains);
    }

    /**
     * Accessor method body.
     *
     * @return Method body.
     */
    public default ST getBody() {
        final String templateString = "$if (m.returnType && !m.voidReturnType)$return $if (m.returnTypeCastRequired)$::java::cast<$boundTemplateType(m.returnType, \"style=type, class=return\")$>($endif$$endif$$if (m.static)$$erasedType(m.model)$::$else$this->template ref_<$erasedType(m.model)$>().$endif$$m.name$($m.argumentNames:{name | ::std::forward<decltype($name$)>($name$)}; anchor, wrap, separator = \", \"$)$if (m.returnTypeCastRequired)$)$endif$;";
        return new ST(StCtx.BUILTINS, templateString)
                .add("m", this);
    }

    /**
     * Indicator if this is a static method.
     *
     * @return True if this is a static method, false if this is an instance
     * method.
     */
    public boolean isStatic();

    /**
     * Test if the method is a const method.
     *
     * Const methods may not mutate their class instance.
     *
     * @return True if the method is a const method.
     */
    public boolean isConst();

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

    /**
     * Simple implementation of {@link AccessorMethod}.
     */
    public static class Impl implements AccessorMethod {
        private final JavaType model;
        private final String name;
        private final List<String> argumentNames;
        private final Type returnType;
        private final Includes includes;
        private final boolean staticVar;
        private final boolean constVar;
        private final Object noexcept;
        private final Visibility visibility;
        private final String docString;
        private final MethodGenerics generics;

        public Impl(JavaType model, String name, List<Type> argumentTypes, List<String> argumentNames, Type returnType, Includes includes, boolean staticVar, boolean constVar, Object noexcept, Visibility visibility, String docString, Map<String, BoundTemplate> methodGenerics) {
            this.model = requireNonNull(model);
            this.name = requireNonNull(name);
            this.argumentNames = requireNonNull(argumentNames);
            this.includes = requireNonNull(includes);
            this.staticVar = staticVar;
            this.constVar = constVar;
            this.noexcept = noexcept;
            this.visibility = requireNonNull(visibility);
            this.docString = docString;
            this.generics = new MethodGenerics(methodGenerics, argumentTypes);
            this.returnType = (returnType == null ? null : returnType.rebind(generics.getGenericNameToDerivationNameRebindMap()));
        }

        @Override
        public JavaType getModel() {
            return model;
        }

        @Override
        public String getName() {
            return name;
        }

        @Override
        public List<Type> getArgumentTypes() {
            return generics.getTemplatedArgumentTypes();
        }

        @Override
        public List<String> getArgumentNames() {
            return argumentNames;
        }

        @Override
        public Type getReturnType() {
            return returnType;
        }

        @Override
        public Includes getIncludes() {
            return includes;
        }

        @Override
        public boolean isStatic() {
            return staticVar;
        }

        @Override
        public boolean isConst() {
            return constVar;
        }

        @Override
        public Object getNoexcept() {
            return noexcept;
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
            return generics.getFunctionGenericsNames();
        }

        @Override
        public List<String> getFunctionGenericsDefault() {
            return generics.getFunctionGenericsDefault();
        }
    }
}
