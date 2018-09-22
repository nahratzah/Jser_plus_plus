package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import static com.github.nahratzah.jser_plus_plus.model.Type.typeFromCfgType;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.ArrayList;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Model of a class member.
 *
 * @author ariane
 */
public interface ClassMemberModel {
    public Visibility getVisibility();

    /**
     * Get all java types that are required to declare the class member.
     *
     * @return All java types for the class member declaration.
     */
    public Stream<Type> getDeclarationTypes();

    /**
     * Get all java types that are required to implement the class member.
     *
     * @return All java types for the class member implementation.
     */
    public Stream<Type> getImplementationTypes();

    /**
     * Get includes required to make the declaration of the class-member work.
     *
     * @return Includes.
     */
    public Stream<String> getDeclarationIncludes();

    /**
     * Get includes required to make the implementation of the class-member
     * work.
     *
     * @return
     */
    public Stream<String> getImplementationIncludes();

    /**
     * Test if this class member is a public method.
     *
     * @return True if the class member is a public method.
     */
    public default boolean isPublicMethod() {
        return false;
    }

    /**
     * Test if this is a class destructor.
     *
     * @return Class destructors are always implemented, even if they are pure
     * virtual.
     */
    public default boolean isDestructorMethod() {
        return this instanceof ClassDestructor;
    }

    /**
     * Test if this is the default constructor.
     *
     * @return True if this is the default constructor.
     */
    public default boolean isDefaultConstructor() {
        return false;
    }

    /**
     * Retrieve the {@link OverrideSelector override selector} for this class
     * member.
     *
     * @return An optional that contains the override selector, if the class
     * member is a method.
     */
    public default Optional<OverrideSelector> getOverrideSelector() {
        return Optional.empty();
    }

    public <T> T visit(Visitor<T> visitor);

    /**
     * Class member visitor.
     *
     * @param <T> Return type of invocations to the visit function.
     */
    public static interface Visitor<T> {
        public T apply(ClassMethod method);

        public T apply(ClassConstructor constructor);

        public T apply(ClassDestructor destructor);
    }

    public static class ClassMethod extends AbstractClassMemberModel {
        public ClassMethod(Context ctx, ClassType model, Method method) {
            super(ctx, model, method.getReturnType(), method.getArguments(), method.getBody());

            this.method = requireNonNull(method);
        }

        public String getName() {
            return method.getName();
        }

        public Includes getIncludes() {
            return method.getIncludes();
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return Stream.of(
                    Stream.of(getReturnType()),
                    getArgumentTypes().stream(),
                    super.getDeclarationTypes())
                    .flatMap(Function.identity());
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            return Stream.concat(
                    getDeclarationTypes(),
                    super.getImplementationTypes());
        }

