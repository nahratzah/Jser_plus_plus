package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

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

    /**
     * Retrieve all friend types.
     *
     * @return List of friend types.
     */
    public List<CfgType> getFriends() {
        return friends;
    }

    public void setFriends(List<CfgType> friends) {
        this.friends = friends;
    }

    /**
     * Retrieve the class documentation string.
     *
     * @return The documentation string of this class.
     */
    public String getDocString() {
        return docString;
    }

    public void setDocString(String docString) {
        this.docString = docString;
    }

    /**
     * Test if development mode is enabled for this class.
     *
     * @return A boolean indicating if development mode is enabled. Or null if
     * unspecified.
     */
    public Boolean getDevMode() {
        return devMode;
    }

    public void setDevMode(Boolean devMode) {
        this.devMode = devMode;
    }

    /**
     * Rules that are suppressed in this class, but not in any classes derived
     * from this class.
     *
     * @return {@link Rule#getId() Rule IDs} of all suppressed
     * {@link Rule rules}.
     */
    public Set<String> getLocalSuppressedRules() {
        return localSuppressedRules;
    }

    public void setLocalSuppressedRules(Set<String> localSuppressedRules) {
        this.localSuppressedRules = localSuppressedRules;
    }

    /**
     * Rules that are suppressed in this class and any classes derived from this
     * class.
     *
     * @return {@link Rule#getId() Rule IDs} of all suppressed
     * {@link Rule rules}.
     */
    public Set<String> getInheritSuppressedRules() {
        return inheritSuppressedRules;
    }

    public void setInheritSuppressedRules(Set<String> inheritSuppressedRules) {
        this.inheritSuppressedRules = inheritSuppressedRules;
    }

    /**
     * Get raw source file input that is emitted before any includes.
     *
     * @return String with raw source file content.
     */
    public String getSrcRaw() {
        return srcRaw;
    }

    public void setSrcRaw(String srcRaw) {
        this.srcRaw = srcRaw;
    }

    @JsonProperty("final")
    private Boolean finalVar;
    @JsonProperty("fields")
    private Map<String, CfgField> fields = new HashMap<>();
    @JsonProperty("members")
    private List<ClassMember> members = new ArrayList<>();
    @JsonProperty("var_type")
    private CfgType varType;
    @JsonProperty("friends")
    private List<CfgType> friends = new ArrayList<>();
    @JsonProperty("doc_string")
    private String docString = null;
    @JsonProperty("dev_mode")
    private Boolean devMode = null;
    @JsonProperty("suppress_local")
    private Set<String> localSuppressedRules = new HashSet<>();
    @JsonProperty("suppress")
    private Set<String> inheritSuppressedRules = new HashSet<>();
    @JsonProperty("src_raw")
    private String srcRaw;
}
