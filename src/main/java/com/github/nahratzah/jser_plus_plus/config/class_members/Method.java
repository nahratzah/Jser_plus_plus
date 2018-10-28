package com.github.nahratzah.jser_plus_plus.config.class_members;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Describe a class method.
 *
 * @author ariane
 */
public class Method implements ClassMember {
    @JsonProperty(value = "name", required = true)
    private String name;
    @JsonProperty("return")
    private CfgType returnType;
    @JsonProperty("arguments")
    private List<CfgArgument> arguments = new ArrayList<>();
    @JsonProperty("includes")
    private Includes includes = new Includes();
    @JsonProperty("body")
    private String body;
    @JsonProperty("virtual")
    private boolean virtual = false;
    @JsonProperty("override")
    private boolean override = false;
    @JsonProperty("const")
    private boolean constVar = false;
    @JsonProperty("final")
    private boolean finalVar = false;
    @JsonProperty("noexcept")
    private Object noexcept = Boolean.FALSE;
    @JsonProperty("static")
    private boolean staticVar = false;
    @JsonProperty("visibility")
    private Visibility visibility = Visibility.PUBLIC;
    @JsonProperty("doc_string")
    private String docString;
    @JsonProperty("covariant_return")
    private Boolean covariantReturn = null;
    @JsonProperty("allow_hide")
    private boolean hideOk = false;
    @JsonProperty("generics")
    private Map<String, String> generics = new LinkedHashMap<>();

    public Method() {
        returnType = new CfgType("void", null);
    }

    @Override
    public <T> T visit(Visitor<T> visitor) {
        return visitor.apply(this);
    }

    /**
     * Name of the method.
     *
     * @return String representing the name of the method.
     */
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    /**
     * Retrieve the templated return type of the method.
     *
     * @return A string encoding of the ST4 template.
     */
    public CfgType getReturnType() {
        return returnType;
    }

    public void setReturnType(CfgType returnType) {
        this.returnType = returnType;
    }

    /**
     * Retrieve the templated arguments of the method.
     *
     * @return A list of strings (one for each argument) encoding of the ST4
     * template.
     */
    public List<CfgArgument> getArguments() {
        return arguments;
    }

    public void setArguments(List<CfgArgument> arguments) {
        this.arguments = arguments;
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

    /**
     * If set, the method must be virtual.
     *
     * @return True if the method must be virtual. False if default rules apply.
     */
    public boolean isVirtual() {
        return virtual;
    }

    public void setVirtual(boolean virtual) {
        this.virtual = virtual;
    }

    /**
     * If set, the method is overriding another method.
     *
     * @return True if this method overrides a method from a super class or
     * interface.
     */
    public boolean isOverride() {
        return override;
    }

    public void setOverride(boolean override) {
        this.override = override;
    }

    /**
     * If set, the method is a const method.
     *
     * @return True if the method is a const method. False otherwise.
     */
    public boolean isConst() {
        return constVar;
    }

    public void setConst(boolean constVar) {
        this.constVar = constVar;
    }

    /**
     * If set, the method is a final method.
     *
     * @return True if the method is final. False otherwise.
     */
    public boolean isFinal() {
        return finalVar;
    }

    public void setFinal(boolean finalVar) {
        this.finalVar = finalVar;
    }

    /**
     * Noexcept specification.
     *
     * This may either be a String, in which case it contains the noexcept
     * predicate, or a boolean.
     *
     * @return The noexcept specification of the function, or null.
     */
    public Object getNoexcept() {
        return noexcept;
    }

    public void setNoexcept(Object noexcept) {
        if (noexcept != null && !(noexcept instanceof String) && !(noexcept instanceof Boolean))
            throw new IllegalArgumentException("Expected string or boolean value.");
        this.noexcept = noexcept;
    }

    /**
     * Test if the method is static.
     *
     * @return True if the method is static, false otherwise.
     */
    public boolean isStatic() {
        return staticVar;
    }

    public void setStatic(boolean staticVar) {
        this.staticVar = staticVar;
    }

    @Override
    public Visibility getVisibility() {
        return visibility;
    }

    public void setVisibility(Visibility visibility) {
        this.visibility = visibility;
    }

    public String getDocString() {
        return docString;
    }

    public void setDocString(String docString) {
        this.docString = docString;
    }

    /**
     * If the method uses a covariant return value.
     *
     * @return True if the method is to use a covariant return type. False if
     * not. If the return is {@code null}, default rules apply.
     */
    public Boolean isCovariantReturn() {
        return covariantReturn;
    }

    public void setCovariantReturn(boolean covariantReturn) {
        this.covariantReturn = covariantReturn;
    }

    /**
     * If the method is allowed to silently hide methods with the same signature
     * from the super class.
     *
     * @return True if it is okay for this method to hide similar methods from
     * super class.
     */
    public boolean isHideOk() {
        return hideOk;
    }

    public void setHideOk(boolean hideOk) {
        this.hideOk = hideOk;
    }

    /**
     * Additional variables for the purpose of generics.
     *
     * The key of the map is the variable name.
     *
     * The value of the map is a string encoding for a generic.
     *
     * @return Map of variable name to generic definition.
     */
    public Map<String, String> getGenerics() {
        return generics;
    }

    public void setGenerics(Map<String, String> generics) {
        this.generics = generics;
    }
}
