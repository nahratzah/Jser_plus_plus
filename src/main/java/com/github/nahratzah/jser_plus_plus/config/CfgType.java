package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Configuration type.
 *
 * @author ariane
 */
public class CfgType {
    @JsonProperty("cxx")
    private String cxxType;
    @JsonProperty("java")
    private String javaType;
    @JsonProperty("includes")
    private Includes includes = new Includes();

    public CfgType() {
    }

    public CfgType(String cxxType, String javaType) {
        this.cxxType = cxxType;
        this.javaType = javaType;
    }

    public String getCxxType() {
        return cxxType;
    }

    public void setCxxType(String cxxType) {
        this.cxxType = cxxType;
    }

    public String getJavaType() {
        return javaType;
    }

    public void setJavaType(String javaType) {
        this.javaType = javaType;
    }

    public Includes getIncludes() {
        return includes;
    }

    public void setIncludes(Includes includes) {
        this.includes = includes;
    }
}
