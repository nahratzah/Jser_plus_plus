package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import java.util.ArrayList;
import java.util.List;

/**
 * A destructor method.
 *
 * @author ariane
 */
public class Constructor implements ClassMember {
    @JsonProperty("arguments")
    private List<String> arguments = new ArrayList<>();
    @JsonProperty("includes")
    private Includes includes = new Includes();
    @JsonProperty("body")
    private String body;

    /**
     * Retrieve the templated arguments of the method.
     *
     * @return A list of strings (one for each argument) encoding of the ST4
     * template.
     */
    public List<String> getArguments() {
        return arguments;
    }

    public void setArguments(List<String> arguments) {
        this.arguments = arguments;
    }

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
