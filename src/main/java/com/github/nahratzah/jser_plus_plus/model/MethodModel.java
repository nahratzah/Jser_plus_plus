package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
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
    public default Optional<ClassMemberModel.OverrideSelector> getOverrideSelector(Context ctx) {
        if (isStatic()) return Optional.empty();
        return Optional.of(new ClassMemberModel.OverrideSelector(ctx, this));
    }

    /**
     * Very simple implementation of {@link MethodModel}.
     */
    public static class SimpleMethodModel implements MethodModel {
        private final ClassType declaringClass;
        private final String name;
        private final Includes includes;
        private final Set<Type> declarationTypes;
        private final Set<Type> implementationTypes;
        private final Type returnType;
        private final List<Type> argumentTypes;
        private final List<String> argumentNames;
        private final String body;
        private final boolean staticVar;
        private final boolean virtualVar;
        private final boolean pureVirtualVar;
        private final boolean overrideVar;
        private final boolean constVar;
        private final boolean finalVar;
        private final Object noexcept;
        private final Visibility visibility;
        private final String docString;

        public SimpleMethodModel(ClassType declaringClass, String name, Includes includes, Set<Type> declarationTypes, Set<Type> implementationTypes, Type returnType, List<Type> argumentTypes, List<String> argumentNames, String body, boolean staticVar, boolean virtualVar, boolean pureVirtualVar, boolean overrideVar, boolean constVar, boolean finalVar, Object noexcept, Visibility visibility, String docString) {
            this.declaringClass = declaringClass;
            this.name = name;
            this.includes = includes;
            this.declarationTypes = declarationTypes;
            this.implementationTypes = implementationTypes;
            this.returnType = returnType;
            this.argumentTypes = argumentTypes;
            this.argumentNames = argumentNames;
            this.body = body;
            this.staticVar = staticVar;
            this.virtualVar = virtualVar;
            this.pureVirtualVar = pureVirtualVar;
            this.overrideVar = overrideVar;
            this.constVar = constVar;
            this.finalVar = finalVar;
            this.noexcept = noexcept;
            this.visibility = visibility;
            this.docString = docString;
        }

        @Override
        public ClassType getDeclaringClass() {
            return declaringClass;
        }

        @Override
        public String getName() {
            return name;
        }

        @Override
        public Includes getIncludes() {
            return includes;
        }

        @Override
        public Stream<Type> getDeclarationTypes() {
            return declarationTypes.stream();
        }

        @Override
        public Stream<Type> getImplementationTypes() {
            return implementationTypes.stream();
        }

        @Override
        public Type getReturnType() {
            return returnType;
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
        public String getBody() {
            return body;
        }

        @Override
        public boolean isStatic() {
            return staticVar;
        }

        @Override
        public boolean isVirtual() {
            return virtualVar;
        }

        @Override
        public boolean isPureVirtual() {
            return pureVirtualVar;
        }

        @Override
        public boolean isOverride() {
            return overrideVar;
        }

        @Override
        public boolean isConst() {
            return constVar;
        }

        @Override
        public boolean isFinal() {
            return finalVar;
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
        public int hashCode() {
            int hash = 7;
            hash = 67 * hash + Objects.hashCode(this.declaringClass);
            hash = 67 * hash + Objects.hashCode(this.name);
            hash = 67 * hash + Objects.hashCode(this.includes);
            hash = 67 * hash + Objects.hashCode(this.returnType);
            hash = 67 * hash + Objects.hashCode(this.argumentTypes);
            hash = 67 * hash + Objects.hashCode(this.argumentNames);
            hash = 67 * hash + Objects.hashCode(this.body);
            hash = 67 * hash + (this.staticVar ? 1 : 0);
            hash = 67 * hash + (this.virtualVar ? 1 : 0);
            hash = 67 * hash + (this.pureVirtualVar ? 1 : 0);
            hash = 67 * hash + (this.overrideVar ? 1 : 0);
            hash = 67 * hash + (this.constVar ? 1 : 0);
            hash = 67 * hash + (this.finalVar ? 1 : 0);
            hash = 67 * hash + Objects.hashCode(this.noexcept);
            hash = 67 * hash + Objects.hashCode(this.visibility);
            hash = 67 * hash + Objects.hashCode(this.docString);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final SimpleMethodModel other = (SimpleMethodModel) obj;
            if (this.staticVar != other.staticVar) return false;
            if (this.virtualVar != other.virtualVar) return false;
            if (this.pureVirtualVar != other.pureVirtualVar) return false;
            if (this.overrideVar != other.overrideVar) return false;
            if (this.constVar != other.constVar) return false;
            if (this.finalVar != other.finalVar) return false;
            if (!Objects.equals(this.name, other.name)) return false;
            if (!Objects.equals(this.body, other.body)) return false;
            if (!Objects.equals(this.docString, other.docString)) return false;
            if (!Objects.equals(this.declaringClass, other.declaringClass))
                return false;
            if (!Objects.equals(this.includes, other.includes)) return false;
            if (!Objects.equals(this.declarationTypes, other.declarationTypes))
                return false;
            if (!Objects.equals(this.implementationTypes, other.implementationTypes))
                return false;
            if (!Objects.equals(this.returnType, other.returnType))
                return false;
            if (!Objects.equals(this.argumentTypes, other.argumentTypes))
                return false;
            if (!Objects.equals(this.argumentNames, other.argumentNames))
                return false;
            if (!Objects.equals(this.noexcept, other.noexcept)) return false;
            if (this.visibility != other.visibility) return false;
            return true;
        }

        @Override
        public String toString() {
            return "SimpleMethodModel{" + "declaringClass=" + declaringClass + ", name=" + name + ", includes=" + includes + ", declarationTypes=" + declarationTypes + ", implementationTypes=" + implementationTypes + ", returnType=" + returnType + ", argumentTypes=" + argumentTypes + ", argumentNames=" + argumentNames + ", body=" + body + ", staticVar=" + staticVar + ", virtualVar=" + virtualVar + ", pureVirtualVar=" + pureVirtualVar + ", overrideVar=" + overrideVar + ", constVar=" + constVar + ", finalVar=" + finalVar + ", noexcept=" + noexcept + ", visibility=" + visibility + ", docString=" + docString + '}';
        }
    }
}
