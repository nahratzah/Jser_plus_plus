package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import static java.util.Objects.requireNonNull;

/**
 * Models a field.
 *
 * @author ariane
 */
public class FieldType {
    public FieldType(String name, Type type) {
        this(name, type, true);
    }

    public FieldType(String name, Type type, boolean enableSerialization) {
        this.name = requireNonNull(name);
        this.type = requireNonNull(type);
        this.encodeEnabled = this.decodeEnabled = enableSerialization;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public Type getType() {
        return type;
    }

    public void setType(Type type) {
        this.type = type;
    }

    public Type getVarType() {
        return (varType != null ? varType : getType());
    }

    public void setVarType(Type varType) {
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
        return setterFn == null ? getVisibility() == Visibility.PUBLIC && !isFinal() : setterFn;
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

    /**
     * Test if the field is final.
     *
     * Final fields will have const pointers, preventing those pointers from
     * aiming at anything else. Note that this does not affect if the object
     * pointed at is mutable.
     *
     * @return True if the field is a final field.
     */
    public boolean isFinal() {
        return finalVar;
    }

    /**
     * Change if the field is final.
     *
     * Final fields will have const pointers, preventing those pointers from
     * aiming at anything else. Note that this does not affect if the object
     * pointed at is mutable.
     *
     * @param finalVar If true, the field pointer will not be mutable.
     */
    public void setFinal(boolean finalVar) {
        this.finalVar = finalVar;
    }

    /**
     * Get the documentation for the field.
     *
     * @return Documentation of the field, or null if the field is undocumented.
     */
    public String getDocString() {
        return docString;
    }

    public void setDocString(String docString) {
        this.docString = docString;
    }

    @Override
    public String toString() {
        return "FieldType{" + "name=" + name + ", type=" + type + ", varType=" + varType + ", encodeEnabled=" + encodeEnabled + ", decodeEnabled=" + decodeEnabled + '}';
    }

    private String name;
    private Type type;
    private Type varType;
    private boolean encodeEnabled;
    private boolean decodeEnabled;
    private Visibility visibility = Visibility.PUBLIC;
    private Boolean getterFn = null;
    private Boolean setterFn = null;
    private boolean finalVar = false;
    private String docString = null;
}
