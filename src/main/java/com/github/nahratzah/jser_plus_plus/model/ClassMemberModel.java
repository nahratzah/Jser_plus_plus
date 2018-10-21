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
    /**
     * Returns the declaring class of this class member.
     *
     * @return The declaring class of this class member.
     */
    public ClassType getDeclaringClass();

    /**
     * Retrieve the visibility of the class member.
     *
     * @return The visibility of the class member.
     */
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
     * Test if this class member is a static method.
     *
     * Constructors and destructors are not static.
     *
     * @return True if the class member is a static method.
     */
    public default boolean isStatic() {
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
     * Test if this is a constructor.
     *
     * @return True if this is a constructor.
     */
    public default boolean isConstructor() {
        return false;
    }

    /**
     * Retrieve the {@link OverrideSelector override selector} for this class
     * member.
     *
     * @param ctx Context to look up types.
     * @return An optional that contains the override selector, if the class
     * member is a method.
     */
    public default Optional<OverrideSelector> getOverrideSelector(Context ctx) {
        requireNonNull(ctx);
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

    public static class ClassMethod extends AbstractClassMemberModel implements MethodModel {
        public ClassMethod(Context ctx, ClassType model, Method method) {
            super(ctx, model, method.getReturnType(), method.getArguments(), method.getBody());

            this.method = requireNonNull(method);
        }

        @Override
        public String getName() {
            return method.getName();
        }

        @Override
        public BoundTemplate.ClassBinding<? extends ClassType> getArgumentDeclaringClass() {
            return cdef.getBoundType();
        }

        @Override
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
            return MethodModel.super.getDeclarationIncludes();
        }

        @Override
        public Stream<String> getImplementationIncludes() {
            return MethodModel.super.getImplementationIncludes();
        }

        @Override
        public boolean isPublicMethod() {
            return getVisibility() == Visibility.PUBLIC;
        }

        @Override
        public boolean isStatic() {
            return method.isStatic();
        }

        @Override
        public boolean isVirtual() {
            return method.isVirtual() || method.isOverride();
        }

        @Override
        public boolean isPureVirtual() {
            return isVirtual() && method.getBody() == null;
        }

        @Override
        public Visibility getVisibility() {
            return method.getVisibility();
        }

        @Override
        public boolean isOverride() {
            return method.isOverride();
        }

        @Override
        public boolean isConst() {
            return method.isConst();
        }

        @Override
        public boolean isFinal() {
            return method.isFinal();
        }

        @Override
        public boolean isHideOk() {
            return method.isHideOk();
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
        @Override
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

        @Override
        public boolean isCovariantReturn() {
            final Boolean covariantReturn = method.isCovariantReturn();
            if (covariantReturn != null) return isVirtual() && covariantReturn;
            return isVirtual();
        }

        @Override
        public String getDocString() {
            return method.getDocString();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
        }

        @Override
        public Optional<OverrideSelector> getOverrideSelector(Context ctx) {
            // Static methods don't have an override selector,
            // since they can not be overriden.
            if (isStatic()) return Optional.empty();

            return Optional.of(new OverrideSelector(ctx, this));
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
            this(ctx, cdef, constructor, true);
        }

        public ClassConstructor(Context ctx, ClassType cdef, Constructor constructor, boolean inheritable) {
            super(ctx, cdef, null, constructor.getArguments(), constructor.getBody());

            this.name = cdef.getClassName();
            this.inheritable = inheritable;
            this.constructor = requireNonNull(constructor);

            {
                final LinkedHashMap<String, String> initializersTmp = new LinkedHashMap<>();

                if (constructor.getSuperInitializer() != null) {
                    final String superName = StCtx.BUILTINS.getInstanceOf("boundTemplateType")
                            .add("t", cdef.getSuperClass())
                            .add("format", "style=erased")
                            .render(Locale.ROOT);
                    initializersTmp.put(superName, renderImpl(constructor.getSuperInitializer()));
                }

                cdef.getFields().stream()
                        .filter(field -> constructor.getInitializers().containsKey(field.getName()))
                        .forEachOrdered(field -> {
                            final String fieldName = field.getName();
                            final String initializer = constructor.getInitializers().get(fieldName);

                            String renderedInitializer = renderImpl(initializer);
                            if (field.isJavaClassVarType()) {
                                if (renderedInitializer.trim().isEmpty())
                                    renderedInitializer = "*this";
                                else
                                    renderedInitializer = "*this, " + renderedInitializer;
                            }

                            initializersTmp.put(fieldName, renderedInitializer);
                        });
                this.initializers = initializersTmp;
            }
        }

        public ClassConstructor(Context ctx, ClassType cdef, ClassConstructor delegate) {
            super(ctx, delegate.cdef, null, delegate.constructor.getArguments(), null);

            this.name = cdef.getClassName();
            this.inheritable = delegate.inheritable;
            this.constructor = requireNonNull(delegate.constructor);

            final String superName = StCtx.BUILTINS.getInstanceOf("boundTemplateType")
                    .add("t", cdef.getSuperClass())
                    .add("format", "style=erased")
                    .render(Locale.ROOT);
            final String superInitializer = this.getArgumentNames().stream()
                    .map(argName -> String.format("::std::forward<decltype(%1$s)>(%1$s)", argName))
                    .collect(Collectors.joining(", "));
            this.initializers = new LinkedHashMap<>(singletonMap(superName, superInitializer));
        }

        public String getName() {
            return name;
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
        public boolean isConstructor() {
            return true;
        }

        public boolean isInheritable() {
            return inheritable;
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
        private final boolean inheritable;
        private final String name;
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

    public static abstract class AbstractClassMemberModel implements ClassMemberModel {
        public AbstractClassMemberModel(Context ctx, ClassType cdef, CfgType returnType, List<CfgArgument> arguments, String body) {
            this.cdef = requireNonNull(cdef);
            this.variables = unmodifiableList(cdef.getTemplateArguments().stream()
                    .map(ClassTemplateArgument::getName)
                    .collect(Collectors.toList()));

            final BiFunction<String, Collection<Type>, String> basicRenderer = (text, collection) -> {
                final Map<String, BoundTemplate.VarBinding> variablesMap = variables.stream()
                        .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));

                return new ST(StCtx.contextGroup(ctx, variablesMap, cdef.getBoundType(), collection::add), text)
                        .add("model", cdef)
                        .render(Locale.ROOT, 78);
            };

            this.declRenderer = (text) -> basicRenderer.apply(text, this.declarationTypes);
            this.implRenderer = (text) -> basicRenderer.apply(text, this.implementationTypes);

            this.returnType = prerender(returnType, ctx);

            {
                final Map<String, BoundTemplate> squashMap = cdef.getErasedTemplateArguments().stream().collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
                final List<Type> argumentTypesTmp = new ArrayList<>();
                final List<String> argumentNamesTmp = new ArrayList<>();
                final List<Type> argumentTypesSquashedTmp = new ArrayList<>();
                arguments.forEach(arg -> {
                    argumentTypesTmp.add(prerender(arg.getType(), ctx));
                    argumentNamesTmp.add(arg.getName());
                    argumentTypesSquashedTmp.add(prerender(arg.getType(), ctx, squashMap));
                });
                this.argumentNames = unmodifiableList(argumentNamesTmp);
                this.argumentTypes = unmodifiableList(argumentTypesTmp);
                this.argumentTypesSquashed = unmodifiableList(argumentTypesSquashedTmp);
            }

            if (body == null) {
                this.body = null;
            } else {
                String renderedBody = renderImpl(body);
                // Remove newline at end, since the render engine will also generate one for us.
                // This gets rid of the blank line at the end of method bodies.
                if (renderedBody.endsWith("\n"))
                    renderedBody = renderedBody.substring(0, renderedBody.length() - 1);
                this.body = renderedBody;
            }
        }

        @Override
        public ClassType getDeclaringClass() {
            return cdef;
        }

        private Type prerender(CfgType cfgType, Context ctx) {
            if (cfgType == null) return null;
            return typeFromCfgType(cfgType, ctx, variables, cdef.getBoundType())
                    .prerender(ctx, singletonMap("model", cdef), variables);
        }

        private Type prerender(CfgType cfgType, Context ctx, Map<String, ? extends BoundTemplate> rebindMap) {
            if (cfgType == null) return null;
            return typeFromCfgType(cfgType, ctx, variables, cdef.getBoundType())
                    .prerender(ctx, singletonMap("model", cdef), rebindMap);
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

        public final List<Type> getArgumentTypesSquashed() {
            return argumentTypesSquashed;
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
        private final List<Type> argumentTypesSquashed;
        private final String body;
    }
}
