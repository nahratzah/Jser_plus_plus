package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;

/**
 * A destructor method.
 *
 * @author ariane
 */
public class Destructor implements ClassMember {
    @JsonProperty("includes")
    private Includes includes = new Includes();
    @JsonProperty("body")
    private String body;

    @Override
    public Includes getIncludes() {
        return includes;
    }

    public void setIncludes(Includes includes) {
        this.includes = includes;
    }

    /**
     * Templated body of the method.
     *
     * @return The string representation of the ST4 template, describing the
     * body of the method.
     */
    public String getBody() {
        return body;
    }

    public void setBody(String body) {
        this.body = body;
    }
}
