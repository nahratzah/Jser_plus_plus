package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import java.util.ArrayList;
import java.util.List;

/**
 * Describe a class method.
 *
 * @author ariane
 */
public class Method implements ClassMember {
    @JsonProperty(value = "name", required = true)
    private String name;
    @JsonProperty("return")
    private String returnType = "void";
    @JsonProperty("arguments")
    private List<String> arguments = new ArrayList<>();
    @JsonProperty("includes")
    private Includes includes = new Includes();
    @JsonProperty("body")
    private String body;

    /**
     * Name of the method.
     *
     * @return String representing the name of the method.
     */
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    /**
     * Retrieve the templated return type of the method.
     *
     * @return A string encoding of the ST4 template.
     */
    public String getReturnType() {
        return returnType;
    }

    public void setReturnType(String returnType) {
        this.returnType = returnType;
    }

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
