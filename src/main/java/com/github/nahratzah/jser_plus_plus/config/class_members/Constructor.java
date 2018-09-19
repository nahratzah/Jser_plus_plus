package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * A destructor method.
 *
 * @author ariane
 */
public class Constructor implements ClassMember {
    @JsonProperty("arguments")
    private List<CfgArgument> arguments = new ArrayList<>();
    @JsonProperty("includes")
    private Includes includes = new Includes();
    @JsonProperty("body")
    private String body;
    @JsonProperty("super")
    private String superInitializer;
    @JsonProperty("initializers")
    private Map<String, String> initializers = new HashMap<>();
    @JsonProperty("visibility")
    private Visibility visibility = Visibility.PUBLIC;

    @Override
    public <T> T visit(Visitor<T> visitor) {
        return visitor.apply(this);
    }

    /**
     * Retrieve the arguments of the method.
     *
     * @return A list of arguments.
     */
    public List<CfgArgument> getArguments() {
        return arguments;
    }

    public void setArguments(List<CfgArgument> arguments) {
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

    /**
     * Retrieve the initializer for the super type.
     *
     * @return Initializer for the super type. May be null.
     */
    public String getSuperInitializer() {
        return superInitializer;
    }

    public void setSuperInitializer(String superInitializer) {
        this.superInitializer = superInitializer;
    }

    /**
     * Constructor initializers.
     *
     * @return Map of field to value template of initializers.
     */
    public Map<String, String> getInitializers() {
        return initializers;
    }

    public void setInitializers(Map<String, String> initializers) {
        this.initializers = initializers;
    }

    @Override
    public Visibility getVisibility() {
        return visibility;
    }

    public void setVisibility(Visibility visibility) {
        this.visibility = visibility;
    }
}
