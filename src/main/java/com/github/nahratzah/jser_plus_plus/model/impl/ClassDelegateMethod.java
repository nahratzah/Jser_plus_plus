package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.MethodModel;
import com.github.nahratzah.jser_plus_plus.model.TemplateSelector;
import com.github.nahratzah.jser_plus_plus.model.Type;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Streams;
import java.util.List;
import java.util.Locale;
import static java.util.Objects.requireNonNull;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Delegate method for a class.
 *
 * @author ariane
 */
public interface ClassDelegateMethod extends MethodModel {
    /**
     * Type model.
     *
     * @return The model on which this method is emitted.
     */
    public ClassType getModel();

    @Override
    public default ClassType getDeclaringClass() {
        return getModel();
    }

    @Override
    public default BoundTemplate.ClassBinding<? extends ClassType> getArgumentDeclaringClass() {
        return getDeclaringClass().getBoundType();
    }

    /**
     * Name of the method.
     *
     * @return A method name.
     */
    @Override
    public String getName();

    /**
     * Name of the method to which we are delegating.
     *
     * @return A method name.
     */
    public String getDelegateName();

    /**
     * Return type of the method. Null if the method does not have a return
     * type.
     *
     * @return Return type of the method, or null.
     */
    @Override
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
    @Override
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
    @Override
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
    @Override
    public Includes getIncludes();

    /**
     * Retrieve all includes from the types mentioned for declaration of the
     * method.
     *
     * @return List of includes at declaration time.
     */
    @Override
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
    @Override
    public default Stream<String> getImplementationIncludes() {
        return Streams.concat(
                getDeclarationTypes().flatMap(type -> type.getIncludes(false)),
                getImplementationTypes().flatMap(type -> type.getIncludes(false)),
                getIncludes().getImplementationIncludes().stream(),
                getArgumentTypes().stream().flatMap(at -> at.getIncludes(false)),
                getReturnType().getIncludes(false));
    }

    /**
     * Return all types required to create the method declaration.
     *
     * @return All types required to create the declaration.
     */
    @Override
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
    @Override
    public default Stream<Type> getImplementationTypes() {
        return getDeclarationTypes();
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
     * Accessor method body.
     *
     * @return Method body.
     */
    @Override
    public default String getBody() {
        final String templateString = "$if (m.returnType && !m.voidReturnType)$return $if (m.returnType.unresolvedTemplateNames)$::java::cast<$boundTemplateType(m.returnType, \"style=type, class=return\")$>($endif$$endif$$m.delegateName$($m.argumentNames:{name | ::std::forward<decltype($name$)>($name$)}; anchor, wrap, separator = \", \"$)$if (m.returnType && !m.voidReturnType && m.returnType.unresolvedTemplateNames)$)$endif$;";
        return new ST(StCtx.BUILTINS, templateString)
                .add("m", this)
                .render(Locale.ROOT);
    }

    /**
     * Indicator if this is a static method.
     *
     * @return True if this is a static method, false if this is an instance
     * method.
     */
    @Override
    public boolean isStatic();

    /**
     * Test if the method is a const method.
     *
     * Const methods may not mutate their class instance.
     *
     * @return True if the method is a const method.
     */
    @Override
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
    @Override
    public Object getNoexcept();

    /**
     * Method {@link Visibility visibility}.
     *
     * @return The {@link Visibility visibility} of the method.
     */
    @Override
    public Visibility getVisibility();

    /**
     * The documentation of the function.
     *
     * @return Doxygen documentation of the method, or null.
     */
    @Override
    public String getDocString();

    /**
     * Get the inline declaration of this method.
     *
     * @return A string containing the inline declaration of this method. Or
     * null if this method isn't inlined.
     */
    @Override
    public default String getInline() {
        return "JSER_INLINE";
    }

    /**
     * Retrieve the the names of generics specification local to the function.
     *
     * @return Ordered list of names.
     */
    @Override
    public List<String> getFunctionGenericsNames();

    /**
     * Retrieve the defaults for the generics specification, local to the
     * function.
     *
     * @return Ordered list of defaults, by name.
     */
    @Override
    public List<String> getFunctionGenericsDefault();

    /**
     * Delegate is not virtual, therefore final has no effect.
     *
     * @return False.
     */
    @Override
    public default boolean isFinal() {
        return false;
    }

    /**
     * Delegate is not virtual, therefore override has no effect.
     *
     * @return False.
     */
    @Override
    public default boolean isOverride() {
        return false;
    }

    /**
     * Delegate is not virtual.
     *
     * @return False.
     */
    @Override
    public default boolean isVirtual() {
        return false;
    }

    /**
     * Delegate is not virtual, therefore can not be pure virtual.
     *
     * @return False.
     */
    @Override
    public default boolean isPureVirtual() {
        return false;
    }

    /**
     * Simple implementation of {@link ClassDelegateMethod}.
     */
    public static class Impl implements ClassDelegateMethod {
        private final ClassType model;
        private final String name;
        private final String delegateName;
        private final List<Type> argumentTypes;
        private final List<String> argumentNames;
        private final Type returnType;
        private final Includes includes;
        private final boolean staticVar;
        private final boolean constVar;
        private final Object noexcept;
        private final Visibility visibility;
        private final String docString;
        private final List<String> functionGenericsNames;
        private final List<String> functionGenericsDefault;

        public Impl(ClassType model, String name, String delegateName, List<Type> argumentTypes, List<String> argumentNames, Type returnType, Includes includes, boolean staticVar, boolean constVar, Object noexcept, Visibility visibility, String docString, List<String> functionGenericsNames, List<String> functionGenericsDefault) {
            this.model = requireNonNull(model);
            this.name = requireNonNull(name);
            this.delegateName = requireNonNull(delegateName);
            this.argumentTypes = requireNonNull(argumentTypes);
            this.argumentNames = requireNonNull(argumentNames);
            this.returnType = returnType;
            this.includes = requireNonNull(includes);
            this.staticVar = staticVar;
            this.constVar = constVar;
            this.noexcept = noexcept;
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
        public String getName() {
            return name;
        }

        @Override
        public String getDelegateName() {
            return delegateName;
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
            return functionGenericsNames;
        }

        @Override
        public List<String> getFunctionGenericsDefault() {
            return functionGenericsDefault;
        }
    }
}
