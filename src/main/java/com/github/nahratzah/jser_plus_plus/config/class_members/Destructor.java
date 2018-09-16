package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;

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
    @JsonProperty("visibility")
    private Visibility visibility = Visibility.PUBLIC;

    @Override
    public <T> T visit(Visitor<T> visitor) {
        return visitor.apply(this);
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

    @Override
    public Visibility getVisibility() {
        return visibility;
    }

    public void setVisibility(Visibility visibility) {
        this.visibility = visibility;
    }

}
