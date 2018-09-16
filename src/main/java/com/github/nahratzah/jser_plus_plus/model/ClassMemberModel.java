package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import java.util.List;
import static java.util.Objects.requireNonNull;
import java.util.stream.Collectors;

/**
 * Model of a class member.
 *
 * @author ariane
 */
public interface ClassMemberModel {
    public Visibility getVisibility();

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
        public ClassMethod(Method method) {
            this.method = requireNonNull(method);
        }

        public String getName() {
            return method.getName();
        }

        public String getReturnType() {
            return method.getReturnType();
        }

        public List<String> getArguments() {
            return method.getArguments();
        }

        public Includes getIncludes() {
            return method.getIncludes();
        }

        public String getBody() {
            return method.getBody();
        }

        public boolean isVirtual() {
            return method.isVirtual() || method.getBody() == null;
        }

        @Override
        public Visibility getVisibility() {
            return method.getVisibility();
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
                    + getArguments().stream().map(Object::toString).collect(Collectors.joining(", ", "(", ")"))
                    + " -> "
                    + getReturnType()
                    + (isVirtual() ? " = 0" : "");
        }

        private final Method method;
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
