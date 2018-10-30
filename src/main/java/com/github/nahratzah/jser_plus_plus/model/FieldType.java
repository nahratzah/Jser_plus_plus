package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.cplusplus.DecodeStage;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import org.stringtemplate.v4.ST;

/**
 * Models a field.
 *
 * @author ariane
 */
public class FieldType {
    public FieldType(Context ctx, ClassType declaringClass, String name, Type type) {
        this(ctx, declaringClass, name, type, true);
    }

    public FieldType(Context ctx, ClassType declaringClass, String name, Type type, boolean enableSerialization) {
        this.ctx = requireNonNull(ctx);
        this.declaringClass = requireNonNull(declaringClass);
        this.serializationName = this.name = requireNonNull(name);
        this.type = requireNonNull(type);
        this.encodeEnabled = this.decodeEnabled = enableSerialization;
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

    private Type getRawVarType() {
        if (varType != null) {
            if (constVar)
                return new ConstType(varType);
            return varType;
        }

        Type result = getType();
        boolean makeConst = constVar;
        if (result instanceof ConstType) {
            makeConst = true;
            result = ((ConstType) result).getType();
        }

        if (result instanceof BoundTemplate.ClassBinding) {
            Type varTypeOfResult = ((BoundTemplate.ClassBinding<?>) result).getType().getVarType();
            if (varTypeOfResult != null) {
                if (varTypeOfResult instanceof ConstType) {
                    makeConst = true;
                    varTypeOfResult = ((ConstType) varTypeOfResult).getType();
                }

                if (varTypeOfResult instanceof BoundTemplate) {
                    result = ((BoundTemplate) varTypeOfResult).rebind(((BoundTemplate.ClassBinding<?>) result).getBindingsMap());
                } else {
                    result = varTypeOfResult;
                }
            }
        }

        if (makeConst)
            result = new ConstType(result);

        return result;
    }

    public Type getFieldType() {
        Type result = getVarType();
        if (isConst()) result = new ConstType(result);
        return result;
    }

    public Type getVarType() {
        final Type result = getRawVarType();
        if (result instanceof ConstType) {
            final ConstType constType = (ConstType) result;
            final Type rawType = constType.getType();
            if ((rawType instanceof CxxType)
                    || (rawType instanceof BoundTemplate.ClassBinding && ((BoundTemplate.ClassBinding<?>) rawType).getType() instanceof PrimitiveType))
                return rawType;
        }
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
        return constVar || (getRawVarType() instanceof ConstType);
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
     * Test if the field is using shared encoding.
     *
     * Default is to use shared encoding.
     *
     * @return True if the field uses shared encoding.
     */
    public boolean isShared() {
        return sharedVar;
    }

    /**
     * Change if the field uses shared encoding.
     *
     * @param sharedVar Value indicating if the field is shared.
     */
    public void setShared(boolean sharedVar) {
        this.sharedVar = sharedVar;
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

    public boolean isJavaClassVarType() {
        final boolean javaFieldType;

        Type rawVarType = getRawVarType();
        if (rawVarType instanceof ConstType)
            rawVarType = ((ConstType) rawVarType).getType();
        if (rawVarType instanceof BoundTemplate.ClassBinding) {
            final JavaType classBindingType = ((BoundTemplate.ClassBinding<?>) rawVarType).getType();
            javaFieldType = !(classBindingType instanceof PrimitiveType);
        } else {
            javaFieldType = rawVarType instanceof BoundTemplate;
        }

        return javaFieldType;
    }

    /**
     * Get the default initializer for this field. May be null.
     *
     * @return The default initializer.
     */
    public String getDefault() {
        if (isJavaClassVarType()) {
            if (defaultInit == null || defaultInit.trim().isEmpty())
                return "*this";
            return "*this, " + defaultInit;
        }
        return defaultInit;
    }

    public void setDefault(String defaultInit) {
        this.defaultInit = defaultInit;
    }

    /**
     * Test if the field must be completely initialized at construction time.
     *
     * @return True if the field must be completely initialized at construction
     * time.
     */
    public boolean isCompleteInit() {
        return completeInit;
    }

    public void setCompleteInit(boolean completeInit) {
        this.completeInit = completeInit;
    }

    /**
     * Get the decode stage at which this field must be present.
     *
     * If the field is a primitive, final, or a non-java const field, this will
     * always return {@link DecodeStage#INITIAL INITIAL}.
     *
     * The default is to put fields that have a serialization name starting with
     * {@code "this$"} as {@link DecodeStage#COMPLETE COMPLETE}, and others as
     * {@link DecodeStage#COMPARABLE COMPARABLE}.
     *
     * If the field is specified as having
     * {@link FieldType#isCompleteInit() complete-init}, the default is to use
     * {@link DecodeStage#INITIAL INITIAL}.
     *
     * @return The decode stage of the field.
     */
    public DecodeStage getDecodeStage() {
        if (this.getSerializationType().isPrimitive() && !this.isSerializationArray())
            return DecodeStage.INITIAL;
        if (this.getVarType() instanceof BoundTemplate) {
            if (this.isFinal()) return DecodeStage.INITIAL;
        } else {
            if (this.isConst()) return DecodeStage.INITIAL;
        }

        if (decodeStage == null) {
            if (this.isCompleteInit())
                return DecodeStage.INITIAL;
            if (this.getSerializationName().startsWith("this$"))
                return DecodeStage.COMPLETE;
            return DecodeStage.COMPARABLE;
        }

        return decodeStage;
    }

    public void setDecodeStage(DecodeStage decodeStage) {
        this.decodeStage = decodeStage;
    }

    /**
     * Test if the field is to be omitted from the class declaration.
     *
     * @return True if the field is to be omitted from the class declaration.
     */
    public boolean isOmit() {
        return omit;
    }

    public void setOmit(boolean omit) {
        this.omit = omit;
    }

    /**
     * Specialized encoder expression.
     *
     * @return Encoder expression.
     */
    public String getEncoderExpr() {
        if (encoderExpr == null) return null;

        String expr = encoderExpr;
        if (expr.endsWith("\n")) expr = expr.substring(0, expr.length() - 1);

        return new ST(StCtx.BUILTINS, expr)
                .add("model", declaringClass)
                .add("this", "_self_")
                .render(Locale.ROOT);
    }

    public void setEncoderExpr(String encoderExpr) {
        this.encoderExpr = encoderExpr;
    }

    /**
     * Get the serialization type.
     *
     * @return The serialization type, without array extents.
     */
    public JavaType getSerializationType() {
        if (!(type instanceof BoundTemplate))
            throw new IllegalStateException("Type is not a java type.");

        return ((BoundTemplate) type).visit(new BoundTemplate.Visitor<JavaType>() {
            @Override
            public JavaType apply(BoundTemplate.VarBinding b) {
                return declaringClass.getTemplateArguments().stream()
                        .filter(cta -> Objects.equals(cta.getName(), b.getName()))
                        .map(ClassTemplateArgument::getExtendBounds)
                        .map(BoundTemplate.MultiType::maybeMakeMultiType)
                        .findAny()
                        .orElseThrow(() -> new IllegalStateException("unable to find template variable " + b.getName()))
                        .visit(this);
            }

            @Override
            public JavaType apply(BoundTemplate.ClassBinding<?> b) {
                return b.getType();
            }

            @Override
            public JavaType apply(BoundTemplate.ArrayBinding b) {
                return b.getType().visit(this);
            }

            @Override
            public JavaType apply(BoundTemplate.Any b) {
                return ctx.resolveClass(Object.class.getName());
            }

            @Override
            public JavaType apply(BoundTemplate.MultiType b) {
                if (b.getTypes().size() == 1)
                    return b.getTypes().iterator().next().visit(this);

                return ctx.resolveClass(Object.class.getName());
            }
        });
    }

    /**
     * Get array extents of the serialization type.
     *
     * @return The number of array extents of the serialization type.
     */
    public int getSerializationExtents() {
        if (!(type instanceof BoundTemplate)) return 0;

        return ((BoundTemplate) type).visit(new BoundTemplate.Visitor<Integer>() {
            @Override
            public Integer apply(BoundTemplate.VarBinding b) {
                return 0;
            }

            @Override
            public Integer apply(BoundTemplate.ClassBinding<?> b) {
                return 0;
            }

            @Override
            public Integer apply(BoundTemplate.ArrayBinding b) {
                return b.getExtents();
            }

            @Override
            public Integer apply(BoundTemplate.Any b) {
                return 0;
            }

            @Override
            public Integer apply(BoundTemplate.MultiType b) {
                return 0;
            }
        });
    }

    public boolean isSerializationArray() {
        return getSerializationExtents() != 0;
    }

    @Override
    public String toString() {
        return "FieldType{" + "name=" + name + ", type=" + type + ", varType=" + varType + ", encodeEnabled=" + encodeEnabled + ", decodeEnabled=" + decodeEnabled + '}';
    }

    private final Context ctx;
    private final ClassType declaringClass;
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
    private boolean sharedVar = true;
    private String docString = null;
    private String defaultInit = null;
    private boolean completeInit = false;
    private DecodeStage decodeStage;
    private boolean omit = false;
    private String encoderExpr;
}
