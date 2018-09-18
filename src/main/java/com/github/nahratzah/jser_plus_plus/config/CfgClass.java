package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
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

    /**
     * Retrieve all direct configured members.
     *
     * @return List of members declared for this config.
     */
    public List<ClassMember> getMembers() {
        return members;
    }

    public void setMembers(List<ClassMember> members) {
        this.members = members;
    }

    /**
     * Retrieve the variable type for this class.
     *
     * The variable type is the type used for variables and fields, instead of
     * the class.
     *
     * @return The variable type of the class.
     */
    public CfgType getVarType() {
        return varType;
    }

    public void setVarType(CfgType varType) {
        this.varType = varType;
    }

    @JsonProperty("final")
    private Boolean finalVar;
    @JsonProperty("fields")
    private Map<String, CfgField> fields = new HashMap<>();
    @JsonProperty("members")
    private List<ClassMember> members = new ArrayList<>();
    @JsonProperty("var_type")
    private CfgType varType;
}
