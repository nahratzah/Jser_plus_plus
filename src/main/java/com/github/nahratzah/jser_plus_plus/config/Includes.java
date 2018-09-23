package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.Collection;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

/**
 * Include specification.
 *
 * @author ariane
 */
public class Includes {
    /**
     * Includes required for declaration.
     */
    @JsonProperty("declaration")
    private Set<String> declarationIncludes = new HashSet<>();
    /**
     * Includes required for implementation.
     */
    @JsonProperty("implementation")
    private Set<String> implementationIncludes = new HashSet<>();

    public Includes() {
    }

    public Includes(Collection<String> declarationIncludes, Collection<String> implementationIncludes) {
        this.declarationIncludes.addAll(declarationIncludes);
        this.implementationIncludes.addAll(implementationIncludes);
    }

    public Set<String> getDeclarationIncludes() {
        return declarationIncludes;
    }

    public void setDeclarationIncludes(Set<String> declarationIncludes) {
        this.declarationIncludes = declarationIncludes;
    }

    public Set<String> getImplementationIncludes() {
        return implementationIncludes;
    }

    public void setImplementationIncludes(Set<String> implementationIncludes) {
        this.implementationIncludes = implementationIncludes;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 79 * hash + Objects.hashCode(this.declarationIncludes);
        hash = 79 * hash + Objects.hashCode(this.implementationIncludes);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final Includes other = (Includes) obj;
        if (!Objects.equals(this.declarationIncludes, other.declarationIncludes))
            return false;
        if (!Objects.equals(this.implementationIncludes, other.implementationIncludes))
            return false;
        return true;
    }
}
