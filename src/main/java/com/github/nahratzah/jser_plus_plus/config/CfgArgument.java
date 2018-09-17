package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Models an argument to a function.
 *
 * @author ariane
 */
public class CfgArgument {
    @JsonProperty(value = "name", required = true)
    private String name;
    @JsonProperty(value = "type", required = true)
    private CfgType type;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public CfgType getType() {
        return type;
    }

    public void setType(CfgType type) {
        this.type = type;
    }
}
