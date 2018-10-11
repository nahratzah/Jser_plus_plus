package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.DecodeStage;
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

    public Boolean isFinal() {
        return finalVar;
    }

    public void setFinal(Boolean finalVar) {
        this.finalVar = finalVar;
    }

    public Boolean isConst() {
        return constVar;
    }

    public void setConst(Boolean constVar) {
        this.constVar = constVar;
    }

    public String getDocString() {
        return docString;
    }

    public void setDocString(String docString) {
        this.docString = docString;
    }

    public CfgType getType() {
        return type;
    }

    public void setType(CfgType type) {
        this.type = type;
    }

    public String getDefault() {
        return defaultInit;
    }

    public void setDefault(String defaultInit) {
        this.defaultInit = defaultInit;
    }

    public String getRename() {
        return rename;
    }

    public void setRename(String rename) {
        this.rename = rename;
    }

    public boolean isCompleteInit() {
        return completeInit;
    }

    public void setCompleteInit(boolean completeInit) {
        this.completeInit = completeInit;
    }

    public DecodeStage getDecodeStage() {
        return decodeStage;
    }

    public void setDecodeStage(DecodeStage decodeStage) {
        this.decodeStage = decodeStage;
    }

    @JsonProperty("getter")
    private Boolean getterFn;
    @JsonProperty("setter")
    private Boolean setterFn;
    @JsonProperty("visibility")
    private Visibility visibility;
    @JsonProperty("final")
    private Boolean finalVar = null;
    @JsonProperty("const")
    private Boolean constVar = null;
    @JsonProperty("doc_string")
    private String docString = null;
    @JsonProperty(value = "type", required = true)
    private CfgType type;
    @JsonProperty(value = "default")
    private String defaultInit;
    @JsonProperty(value = "rename")
    private String rename;
    @JsonProperty(value = "decode_complete")
    private boolean completeInit = false;
    @JsonProperty(value = "decode_stage")
    private DecodeStage decodeStage;
}