        @Override
        public Stream<String> getDeclarationIncludes() {
            return Stream.of(
                    getIncludes().getDeclarationIncludes().stream(),
                    getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)),
                    getReturnType().getIncludes(true))
                    .flatMap(Function.identity());
        }

        @Override
        public Stream<String> getImplementationIncludes() {
            return Stream.of(
                    getIncludes().getImplementationIncludes().stream(),
                    getArgumentTypes().stream().flatMap(at -> at.getIncludes(false)),
                    getReturnType().getIncludes(false))
                    .flatMap(Function.identity());
        }

        @Override
        public boolean isPublicMethod() {
            return getVisibility() == Visibility.PUBLIC;
        }

        public boolean isVirtual() {
            return method.isVirtual() || method.isOverride();
        }

        public boolean isPureVirtual() {
            return isVirtual() && method.getBody() == null;
        }

        @Override
        public Visibility getVisibility() {
            return method.getVisibility();
        }

        public boolean isOverride() {
            return method.isOverride();
        }

        public boolean isConst() {
            return method.isConst();
        }

        public boolean isFinal() {
            return method.isFinal();
        }

        /**
         * Returns the noexcept specification.
         *
         * May return null.
         *
         * @return A {@link String string} or {@link ST template} with the
         * noexcept specification.
         * @throws IllegalStateException If the underlying method has a noexcept
         * specification that is neither a string nor a boolean.
         */
        public Object getNoexcept() {
            if (method.getNoexcept() == null) {
                return null;
            } else if (method.getNoexcept() instanceof String) {
                return new ST("noexcept($noexcept; anchor$)", '$', '$')
                        .add("noexcept", method.getNoexcept());
            } else if (method.getNoexcept() instanceof Boolean) {
                return ((Boolean) method.getNoexcept() ? "noexcept" : null);
            } else {
                throw new IllegalStateException("noexcept is required to be null, a string, or a boolean.");
            }
        }

        public String getDocString() {
            return method.getDocString();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
        }

        @Override
        public Optional<OverrideSelector> getOverrideSelector() {
            return Optional.of(new OverrideSelector(this));
        }

        @Override
        public String toString() {
            return "method: "
                    + getVisibility()
                    + " auto "
                    + getName()
                    + getArgumentNames().stream().map(Object::toString).collect(Collectors.joining(", ", "(", ")"))
                    + " -> "
                    + getReturnType()
                    + (isVirtual() ? " = 0" : "");
        }

        private final Method method;
    }

    public static class ClassConstructor extends AbstractClassMemberModel {
        public ClassConstructor(Context ctx, ClassType cdef, Constructor constructor) {
            super(ctx, cdef, null, constructor.getArguments(), constructor.getBody());

            this.constructor = requireNonNull(constructor);

            {
                final LinkedHashMap<String, String> initializersTmp = new LinkedHashMap<>();
                if (constructor.getSuperInitializer() != null)
                    initializersTmp.put(cdef.getSuperClass().getType().getClassName(), renderImpl(constructor.getSuperInitializer()));
                cdef.getFields().stream()
                        .filter(field -> constructor.getInitializers().containsKey(field.getName()))
                        .forEachOrdered(field -> {
                            final String name = field.getName();
                            final String initializer = constructor.getInitializers().get(name);
                            initializersTmp.put(name, renderImpl(initializer));
                        });
                this.initializers = initializersTmp;
            }
        }

        public String getName() {
            return cdef.getClassName();
        }

        public Includes getIncludes() {
            return constructor.getIncludes();
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return Stream.of(
                    getArgumentTypes().stream(),
                    super.getDeclarationTypes())
                    .flatMap(Function.identity());
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            return Stream.concat(
                    getDeclarationTypes(),
                    super.getImplementationTypes());
        }

        @Override
        public Stream<String> getDeclarationIncludes() {
            return Stream.of(
                    getIncludes().getDeclarationIncludes().stream(),
                    getArgumentTypes().stream().flatMap(at -> at.getIncludes(true)))
                    .flatMap(Function.identity());
        }

        @Override
        public Stream<String> getImplementationIncludes() {
            return Stream.of(
                    getIncludes().getImplementationIncludes().stream(),
                    getArgumentTypes().stream().flatMap(at -> at.getIncludes(false)))
                    .flatMap(Function.identity());
        }

        public Set<Map.Entry<String, String>> getInitializers() {
            return initializers.entrySet();
        }

        @Override
        public Visibility getVisibility() {
            return constructor.getVisibility();
        }

        @Override
        public boolean isDefaultConstructor() {
            return getArgumentTypes().isEmpty();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
        }

        @Override
        public String toString() {
            return "method: "
                    + getVisibility()
                    + " constructor "
                    + getArgumentNames().stream().map(Object::toString).collect(Collectors.joining(", ", "(", ")"));
        }

        private final Constructor constructor;
        private final LinkedHashMap<String, String> initializers;
    }

    public static class ClassDestructor extends AbstractClassMemberModel {
        public ClassDestructor(Context ctx, ClassType cdef, Destructor destructor) {
            super(ctx, cdef, null, EMPTY_LIST, destructor.getBody());
            this.destructor = requireNonNull(destructor);
        }

        public Includes getIncludes() {
            return destructor.getIncludes();
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return Stream.empty();
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            return Stream.empty();
        }

        @Override
        public Stream<String> getDeclarationIncludes() {
            return getIncludes().getDeclarationIncludes().stream();
        }

        @Override
        public Stream<String> getImplementationIncludes() {
            return getIncludes().getImplementationIncludes().stream();
        }

        public boolean isVirtual() {
            return true;
        }

        public boolean isPureVirtual() {
            return cdef.isAbstract() || cdef.isInterface();
        }

        public boolean isOverride() {
            return true;
        }

        @Override
        public Visibility getVisibility() {
            return destructor.getVisibility();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
        }

        @Override
        public String toString() {
            return "method: "
                    + getVisibility()
                    + " destructor";
        }

        private final Destructor destructor;
    }

    /**
     * The significant portion of a method for override comparisons.
     */
    public static class OverrideSelector {
        public OverrideSelector(ClassMethod method) {
            this(
                    method.cdef,
                    method.getName(),
                    method.isConst(),
                    method.getArgumentTypes());
        }

        public OverrideSelector(ClassType declaringClass, String name, boolean constVar, List<Type> arguments) {
            this(declaringClass, name, constVar, arguments, declaringClass.getTemplateArgumentNames());
        }

        public OverrideSelector(ClassType declaringClass, String name, boolean constVar, List<Type> arguments, Collection<String> variables) {
            this(declaringClass, name, constVar, arguments, makeVariablesMap_(variables));
        }

        public OverrideSelector(ClassType declaringClass, String name, boolean constVar, List<Type> arguments, Map<String, ? extends BoundTemplate> variablesMap) {
            this.declaringClass = declaringClass;
            this.name = requireNonNull(name);
            this.constVar = requireNonNull(constVar);
            this.variablesMap = requireNonNull(variablesMap);
            this.arguments = requireNonNull(arguments);
        }

        public String getName() {
            return name;
        }

        public boolean isConst() {
            return constVar;
        }

        public List<Type> getArguments() {
            return unmodifiableList(arguments);
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 97 * hash + Objects.hashCode(this.name);
            hash = 97 * hash + (this.constVar ? 1 : 0);
            hash = 97 * hash + Objects.hashCode(this.arguments);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final OverrideSelector other = (OverrideSelector) obj;
            if (this.constVar != other.constVar) return false;
            if (!Objects.equals(this.name, other.name)) return false;
            if (!Objects.equals(this.arguments, other.arguments)) return false;
            return true;
        }

        @Override
        public String toString() {
            return "auto "
                    + name
                    + arguments.stream()
                            .map(Object::toString)
                            .collect(Collectors.joining(", ", "(", ")"))
                    + (constVar ? " const" : "");
        }

        private static Map<String, BoundTemplate.VarBinding> makeVariablesMap_(Collection<String> variables) {
            return variables.stream()
                    .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));
        }

        private final String name;
        private final boolean constVar;
        private final List<Type> arguments;
        private final transient Map<String, ? extends BoundTemplate> variablesMap;
        private final transient ClassType declaringClass;
    }
}

