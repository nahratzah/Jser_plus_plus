package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import static java.util.Objects.requireNonNull;

/**
 * Models a field.
 *
 * @author ariane
 */
public class FieldType {
    public FieldType(String name, BoundTemplate type) {
        this(name, type, true);
    }

    public FieldType(String name, BoundTemplate type, boolean enableSerialization) {
        this.name = requireNonNull(name);
        this.type = this.varType = requireNonNull(type);
        this.encodeEnabled = this.decodeEnabled = enableSerialization;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public BoundTemplate getType() {
        return type;
    }

    public void setType(BoundTemplate type) {
        this.type = type;
    }

    public BoundTemplate getVarType() {
        return varType;
    }

    public void setVarType(BoundTemplate varType) {
        this.varType = varType;
    }

    public boolean isEncodeEnabled() {
        return encodeEnabled;
    }

    public void setEncodeEnabled(boolean encodeEnabled) {
        this.encodeEnabled = encodeEnabled;
    }

    public boolean isDecodeEnabled() {
        return decodeEnabled;
    }

    public void setDecodeEnabled(boolean decodeEnabled) {
        this.decodeEnabled = decodeEnabled;
    }

    /**
     * Field visibility.
     *
     * @return The visibility of the field.
     */
    public Visibility getVisibility() {
        return visibility;
    }

    /**
     * Assign visibility for this field.
     *
     * @param visibility The visibility for the field.
     */
    public void setVisibility(Visibility visibility) {
        this.visibility = visibility;
    }

    /**
     * Allow a getter function for this field.
     *
     * @return True if a getter function for this field is to be generated.
     */
    public boolean isGetterFn() {
        return getterFn == null ? visibility == Visibility.PUBLIC : getterFn;
    }

    /**
     * Enable or disable getter function generation.
     *
     * A null value indicates default rules. The default rules are to generate
     * one if visibility is public.
     *
     * @param getterFn A boolean indicating whether a getter function is to be
     * generated. If the parameter is null, the default rules apply.
     */
    public void setGetterFn(Boolean getterFn) {
        this.getterFn = getterFn;
    }

    /**
     * Allow a setter function for this field.
     *
     * @return True if a setter function for this field is to be generated.
     */
    public boolean isSetterFn() {
        return setterFn == null ? visibility == Visibility.PUBLIC : setterFn;
    }

    /**
     * Enable or disable setter function generation.
     *
     * A null value indicates default rules. The default rules are to generate
     * one if visibility is public.
     *
     * @param setterFn A boolean indicating whether a setter function is to be
     * generated. If the parameter is null, the default rules apply.
     */
    public void setSetterFn(Boolean setterFn) {
        this.setterFn = setterFn;
    }

    @Override
    public String toString() {
        return "FieldType{" + "name=" + name + ", type=" + type + ", varType=" + varType + ", encodeEnabled=" + encodeEnabled + ", decodeEnabled=" + decodeEnabled + '}';
    }

    private String name;
    private BoundTemplate type;
    private BoundTemplate varType;
    private boolean encodeEnabled;
    private boolean decodeEnabled;
    private Visibility visibility = Visibility.PUBLIC;
    private Boolean getterFn = null;
    private Boolean setterFn = null;
}
