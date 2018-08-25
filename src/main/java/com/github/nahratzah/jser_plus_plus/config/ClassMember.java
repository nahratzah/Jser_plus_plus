package com.github.nahratzah.jser_plus_plus.config;

/**
 * Describes a class member.
 *
 * @author ariane
 */
public interface ClassMember {
    /**
     * Includes specification to make the class member function.
     *
     * @return Includes specification listing the headers required to make this
     * class member declarable and implementable.
     */
    public Includes getIncludes();
}
