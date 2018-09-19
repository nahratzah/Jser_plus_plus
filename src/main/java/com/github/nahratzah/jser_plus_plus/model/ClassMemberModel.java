package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import static com.github.nahratzah.jser_plus_plus.model.Type.typeFromCfgType;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.ArrayList;
import static java.util.Collections.unmodifiableList;
import java.util.List;
import static java.util.Objects.requireNonNull;
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

    public static class ClassMethod implements ClassMemberModel {
        public ClassMethod(Context ctx, ClassType cdef, Method method) {
            final List<String> variables = unmodifiableList(cdef.getTemplateArguments().stream()
                    .map(ClassTemplateArgument::getName)
                    .collect(Collectors.toList()));

            this.cdef = requireNonNull(cdef);
            this.method = requireNonNull(method);
            this.returnType = typeFromCfgType(method.getReturnType(), ctx, variables);

            {
                final List<Type> argumentTypesTmp = new ArrayList<>();
                final List<String> argumentNamesTmp = new ArrayList<>();
                method.getArguments()
                        .forEach(arg -> {
                            argumentTypesTmp.add(typeFromCfgType(arg.getType(), ctx, variables));
                            argumentNamesTmp.add(arg.getName());
                        });
                this.argumentNames = unmodifiableList(argumentNamesTmp);
                this.argumentTypes = unmodifiableList(argumentTypesTmp);
            }
        }

        public String getName() {
            return method.getName();
        }

        public Type getReturnType() {
            return returnType;
        }

        public List<Type> getArgumentTypes() {
            return argumentTypes;
        }

        public List<String> getArgumentNames() {
            return argumentNames;
        }

        public Includes getIncludes() {
            return method.getIncludes();
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return Stream.concat(Stream.of(getReturnType()), getArgumentTypes().stream());
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            // XXX
            return getDeclarationTypes();
        }

        public ST getBody() {
            return new ST(StCtx.BUILTINS, method.getBody())
                    .add("cdef", cdef)
                    .add("method", this);
        }

        public boolean isVirtual() {
            return method.isVirtual() || method.getBody() == null;
        }

        @Override
        public Visibility getVisibility() {
            return method.getVisibility();
        }

        public String getDocString() {
            return method.getDocString();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
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
        private final ClassType cdef;
        private final Type returnType;
        private final List<String> argumentNames;
        private final List<Type> argumentTypes;
    }

    public static class ClassConstructor implements ClassMemberModel {
        public ClassConstructor(Constructor constructor) {
            this.constructor = requireNonNull(constructor);
        }

        public List<String> getArguments() {
            return constructor.getArguments();
        }

        public Includes getIncludes() {
            return constructor.getIncludes();
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return Stream.empty();
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            return Stream.empty();
        }

        public String getBody() {
            return constructor.getBody();
        }

        @Override
        public Visibility getVisibility() {
            return constructor.getVisibility();
        }

        @Override
        public <T> T visit(Visitor<T> visitor) {
            return visitor.apply(this);
        }

        @Override
        public String toString() {
            return "method: "
                    + getVisibility()
                    + "constructor "
                    + getArguments().stream().map(Object::toString).collect(Collectors.joining(", ", "(", ")"));
        }

        private final Constructor constructor;
    }

    public static class ClassDestructor implements ClassMemberModel {
        public ClassDestructor(Destructor destructor) {
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

        public String getBody() {
            return destructor.getBody();
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
                    + "destructor";
        }

        private final Destructor destructor;
    }
}
