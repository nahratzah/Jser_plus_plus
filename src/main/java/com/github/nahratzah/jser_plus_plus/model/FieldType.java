package com.github.nahratzah.jser_plus_plus.model;

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

    @Override
    public String toString() {
        return "FieldType{" + "name=" + name + ", type=" + type + ", varType=" + varType + ", encodeEnabled=" + encodeEnabled + ", decodeEnabled=" + decodeEnabled + '}';
    }

    private String name;
    private BoundTemplate type;
    private BoundTemplate varType;
    private boolean encodeEnabled;
    private boolean decodeEnabled;
}