abstract class AbstractClassMemberModel implements ClassMemberModel {
    public AbstractClassMemberModel(Context ctx, ClassType cdef, CfgType returnType, List<CfgArgument> arguments, String body) {
        this.cdef = requireNonNull(cdef);
        this.variables = unmodifiableList(cdef.getTemplateArguments().stream()
                .map(ClassTemplateArgument::getName)
                .collect(Collectors.toList()));

        final BiFunction<String, Collection<Type>, String> basicRenderer = (text, collection) -> {
            final Map<String, BoundTemplate.VarBinding> variablesMap = variables.stream()
                    .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));

            return new ST(StCtx.contextGroup(ctx, variablesMap, collection::add), text)
                    .add("model", cdef)
                    .render(Locale.ROOT);
        };

        this.declRenderer = (text) -> basicRenderer.apply(text, this.declarationTypes);
        this.implRenderer = (text) -> basicRenderer.apply(text, this.implementationTypes);

        this.returnType = prerender(returnType, ctx);

        {
            final List<Type> argumentTypesTmp = new ArrayList<>();
            final List<String> argumentNamesTmp = new ArrayList<>();
            arguments.forEach(arg -> {
                argumentTypesTmp.add(prerender(arg.getType(), ctx));
                argumentNamesTmp.add(arg.getName());
            });
            this.argumentNames = unmodifiableList(argumentNamesTmp);
            this.argumentTypes = unmodifiableList(argumentTypesTmp);
        }

        if (body == null)
            this.body = null;
        else
            this.body = renderImpl(body);
    }

    /**
     * Returns the declaring class of this class member.
     *
     * @return The declaring class of this class member.
     */
    public ClassType getDeclaringClass() {
        return cdef;
    }

    private Type prerender(CfgType cfgType, Context ctx) {
        if (cfgType == null) return null;
        return typeFromCfgType(cfgType, ctx, variables)
                .prerender(ctx, singletonMap("model", cdef), variables);
    }

    protected final String renderDecl(String text) {
        return declRenderer.apply(text);
    }

    protected final String renderImpl(String text) {
        return implRenderer.apply(text);
    }

    @Override
    public Stream<Type> getDeclarationTypes() {
        return declarationTypes.stream();
    }

    @Override
    public Stream<Type> getImplementationTypes() {
        return implementationTypes.stream();
    }

    public final Type getReturnType() {
        return returnType;
    }

    public final List<Type> getArgumentTypes() {
        return argumentTypes;
    }

    public final List<String> getArgumentNames() {
        return argumentNames;
    }

    public final String getBody() {
        return body;
    }

    protected final ClassType cdef;
    protected final List<String> variables;
    private final List<Type> declarationTypes = new ArrayList<>();
    private final List<Type> implementationTypes = new ArrayList<>();
    private final Function<String, String> declRenderer;
    private final Function<String, String> implRenderer;
    private final Type returnType;
    private final List<String> argumentNames;
    private final List<Type> argumentTypes;
    private final String body;
}
