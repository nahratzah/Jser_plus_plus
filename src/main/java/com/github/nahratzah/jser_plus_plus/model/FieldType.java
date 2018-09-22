package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Collection;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.function.Function;
import java.util.stream.Collectors;
import org.stringtemplate.v4.ST;

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
        this.serializationName = name;
    }

    public String getSerializationName() {
        return serializationName;
    }

    public void setSerializationName(String serializationName) {
        this.serializationName = serializationName;
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
        Type result = (varType != null ? varType : getType());

        if (result instanceof BoundTemplate.ClassBinding) {
            final Type varTypeOfResult = ((BoundTemplate.ClassBinding) result).getType().getVarType();
            if (varTypeOfResult != null) result = varTypeOfResult;
        }

        if (isConst()
                && !(result instanceof CxxType)
                && !(result instanceof BoundTemplate.ClassBinding && ((BoundTemplate.ClassBinding) result).getType() instanceof PrimitiveType)) {
            result = new ConstType(result);
        }

        return result;
    }

    public Type getFieldType() {
        Type result = getVarType();
        if (isConst()) result = new ConstType(result);
        return result;
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
        if (setterFn != null) return setterFn;
        return getVisibility() == Visibility.PUBLIC
                && (getVarType() instanceof CxxType ? !isConst() : !isFinal());
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
     * Test if the field is const.
     *
     * Const-ness is a property of the element pointed to. If the field is not a
     * pointer field, it applies to the type directly.
     *
     * @return True is the element type if const.
     */
    public boolean isConst() {
        return constVar;
    }

    /**
     * Change if the field is const.
     *
     * @param constVar Value indicating if the field should be a const field.
     */
    public void setConst(boolean constVar) {
        this.constVar = constVar;
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

    /**
     * Get the default initializer for this field. May be null.
     *
     * @return The default initializer.
     */
    public String getDefault() {
        return defaultInit;
    }

    public void setDefault(String defaultInit) {
        this.defaultInit = defaultInit;
    }

    /**
     * Prerender the types.
     *
     * @param ctx Context for type lookups.
     * @param renderArgs Arguments to the renderer.
     * @param variables List of type variables in this context.
     */
    public void prerender(Context ctx, Map<String, ?> renderArgs, Collection<String> variables) {
        if (type != null)
            type = type.prerender(ctx, renderArgs, variables);
        if (varType != null)
            varType = varType.prerender(ctx, renderArgs, variables);

        defaultInit = prerender(defaultInit, ctx, renderArgs, variables);
    }

    /**
     * Prerender the types.
     *
     * @param ctx Context for type lookups.
     * @param renderArgs Arguments to the renderer.
     * @param variables List of type variables in this context.
     */
    public void prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
        if (type != null)
            type = type.prerender(ctx, renderArgs, variables);
        if (varType != null)
            varType = varType.prerender(ctx, renderArgs, variables);

        defaultInit = prerender(defaultInit, ctx, renderArgs, variables);
    }

    private static String prerender(String text, Context ctx, Map<String, ?> renderArgs, Collection<String> variables) {
        final Map<String, BoundTemplate.VarBinding> variablesMap = variables.stream()
                .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));
        return prerender(text, ctx, renderArgs, variablesMap);
    }

    private static String prerender(String text, Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
        if (text == null) return null;

        final Collection<Type> newDeclTypes = new HashSet<>(); // XXX use
        final ST stringTemplate = new ST(StCtx.contextGroup(ctx, variables, newDeclTypes::add), text);
        renderArgs.forEach(stringTemplate::add);
        return stringTemplate.render(Locale.ROOT);
    }

    @Override
    public String toString() {
        return "FieldType{" + "name=" + name + ", type=" + type + ", varType=" + varType + ", encodeEnabled=" + encodeEnabled + ", decodeEnabled=" + decodeEnabled + '}';
    }

    private String serializationName;
    private String name;
    private Type type;
    private Type varType;
    private boolean encodeEnabled;
    private boolean decodeEnabled;
    private Visibility visibility = Visibility.PUBLIC;
    private Boolean getterFn = null;
    private Boolean setterFn = null;
    private boolean finalVar = false;
    private boolean constVar = false;
    private String docString = null;
    private String defaultInit = null;
}
