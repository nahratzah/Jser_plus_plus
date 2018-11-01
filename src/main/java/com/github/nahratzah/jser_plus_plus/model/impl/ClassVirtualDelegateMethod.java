package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.ImplementedClassMethod;
import com.github.nahratzah.jser_plus_plus.model.MethodModel;
import com.github.nahratzah.jser_plus_plus.model.TemplateSelector;
import com.github.nahratzah.jser_plus_plus.model.Type;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Streams;
import java.util.Collections;
import static java.util.Collections.EMPTY_MAP;
import static java.util.Collections.singletonMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * A delegate method, used to implement a virtual function such that it forwards
 * to another virtual function.
 *
 * @author ariane
 */
public interface ClassVirtualDelegateMethod extends MethodModel {
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
                Stream.of("java/_maybe_cast.h"),
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
            return "void".equals(((CxxType) type).getPreRendered());
        return false;
    }

    /**
     * Test if the return type needs to be converted.
     *
     * @return True if the return type needs to be converted, false otherwise.
     */
    public boolean isReturnTypeCastRequired();

    /**
     * Accessor method body.
     *
     * @return Method body.
     */
    @Override
    public default String getBody() {
        final String templateString = "$if (m.returnType && !m.voidReturnType)$return $if (m.returnTypeCastRequired)$::java::cast<$boundTemplateType(m.returnType, \"style=type, class=return\")$>($endif$$endif$this->$m.delegateName$($[{$tagType(m.targetTagType)$()}, rest(m.argumentNames):{name | ::java::_maybe_cast(::std::forward<decltype($name$)>($name$))}]; anchor, wrap, separator = \", \"$)$if (m.returnType && !m.voidReturnType && m.returnTypeCastRequired)$)$endif$;";
        return new ST(StCtx.BUILTINS, templateString)
                .add("m", this)
                .render(Locale.ROOT);
    }

    /**
     * Type used for target invocation.
     *
     * @return The tag type for the target invocation.
     */
    public ClassType getTargetTagType();

    /**
     * Indicator if this is a static method.
     *
     * @return False.
     */
    @Override
    public default boolean isStatic() {
        return false;
    }

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
     * @return The {@link Visibility#PRIVATE private visibility} of the method.
     */
    @Override
    public default Visibility getVisibility() {
        return Visibility.PRIVATE;
    }

    /**
     * The documentation of the function.
     *
     * Because the documentation is already present on the function we are
     * overriding, we don't need to repeat it.
     *
     * @return Null, indicating there is no documentation string.
     */
    @Override
    public default String getDocString() {
        return null;
    }

    /**
     * Delegate method is not inline.
     *
     * @return null.
     */
    @Override
    public default String getInline() {
        return null;
    }

    /**
     * Retrieve the the names of generics specification local to the function.
     *
     * @return {@link Collections#EMPTY_LIST Empty list}.
     */
    @Override
    public default List<String> getFunctionGenericsNames() {
        return Collections.EMPTY_LIST;
    }

    /**
     * Retrieve the defaults for the generics specification, local to the
     * function.
     *
     * @return {@link Collections#EMPTY_LIST Empty list}.
     */
    @Override
    public default List<String> getFunctionGenericsDefault() {
        return Collections.EMPTY_LIST;
    }

    @Override
    public default Map<String, BoundTemplate> getMethodGenerics() {
        return EMPTY_MAP;
    }

    /**
     * Test if the method is final.
     *
     * @return False.
     */
    @Override
    public boolean isFinal();

    /**
     * Delegate overrides method.
     *
     * @return False.
     */
    @Override
    public default boolean isOverride() {
        return true;
    }

    /**
     * Delegate is virtual.
     *
     * @return False.
     */
    @Override
    public default boolean isVirtual() {
        return true;
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
     * Implementation for an overriden method that delegates to a target
     * implementation.
     */
    public static class Impl implements ClassVirtualDelegateMethod {
        private static final Logger LOG = Logger.getLogger(Impl.class.getName());
        private final ImplementedClassMethod target;
        private final ImplementedClassMethod overridenMethod;
        private final Includes includes;
        private final String overridenMethodName, targetMethodName;

        public Impl(ImplementedClassMethod target, ImplementedClassMethod overridenMethod, Includes includes, String targetMethodName, String overridenMethodName) {
            this.target = requireNonNull(target);
            this.overridenMethod = requireNonNull(overridenMethod);
            this.includes = requireNonNull(includes);
            this.targetMethodName = (targetMethodName == null ? this.target.getSelector().getName() : targetMethodName);
            this.overridenMethodName = (overridenMethodName == null ? this.overridenMethod.getSelector().getName() : overridenMethodName);
        }

        @Override
        public ClassType getModel() {
            return target.getCls();
        }

        @Override
        public String getName() {
            return overridenMethodName;
        }

        @Override
        public String getDelegateName() {
            return targetMethodName;
        }

        @Override
        public Type getReturnType() {
            return overridenMethod.getErasedSelector().getReturnType();
        }

        @Override
        public List<Type> getArgumentTypes() {
            return Stream.<Type>concat(
                    Stream.of(overridenMethod.getTagType())
                            .filter(Objects::nonNull)
                            .map(cls -> new CxxType(Context.UNIMPLEMENTED_CONTEXT, "$tagType(x)$", new Includes(), singletonMap("x", cls), EMPTY_MAP, null)),
                    overridenMethod.getErasedSelector().getArguments().stream())
                    .collect(Collectors.toList());
        }

        @Override
        public List<String> getArgumentNames() {
            return Stream.concat(
                    Stream.of(overridenMethod.getTagType())
                            .filter(Objects::nonNull)
                            .map(ignored -> "_tag_"),
                    overridenMethod.getErasedSelector().getUnderlyingMethod().getArgumentNames().stream())
                    .collect(Collectors.toList());
        }

        @Override
        public Includes getIncludes() {
            return includes;
        }

        @Override
        public boolean isReturnTypeCastRequired() {
            // If the generics return types don't match, the return type should be a specialization (co-variant return).
            // In this case, no cast is needed.
            // However, if the generics return types match, but the erased types don't, we're cross casting.
            // This occurs if any of the template argument in the return type has been changed.
            return Objects.equals(target.getSelector().getReturnType(), overridenMethod.getSelector().getReturnType())
                    && !Objects.equals(target.getErasedSelector().getReturnType(), overridenMethod.getErasedSelector().getReturnType());
        }

        @Override
        public ClassType getTargetTagType() {
            return target.getTagType();
        }

        @Override
        public boolean isConst() {
            return overridenMethod.getSelector().isConst();
        }

        @Override
        public Object getNoexcept() {
            return overridenMethod.getSelector().getUnderlyingMethod().getNoexcept();
        }

        @Override
        public boolean isFinal() {
            return target.getSelector().getUnderlyingMethod().isFinal();
        }
    }
}
