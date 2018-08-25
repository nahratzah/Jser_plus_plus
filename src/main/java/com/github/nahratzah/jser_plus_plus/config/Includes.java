package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.ArrayList;
import java.util.List;

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
    private List<String> declarationIncludes = new ArrayList<>();
    /**
     * Includes required for implementation.
     */
    @JsonProperty("implementation")
    private List<String> implementationIncludes = new ArrayList<>();

    public List<String> getDeclarationIncludes() {
        return declarationIncludes;
    }

    public void setDeclarationIncludes(List<String> declarationIncludes) {
        this.declarationIncludes = declarationIncludes;
    }

    public List<String> getImplementationIncludes() {
        return implementationIncludes;
    }

    public void setImplementationIncludes(List<String> implementationIncludes) {
        this.implementationIncludes = implementationIncludes;
    }
}
