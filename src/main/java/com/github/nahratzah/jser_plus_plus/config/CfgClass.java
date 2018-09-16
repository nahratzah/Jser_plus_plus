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
    public Map<String, CfgField> getFields() {
        return fields;
    }

    public void setFields(Map<String, CfgField> fields) {
        this.fields = fields;
    }

    @JsonProperty("fields")
    private Map<String, CfgField> fields = new HashMap<>();
}
