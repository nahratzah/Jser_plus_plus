package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.HashMap;
import java.util.Map;

/**
 * Models the configuration of a particular class.
 *
 * @author ariane
 */
public class CfgClass {
    /**
     * If present, overrides the 'final' attribute of the class.
     *
     * @return True if the class is final, false if the class is not final, null
     * if the default algorithm applies.
     */
    public Boolean isFinal() {
        return finalVar;
    }

    /**
     * Change the final attribute of the class.
     *
     * @param finalVar True if the class is final, false if the class is not
     * final, null if the default algorithm applies.
     */
    public void setFinal(Boolean finalVar) {
        this.finalVar = finalVar;
    }

    public Map<String, CfgField> getFields() {
        return fields;
    }

    public void setFields(Map<String, CfgField> fields) {
        this.fields = fields;
    }

    @JsonProperty("final")
    private Boolean finalVar;
    @JsonProperty("fields")
    private Map<String, CfgField> fields = new HashMap<>();
}
