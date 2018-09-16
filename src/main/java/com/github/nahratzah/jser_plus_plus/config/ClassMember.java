package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonSubTypes;
import com.fasterxml.jackson.annotation.JsonTypeInfo;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;

/**
 * Describes a class member.
 *
 * @author ariane
 */
@JsonSubTypes({
    @JsonSubTypes.Type(name = "method", value = Method.class)
    , @JsonSubTypes.Type(name = "constructor", value = Constructor.class)
    , @JsonSubTypes.Type(name = "destructor", value = Destructor.class)})
@JsonTypeInfo(property = "type", use = JsonTypeInfo.Id.NAME)
public interface ClassMember {
    /**
     * Includes specification to make the class member function.
     *
     * @return Includes specification listing the headers required to make this
     * class member declarable and implementable.
     */
    public Includes getIncludes();

    /**
     * Retrieve the visibility of the class member.
     *
     * @return Visibility of the class member.
     */
    public Visibility getVisibility();

    /**
     * Visit the class member implementations.
     *
     * @param <T> Return type of the visitor application.
     * @param visitor The visitor to apply.
     * @return Result of applying the visitor to the derived type.
     */
    public <T> T visit(Visitor<T> visitor);

    /**
     * Visitor for class members.
     *
     * @param <T> Return type of the visitor application.
     */
    public static interface Visitor<T> {
        public T apply(Method method);

        public T apply(Constructor constructor);

        public T apply(Destructor destructor);
    }
}
