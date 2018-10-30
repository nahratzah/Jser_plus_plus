package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.misc.AccessorMethodComparator;
import com.github.nahratzah.jser_plus_plus.model.impl.AccessorMethod;
import com.google.common.collect.Collections2;
import java.util.Collection;
import static java.util.Collections.unmodifiableCollection;
import java.util.List;
import static java.util.Objects.requireNonNull;
import java.util.TreeSet;
import java.util.function.Predicate;

/**
 * Models an accessor to a class.
 *
 * Accessors are used to access the fields and methods of a class, while
 * applying and removing type erasure.
 *
 * @author ariane
 */
public class Accessor {
    /**
     * Construct an accessor for the given {@link ClassType class}.
     *
     * @param classType The {@link ClassType class} that is accessed via this
     * accessor.
     */
    public Accessor(ClassType classType) {
        this.model = requireNonNull(classType);
    }

    /**
     * Add a method to the accessor.
     *
     * @param method The method to add.
     */
    public void add(AccessorMethod method) {
        accessorMethods.add(method);
    }

    /**
     * Retrieve the model of the accessor.
     *
     * @return Model of the accessor.
     */
    public ClassType getModel() {
        return model;
    }

    /**
     * Get friend types for this accessor.
     *
     * @return Friend types.
     */
    public List<Type> getFriends() {
        return model.getFriends();
    }

    /**
     * Get the documentation string or this accessor.
     *
     * @return Doxygen documentation string.
     */
    public String getDocString() {
        return model.getDocString();
    }

    /**
     * Retrieve all instance methods.
     *
     * @return Collection of instance methods.
     */
    public Collection<AccessorMethod> getInstanceMethods() {
        return Collections2.filter(unmodifiableCollection(accessorMethods), STATIC_METHOD_PREDICATE.negate()::test);
    }

    /**
     * Retrieve all static methods.
     *
     * @return Collection of static methods.
     */
    public Collection<AccessorMethod> getStaticMethods() {
        return Collections2.filter(unmodifiableCollection(accessorMethods), STATIC_METHOD_PREDICATE::test);
    }

    /**
     * If set, the model for this accessor is an interface.
     *
     * @return True if the model is an interface.
     */
    public boolean isInterface() {
        return model.isInterface();
    }

    /**
     * If set, the model for this accessor is an abstract class.
     *
     * @return True if the model is an abstract class.
     */
    public boolean isAbstract() {
        return model.isAbstract();
    }

    /**
     * If set, the model for this accessor is an enum.
     *
     * @return True if the model is an enum.
     */
    public boolean isEnum() {
        return model.isEnum();
    }

    /**
     * If set, the model for this type is in development mode and may not be
     * fully implemented or instantiable.
     *
     * @return True if the model is in development mode.
     */
    public boolean isDevMode() {
        return model.isDevMode();
    }

    /**
     * Predicate that tests for static methods.
     */
    private static final Predicate<AccessorMethod> STATIC_METHOD_PREDICATE = AccessorMethod::isStatic;
    /**
     * Class type for which this accessor is an accessor.
     */
    private final ClassType model;
    /**
     * All accessor methods.
     */
    private final Collection<AccessorMethod> accessorMethods = new TreeSet<>(new AccessorMethodComparator());
}
