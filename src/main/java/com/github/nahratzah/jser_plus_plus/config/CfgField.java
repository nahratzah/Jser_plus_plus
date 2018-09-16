package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;

/**
 * Configuration for a field.
 *
 * @author ariane
 */
public class CfgField {
    public Boolean getGetterFn() {
        return getterFn;
    }

    public void setGetterFn(Boolean getterFn) {
        this.getterFn = getterFn;
    }

    public Boolean getSetterFn() {
        return setterFn;
    }

    public void setSetterFn(Boolean setterFn) {
        this.setterFn = setterFn;
    }

    public Visibility getVisibility() {
        return visibility;
    }

    public void setVisibility(Visibility visibility) {
        this.visibility = visibility;
    }

    public boolean isFinal() {
        return finalVar;
    }

    public void setFinal(boolean finalVar) {
        this.finalVar = finalVar;
    }

    @JsonProperty("getter")
    private Boolean getterFn;
    @JsonProperty("setter")
    private Boolean setterFn;
    @JsonProperty("visibility")
    private Visibility visibility;
    @JsonProperty("final")
    private boolean finalVar = false;
}
