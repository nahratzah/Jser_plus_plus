package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.CfgField;
import com.github.nahratzah.jser_plus_plus.config.ClassConfig;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.java.ReflectUtil;
import com.github.nahratzah.jser_plus_plus.misc.SimpleMapEntry;
import static com.github.nahratzah.jser_plus_plus.model.JavaType.getAllTypeParameters;
import static com.github.nahratzah.jser_plus_plus.model.Type.typeFromCfgType;
import com.github.nahratzah.jser_plus_plus.output.builtins.ConstTypeRenderer;
import com.github.nahratzah.jser_plus_plus.output.builtins.FunctionAttrMap;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.io.ObjectStreamClass;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singletonList;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableCollection;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableMap;
import java.util.Comparator;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.function.BiConsumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Models a normal class.
 *
 * @author ariane
 */
public class ClassType implements JavaType {
    private static final Logger LOG = Logger.getLogger(ClassType.class.getName());
    private static final boolean WRITE_POST_PROCESSING_RESULT = false;
    private static final String VIRTUAL_FUNCTION_PREFIX = "_virtual_";
    private static final String REDECLARE_BODY_TEMPLATE = "return "
            + "$if (needCast)$::java::cast<$boundTemplateType(declare.returnType, \"style=type, class\")$>($endif$"
            + "this->$underlying.declaringClass.className$::$underlying.name$"
            + "($declare.underlyingMethod.argumentNames: { name | ::std::forward<decltype($name$)>($name$)}; anchor, wrap, separator = \", \"$)"
            + "$if (needCast)$)$endif$;";
    private static final String FORWARDING_BODY_TEMPLATE = "return "
            + "this->"
            + "$if (covariantReturn)$" + VIRTUAL_FUNCTION_PREFIX + "$endif$"
            + "$fn.name$"
            + "("
            + "$if (covariantReturn)$$tagType(model)$()$endif$$if (covariantReturn && fn.underlyingMethod.argumentNames)$, $endif$"
            + "$fn.underlyingMethod.argumentNames: { name | ::java::_maybe_cast(::std::forward<decltype($name$)>($name$))}; anchor, wrap, separator = \", \"$"
            + ")"
            + ";";

    public ClassType(Class<?> c) {
        this.c = requireNonNull(c);
    }

    @Override
    public void init(Context ctx, Config cfg) {
        LOG.log(Level.FINE, "Initializing {0}", c);
        final ClassConfig classCfg = cfg.getConfigForClass(c);

        final List<? extends TypeVariable<? extends Class<?>>> cTypeParameters = getAllTypeParameters(this.c);
        LOG.log(Level.FINE, "Type parameters: {0}", cTypeParameters);
        final Map<String, String> argRename = unmodifiableMap(cfg.getTemplateArguments(c.getName())
                .map(cfgParameters -> buildRenameMap(cTypeParameters, cfgParameters))
                .orElseGet(() -> buildRenameMap(cTypeParameters)));
        LOG.log(Level.FINE, "Rename map for {0} = {1}", new Object[]{c, argRename});

        final ObjectStreamClass streamClass = ObjectStreamClass.lookupAny(this.c);

        this.serialVersionUID = streamClass.getSerialVersionUID();
        initTemplateArguments(ctx, classCfg, cTypeParameters, argRename);
        initClassAttributes(ctx, classCfg, argRename);
        initSuperTypes(ctx, classCfg, argRename);

        classCfg.updateWithSuperTypes(
                isAbstract(),
                isInterface(),
                isEnum(),
                cfgType());

        initFields(ctx, classCfg, argRename, streamClass);
        initClassMembers(ctx, classCfg, argRename);
        initFriendTypes(ctx, classCfg, argRename);

        if (findAllInherits().anyMatch(type -> ctx.resolveClass(java.io.Serializable.class.getName()) == type)
                && !findAllInherits().anyMatch(type -> ctx.resolveClass(java.lang.Enum.class.getName()) == type))
            initSerializationConstructor(ctx, classCfg, argRename);
    }

    private void initTemplateArguments(Context ctx, ClassConfig classCfg, List<? extends TypeVariable<? extends Class<?>>> cTypeParameters, Map<String, String> argRename) {
        this.templateArguments = cTypeParameters.stream()
                .map(cTypeParameter -> {
                    final String argName = requireNonNull(argRename.get(cTypeParameter.getName()));
                    final List<BoundTemplate> argBounds = Arrays.stream(cTypeParameter.getBounds())
                            .map(b -> ReflectUtil.visitType(b, new BoundsMapping(ctx, argRename)))
                            .collect(Collectors.toList());
                    return new ClassTemplateArgument(argName, argBounds);
                })
                .collect(Collectors.toList());
    }

    private void initClassAttributes(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        this.finalVar = Modifier.isFinal(this.c.getModifiers());
        if (classCfg.isFinal() != null)
            this.finalVar = classCfg.isFinal();

        if (classCfg.getVarType() != null) {
            this.varType = typeFromCfgType(classCfg.getVarType(), ctx, argRename.values())
                    .prerender(ctx, singletonMap("model", this), argRename.values());
        }

        this.docString = classCfg.getDocString();
        this.devMode = classCfg.isDevMode();
    }

    private void initSuperTypes(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        this.superType = Optional.ofNullable(this.c.getGenericSuperclass())
                .map(t -> ReflectUtil.visitType(t, new ParentTypeVisitor(ctx, argRename)))
                .orElse(null);
        this.interfaceTypes = Arrays.stream(this.c.getGenericInterfaces())
                .map(t -> ReflectUtil.visitType(t, new ParentTypeVisitor(ctx, argRename)))
                .collect(Collectors.toList());
    }

    private void initFields(Context ctx, ClassConfig classCfg, Map<String, String> argRename, ObjectStreamClass streamClass) {
        class IntermediateFieldDescr {
            public IntermediateFieldDescr(String name, Class<?> classType, Type reflectType, Field reflectField, boolean shared) {
                this.name = requireNonNull(name);
                this.classType = requireNonNull(classType);
                this.reflectType = reflectType;
                this.reflectField = reflectField;
                this.shared = shared;
            }

            /**
             * Name of the field.
             */
            public final String name;
            /**
             * Serialization type of the field.
             */
            public final Class<?> classType;
            /**
             * Type of the field, found via reflection. May be null.
             */
            public final Type reflectType;
            /**
             * Field found using reflection. May be null.
             */
            public final Field reflectField;
            /**
             * True if the field uses shared encoding. False otherwise.
             */
            public final boolean shared;
        }

        // Generate fields based on reflection logic.
        final List<FieldType> reflectedFields = Arrays.stream(streamClass.getFields())
                .map(f -> {
                    final String name = f.getName();
                    final Class<?> type = f.getType();

                    // Fill in the reflect type only if the class implementation
                    // and the serialization code agree on the type.
                    Type reflectType;
                    Field reflectField;
                    try {
                        reflectField = this.c.getDeclaredField(name);
                        if (Objects.equals(reflectField.getType(), type)) {
                            reflectType = reflectField.getGenericType();
                        } else {
                            reflectType = null;
                            reflectField = null;
                        }
                    } catch (NoSuchFieldException ex) {
                        reflectType = null;
                        reflectField = null;
                    }
                    return new IntermediateFieldDescr(name, type, reflectType, reflectField, !f.isUnshared());
                })
                .map(iField -> {
                    final Type visitType = iField.reflectType != null ? iField.reflectType : iField.classType;
                    final BoundTemplate type = ReflectUtil.visitType(visitType, new BoundsMapping(ctx, argRename));

                    final FieldType fieldType = new FieldType(ctx, this, iField.name, type);
                    if (iField.reflectField != null) {
                        fieldType.setFinal(Modifier.isFinal(iField.reflectField.getModifiers()));
                    }
                    fieldType.setShared(iField.shared);
                    return fieldType;
                })
                .collect(Collectors.toList());
        // Store names of reflected fields, for quick access.
        final Set<String> reflectedFieldNames = reflectedFields.stream()
                .map(FieldType::getName)
                .collect(Collectors.toSet());

        // Generate list of configured fields.
        final List<FieldType> syntheticFields = classCfg.getFields().entrySet().stream()
                .filter(entry -> !reflectedFieldNames.contains(entry.getKey()))
                .map(entry -> {
                    final String name = entry.getKey();
                    final CfgField fieldCfg = entry.getValue();
                    final BoundTemplate type = new BoundTemplate.Any(); // XXX figure out how to configure a synthetic field!

                    return new FieldType(ctx, this, name, type, false);
                })
                .collect(Collectors.toList());

        // Apply configuration patches to all the fields.
        this.fields = Stream.concat(reflectedFields.stream(), syntheticFields.stream())
                .peek(iField -> {
                    final CfgField fieldCfg = classCfg.getFields().get(iField.getName());
                    if (fieldCfg == null) return;

                    if (fieldCfg.getVisibility() != null)
                        iField.setVisibility(fieldCfg.getVisibility());
                    iField.setGetterFn(fieldCfg.getGetterFn());
                    iField.setSetterFn(fieldCfg.getSetterFn());
                    if (fieldCfg.isFinal() != null)
                        iField.setFinal(fieldCfg.isFinal());
                    if (fieldCfg.isConst() != null)
                        iField.setConst(fieldCfg.isConst());
                    if (fieldCfg.getDocString() != null)
                        iField.setDocString(fieldCfg.getDocString());
                    if (fieldCfg.getType() != null)
                        iField.setType(typeFromCfgType(fieldCfg.getType(), ctx, argRename.values()));
                    if (fieldCfg.getDefault() != null)
                        iField.setDefault(fieldCfg.getDefault());
                    if (fieldCfg.getRename() != null)
                        iField.setName(fieldCfg.getRename());
                    iField.setCompleteInit(fieldCfg.isCompleteInit());
                    if (fieldCfg.getDecodeStage() != null)
                        iField.setDecodeStage(fieldCfg.getDecodeStage());
                    iField.setOmit(fieldCfg.isOmit());
                    if (fieldCfg.getEncoderExpr() != null)
                        iField.setEncoderExpr(fieldCfg.getEncoderExpr());
                })
                .peek(iField -> {
                    iField.prerender(singletonMap("model", this), getTemplateArgumentNames());
                })
                .collect(Collectors.toList());
    }

    private void initClassMembers(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        final ClassType cdef = this;
        this.classMembers = classCfg.getClassMembers()
                .map(cfgMember -> {
                    return cfgMember.visit(new ClassMember.Visitor<ClassMemberModel>() {
                        @Override
                        public ClassMemberModel apply(Method method) {
                            return new ClassMemberModel.ClassMethod(ctx, cdef, method);
                        }

                        @Override
                        public ClassMemberModel apply(Constructor constructor) {
                            return new ClassMemberModel.ClassConstructor(ctx, cdef, constructor);
                        }

                        @Override
                        public ClassMemberModel apply(Destructor destructor) {
                            throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
                        }
                    });
                })
                .collect(Collectors.toList());
    }

    private void initSerializationConstructor(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        final Predicate<FieldType> isPrimitive = field -> field.getSerializationType().isPrimitive() && !field.isSerializationArray();

        final ClassType superClass = (getSuperClass() == null ? null : getSuperClass().getType());
        final boolean superClassIsSerializable = superClass != null
                && superClass.findAllInherits()
                        .anyMatch(type -> ctx.resolveClass(java.io.Serializable.class.getName()) == type);

        // Build `__decoder__` argument.
        com.github.nahratzah.jser_plus_plus.config.CfgType ctxType = new com.github.nahratzah.jser_plus_plus.config.CfgType("const ::java::serialization::class_decoder<$tagType(model)$>&", null);
        ctxType.setIncludes(new Includes(
                singletonList("java/serialization/decoder_fwd.h"),
                singletonList("java/serialization/decoder.h")));
        final CfgArgument ctxArg = new CfgArgument();
        ctxArg.setName("__decoder__");
        ctxArg.setType(ctxType);

        final Constructor constructorCfg = new Constructor();
        if (superClassIsSerializable)
            constructorCfg.setSuperInitializer(ctxArg.getName());
        constructorCfg.setVisibility(Visibility.PROTECTED);
        constructorCfg.setArguments(singletonList(ctxArg));
        constructorCfg.setIncludes(new Includes(
                Arrays.asList(
                        "java/serialization/decoder_fwd.h"),
                Arrays.asList(
                        "cycle_ptr/cycle_ptr.h",
                        "java/serialization/decoder.h",
                        "java/ref.h",
                        "java/primitives.h",
                        "optional")));

        final Stream<Map.Entry<String, String>> primitiveFieldInitializers = getSerializationFields().stream()
                .filter(field -> !field.isOmit())
                .filter(field -> field.getDecodeStage().isAtInitialStage())
                .filter(isPrimitive)
                .map(field -> {
                    final String typeFullyQualifiedName = Stream.concat(field.getSerializationType().getNamespace().stream(), Stream.of(field.getSerializationType().getClassName()))
                            .collect(Collectors.joining("::", "::", ""));

                    final String initializer = ctxArg.getName() + ".get_primitive_field<${" + typeFullyQualifiedName.replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "}$>("
                            + "u$cxxString({" + field.getSerializationName().replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "})$"
                            + (field.getDefault() != null ? ", std::optional<${" + typeFullyQualifiedName.replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "}$>(" + field.getDefault() + ")" : "")
                            + ")";
                    return new SimpleMapEntry<>(field.getName(), initializer);
                });

        final Stream<Map.Entry<String, String>> finalFieldInitializers = getSerializationFields().stream()
                .filter(field -> !field.isOmit())
                .filter(field -> field.getDecodeStage().isAtInitialStage())
                .filter(isPrimitive.negate())
                .map(field -> {
                    final String castType = ConstTypeRenderer.apply(field.getVarType(), "style=type, class");
                    final String getFieldMethod = field.isCompleteInit() ? "get_complete_field" : "get_initial_field";

                    final String initializer = ctxArg.getName() + "." + getFieldMethod + "<${" + castType.replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "}$>("
                            + "u$cxxString({" + field.getSerializationName().replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "})$"
                            + ")"; // get_complete_field
                    return new SimpleMapEntry<>(field.getName(), initializer);
                });

        constructorCfg.setInitializers(Stream.of(primitiveFieldInitializers, finalFieldInitializers)
                .flatMap(Function.identity())
                .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue)));
        classMembers.add(new ClassMemberModel.ClassConstructor(ctx, this, constructorCfg));
    }

    private void initFriendTypes(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        this.friends = classCfg.getFriends().stream()
                .map(cfgType -> typeFromCfgType(cfgType, ctx, argRename.values()))
                .map(type -> type.prerender(ctx, singletonMap("model", this), argRename.values()))
                .collect(Collectors.toList());
    }

    @Override
    public int getNumTemplateArguments() {
        return getAllTypeParameters(this.c).size();
    }

    @Override
    public String getName() {
        return c.getName();
    }

    @Override
    public List<ClassTemplateArgument> getTemplateArguments() {
        requireNonNull(templateArguments, "must call init() method first!");
        return unmodifiableList(templateArguments);
    }

    /**
     * Retrieve all template arguments, with self-references erased, and
     * ordered.
     *
     * Returned arguments will be rebound to have self-references erased.
     * Returned arguments will be re-ordered to ensure no type depends on a type
     * that comes later in the list.
     *
     * @return Defaulted template arguments.
     */
    public ArrayList<Map.Entry<String, BoundTemplate>> getErasedTemplateArguments() {
        // Output result.
        final LinkedHashMap<String, List<BoundTemplate>> mapping = new LinkedHashMap<>();
        // Process raw arguments in deterministic order.
        final Map<String, List<BoundTemplate>> rawArguments = new LinkedHashMap<>();
        getTemplateArguments()
                .stream()
                .sorted(Comparator.comparing(ClassTemplateArgument::getName))
                .forEach(cta -> rawArguments.put(cta.getName(), cta.getExtendBounds()));

        class Processor implements BiConsumer<String, List<BoundTemplate>> {
            private final Set<String> inProgress = new HashSet<>();

            @Override
            public void accept(String name, List<BoundTemplate> rawExtendTypes) {
                if (mapping.containsKey(name)) return;

                if (!inProgress.add(name))
                    throw new IllegalStateException("Recursive use of template argument.");
                try {
                    // Build a map to clear self-references.
                    final Map<String, BoundTemplate> selfRebind = singletonMap(name, new BoundTemplate.Any());
                    // Replace self-references with the BoundTemplate.Any type.
                    final List<BoundTemplate> extendTypes = rawExtendTypes.stream()
                            .map(type -> type.rebind(selfRebind))
                            .collect(Collectors.toList());
                    // Figure out all argument names we depend on.
                    final List<String> requiredNames = extendTypes.stream()
                            .map(type -> type.getUnresolvedTemplateNames())
                            .flatMap(c -> c.stream())
                            .distinct()
                            .sorted()
                            .collect(Collectors.toList());

                    // Process all dependent names.
                    requiredNames.stream()
                            .filter(rawArguments::containsKey)
                            .forEach(requiredName -> accept(requiredName, rawArguments.get(requiredName)));

                    // Add current name.
                    mapping.put(name, extendTypes);
                } finally {
                    inProgress.remove(name);
                }
            }
        }

        // Process all arguments.
        rawArguments.forEach(new Processor());

        // Return result of processing operation.
        return new ArrayList<>(mapping.entrySet().stream()
                .map(mappingEntry -> new SimpleMapEntry<>(mappingEntry.getKey(), BoundTemplate.MultiType.maybeMakeMultiType(mappingEntry.getValue())))
                .collect(Collectors.toList()));
    }

    @Override
    public BoundTemplate.ClassBinding<? extends ClassType> getSuperClass() {
        return this.superType;
    }

    @Override
    public Collection<BoundTemplate.ClassBinding<? extends ClassType>> getInterfaces() {
        return unmodifiableCollection(this.interfaceTypes);
    }

    @Override
    public Stream<String> getImplementationIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        if (!recursionGuard.add(this)) return Stream.empty();

        final Stream<String> parentTypes = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull), getInterfaces().stream())
                .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));

        final Stream<String> fieldIncludes;
        final Stream<String> memberIncludes;
        final Stream<String> friendIncludes;
        if (publicOnly) {
            fieldIncludes = getFields().stream()
                    .map(field -> field.getVarType())
                    .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));
            memberIncludes = Stream.concat(
                    getClassMemberFunctions().stream()
                            .flatMap(member -> member.getDeclarationIncludes()),
                    getClassNonMemberFunctions().stream()
                            .flatMap(member -> member.getDeclarationIncludes()));
            friendIncludes = friends.stream()
                    .flatMap(friend -> friend.getIncludes(true));
        } else {
            fieldIncludes = getFields().stream()
                    .flatMap(field -> Stream.of(field.getType(), field.getVarType()))
                    .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));
            memberIncludes = Stream.concat(
                    getClassMemberFunctions().stream()
                            .flatMap(member -> member.getImplementationIncludes()),
                    getClassMembers().stream()
                            .flatMap(member -> member.getImplementationIncludes()));
            friendIncludes = friends.stream()
                    .flatMap(friend -> friend.getIncludes(false));
        }

        return Stream.of(parentTypes, fieldIncludes, memberIncludes, friendIncludes)
                .flatMap(Function.identity());
    }

    @Override
    public Stream<JavaType> getForwardDeclarationJavaTypes() {
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> superTypes = Stream.concat(
                Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> publicFields = getFields().stream()
                .filter(field -> field.isGetterFn() || field.isSetterFn())
                .map(field -> field.getVarType());
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> memberTypes = getClassMembers().stream()
                .filter(member -> member.getVisibility() == Visibility.PUBLIC)
                .flatMap(member -> member.getDeclarationTypes());

        return Stream.of(superTypes, publicFields, memberTypes)
                .flatMap(Function.identity())
                .flatMap(com.github.nahratzah.jser_plus_plus.model.Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getDeclarationCompleteJavaTypes() {
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> superTypes = Stream.concat(
                Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());

        return Stream.of(superTypes)
                .flatMap(Function.identity())
                .flatMap(com.github.nahratzah.jser_plus_plus.model.Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getDeclarationForwardJavaTypes() {
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> templateTypes = getTemplateArguments().stream()
                .map(ClassTemplateArgument::getExtendBounds)
                .flatMap(Collection::stream);
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> fieldTypes = getFields().stream()
                .map(field -> field.getVarType());
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> memberTypes = Stream.concat(
                this.getClassMemberFunctions().stream()
                        .flatMap(member -> member.getDeclarationTypes()),
                getClassNonMemberFunctions().stream()
                        .flatMap(member -> member.getDeclarationTypes()));

        return Stream.of(templateTypes, fieldTypes, memberTypes)
                .flatMap(Function.identity())
                .flatMap(com.github.nahratzah.jser_plus_plus.model.Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getImplementationJavaTypes() {
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> superTypes = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> fieldTypes = getFields().stream()
                .flatMap(field -> Stream.of(field.getType(), field.getVarType()));
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> memberTypes = Stream.concat(
                getClassMemberFunctions().stream()
                        .flatMap(member -> member.getImplementationTypes()),
                getClassNonMemberFunctions().stream()
                        .flatMap(member -> member.getImplementationTypes()));

        return Stream.of(superTypes, fieldTypes, memberTypes)
                .flatMap(Function.identity())
                .flatMap(com.github.nahratzah.jser_plus_plus.model.Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    public long getSerialVersionUID() {
        return serialVersionUID;
    }

    public String getSerialVersionUID_string() {
        return String.format("%#x", getSerialVersionUID()) + "ULL";
    }

    private Stream<ClassType> findAllInherits() {
        return Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull), getInterfaces().stream())
                .map(BoundTemplate.ClassBinding::getType)
                .flatMap((ClassType parentType) -> {
                    return Stream.concat(Stream.of(parentType), parentType.findAllInherits());
                });
    }

    public Map<String, Object> getTestInherits() {
        return new FunctionAttrMap((String clsName) -> {
            return Objects.equals(getName(), clsName)
                    || findAllInherits()
                            .map(ClassType::getName)
                            .anyMatch(clsName::equals);
        });
    }

    /**
     * Retrieve the fields of this type.
     *
     * @return The fields of this type.
     */
    public List<FieldType> getFields() {
        return fields.stream()
                .filter(f -> !f.isOmit())
                .collect(Collectors.toList());
    }

    /**
     * Retrieve the fields that have serialization enabled.
     *
     * @return List of fields for which serialization is enabled.
     */
    public List<FieldType> getSerializationFields() {
        return fields.stream()
                .filter(f -> f.isDecodeEnabled() || f.isEncodeEnabled())
                .collect(Collectors.toList());
    }

    @Override
    public boolean isInterface() {
        return Modifier.isInterface(this.c.getModifiers());
    }

    @Override
    public boolean isAbstract() {
        return Modifier.isAbstract(this.c.getModifiers()) && !isInterface();
    }

    @Override
    public boolean isFinal() {
        return finalVar;
    }

    public Collection<ClassMemberModel> getClassMembers() {
        return classMembers;
    }

    public Collection<ClassMemberModel.ClassConstructor> getConstructors() {
        return getClassMembers().stream()
                .filter(member -> member.getVisibility() == Visibility.PUBLIC)
                .filter(ClassMemberModel.ClassConstructor.class::isInstance)
                .map(ClassMemberModel.ClassConstructor.class::cast)
                .collect(Collectors.toList());
    }

    public Collection<MethodModel> getAccessorMethods() {
        return getClassMembers().stream()
                .filter(member -> !member.isStatic())
                .filter(MethodModel.class::isInstance)
                .map(MethodModel.class::cast)
                .filter(((Predicate<MethodModel>) suppressedAccessorMethods::contains).negate())
                .collect(Collectors.toList());
    }

    public Collection<MethodModel> getStaticAccessorMethods() {
        return getClassMembers().stream()
                .filter(member -> member.isPublicMethod())
                .filter(member -> member.isStatic())
                .filter(MethodModel.class::isInstance)
                .map(MethodModel.class::cast)
                .collect(Collectors.toList());
    }

    public List<MethodModel> getClassMemberFunctions() {
        return classMemberFunctions.stream()
                .sorted(Comparator.comparing(MethodModel::getVisibility)
                        .thenComparing(MethodModel::getName)
                        .thenComparing(MethodModel::isConst, Comparator.reverseOrder()))
                .collect(Collectors.toList());
    }

    public List<ClassMemberModel> getClassNonMemberFunctions() {
        return getClassMembers().stream()
                .filter(classMember -> classMember.isStatic() || !(classMember instanceof ClassMemberModel.ClassMethod))
                .collect(Collectors.toList());
    }

    /**
     * Test if this class has a default constructor defined.
     *
     * @return True if the class has a default constructor defined.
     */
    public boolean getHasDefaultConstructor() {
        return getClassMembers().stream()
                .anyMatch(ClassMemberModel::isDefaultConstructor);
    }

    /**
     * Get a version of this type with all template arguments bound to the
     * wildcard generic.
     *
     * @return A bound type for this class.
     */
    public com.github.nahratzah.jser_plus_plus.model.Type getPackType() {
        return new BoundTemplate.ClassBinding<>(
                this,
                getTemplateArgumentNames().stream().map(ignored -> new BoundTemplate.Any()).collect(Collectors.toList()));
    }

    /**
     * Get a version of this type with all template arguments bound to the
     * wildcard generic.
     *
     * @return A const bound type for this class.
     */
    public com.github.nahratzah.jser_plus_plus.model.Type getConstPackType() {
        return new ConstType(getPackType());
    }

    @Override
    public com.github.nahratzah.jser_plus_plus.model.Type getVarType() {
        return varType;
    }

    public List<com.github.nahratzah.jser_plus_plus.model.Type> getFriends() {
        return friends;
    }

    @Override
    public synchronized void postProcess(Context ctx) {
        if (postProcessingDone) return;
        postProcessingDone = true;

        // All parents.
        final List<BoundTemplate.ClassBinding<? extends ClassType>> parentModels = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull), getInterfaces().stream())
                .collect(Collectors.toList());

        // Ensure parent types have all had their post processing phase completed.
        parentModels.stream()
                .map(BoundTemplate.ClassBinding::getType)
                .forEach(parentType -> parentType.postProcess(ctx));

        // All methods from parents that have been superseded.
        // We'll add our own suppressed methods to this during the post processing phase.
        this.allResolvedMethods = parentModels.stream()
                .flatMap(parentTemplate -> {
                    final ClassType parentType = parentTemplate.getType();
                    return parentType.allResolvedMethods.stream();
                })
                .collect(Collectors.toSet());
        final Predicate<ClassMemberModel.OverrideSelector> isParentResolvedMethod = (selector) -> allResolvedMethods.contains(new ResolvedMethod(selector));

        // Figure out erased bindings for our type.
        final Map<String, BoundTemplate> erasedBindings = getErasedTemplateArguments().stream()
                .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));

        // Maintain a mapping from erased type to template type.
        // We want to put template types, not erased types, into the {@link ClassType#allMethods} collection.
        final Map<ClassType, Map<ClassMemberModel.OverrideSelector, ClassMemberModel.OverrideSelector>> erasedToTemplateMapping = parentModels.stream()
                .flatMap(parentTemplate -> {
                    final Map<String, BoundTemplate> bindingMap = parentTemplate.getBindingsMap();
                    return parentTemplate.getType().getAllMethods().stream()
                            .map(selector -> selector.rebind(bindingMap));
                })
                .peek(selector -> LOG.log(Level.FINE, "{0}: importing parent method {1}", new Object[]{getName(), selector}))
                .map(selector -> new SimpleMapEntry<>(selector.rebind(erasedBindings), selector))
                .collect(Collectors.groupingBy(
                        selector -> selector.getValue().getDeclaringClass(),
                        Collectors.toMap(
                                Map.Entry::getKey,
                                Map.Entry::getValue,
                                (selX, selY) -> {
                                    if (!Objects.equals(selX.getDeclaringType(), selY.getDeclaringType()))
                                        throw new IllegalStateException("Collision between methods: " + selX + ", " + selY);
                                    return selX;
                                })));
        final Function<ClassMemberModel.OverrideSelector, ClassMemberModel.OverrideSelector> erasedToTemplate = (selector) -> {
            final Map<ClassMemberModel.OverrideSelector, ClassMemberModel.OverrideSelector> classMap = requireNonNull(
                    erasedToTemplateMapping.get(selector.getDeclaringClass()),
                    "Class " + selector.getDeclaringClass().getName() + " not found: " + selector);
            return requireNonNull(
                    classMap.get(selector),
                    "Selector " + selector + " not found in class " + selector.getDeclaringClass().getName());
        };

        // All parent models, but with their template arguments bound, such that our erased types are propagated.
        final List<BoundTemplate.ClassBinding<? extends ClassType>> erasedParentModels = parentModels.stream()
                .map(parentTemplate -> parentTemplate.rebind(erasedBindings))
                .collect(Collectors.toList());

        // All locally declared member functions,
        // mapped to the methods they override.
        final Map<ClassMemberModel.OverrideSelector, List<ClassMemberModel.OverrideSelector>> myMethods;
        {
            final List<ClassMemberModel.OverrideSelector> myMethodsTmp = getClassMembers().stream()
                    .map(classMemberModel -> classMemberModel.getOverrideSelector(ctx))
                    .filter(Optional::isPresent)
                    .map(Optional::get)
                    .map(selector -> {
                        final ClassMemberModel.OverrideSelector erasedSelector = selector.getErasedMethod();
                        // Record mapping while we're here.
                        erasedToTemplateMapping
                                .computeIfAbsent(selector.getDeclaringClass(), x -> new HashMap<>())
                                .put(erasedSelector, selector);
                        return erasedSelector;
                    })
                    .collect(Collectors.toList());
            try {
                myMethods = myMethodsTmp.stream()
                        .collect(Collectors.toMap(Function.identity(), x -> new ArrayList<>()));
            } catch (IllegalStateException ex) {
                // Figure out all methods that cause collision and report them, then terminate with an exception.
                myMethodsTmp.stream()
                        .collect(Collectors.groupingBy(Function.identity()))
                        .values().stream()
                        .filter(methods -> methods.size() != 1)
                        .forEach(methods -> LOG.log(Level.SEVERE, "Colliding methods in {0}: {1}", new Object[]{getName(), methods}));
                throw new IllegalStateException("Method collision detected!", ex);
            }

            erasedParentModels.stream()
                    // Rebind and retrieve the parent methods, to the binding map (which now holds out erased bindings).
                    .flatMap(parentTemplate -> {
                        final ClassType parentType = parentTemplate.getType();
                        return Stream.concat(parentType.allResolvedMethods.stream().map(ResolvedMethod::getSelector), parentType.getAllMethods().stream())
                                .map(parentMethod -> parentMethod.rebind(parentTemplate.getBindingsMap()));
                    })
                    // Keep the ones that match prototypes in our class.
                    .filter(myMethods::containsKey)
                    // Add them to the mapping of overrides.
                    .forEach(parentMethod -> myMethods.get(parentMethod).add(parentMethod));
        }

        // Add all methods that we override to the set of resolved methods.
        myMethods.entrySet().stream()
                .filter(entry -> !entry.getValue().isEmpty())
                .peek(entry -> {
                    // Log override.
                    LOG.log(Level.FINE, "class {0} declares method `{1}` overriding: {2}", new Object[]{
                        getName(),
                        entry.getKey(),
                        entry.getValue()});
                })
                .map(Map.Entry::getValue)
                .flatMap(Collection::stream)
                .map(ResolvedMethod::new)
                .forEach(allResolvedMethods::add);

        // Figure out which methods need to be re-declared so that we can invoke them with the appropriate rebound types.
        final Map<MethodModel, ClassMemberModel.OverrideSelector> redeclareMethods = erasedParentModels.stream()
                .flatMap(parentTemplate -> parentTemplate.getType().redeclareMethods(ctx, parentTemplate.getBindingsMap()))
                .filter(override -> !myMethods.containsKey(override))
                .peek(override -> {
                    LOG.log(Level.FINE, "class {0} needs to redeclare `{1}` as `{2}`", new Object[]{
                        getName(),
                        override.getUnderlyingMethod().getOverrideSelector(ctx).get(),
                        override});
                })
                .collect(Collectors.toMap(ClassMemberModel.OverrideSelector::getUnderlyingMethod, Function.identity()));

        // Figure out which other methods are available on the parent.
        // Only methods that are not re-declared and not overriden are here.
        final List<ClassMemberModel.OverrideSelector> allKeptParentMethods;
        {
            final Map<ClassMemberModel.OverrideSelector, List<ClassMemberModel.OverrideSelector>> allKeptParentMethodsTmp = erasedParentModels.stream()
                    .flatMap(erasedParentTemplate -> {
                        return erasedParentTemplate.getType().getAllMethods().stream()
                                .filter(isParentResolvedMethod.negate())
                                .map(overrideSelector -> overrideSelector.rebind(erasedParentTemplate.getBindingsMap()));
                    })
                    .filter(((Predicate<ClassMemberModel.OverrideSelector>) myMethods::containsKey).negate())
                    .filter(((Predicate<ClassMemberModel.OverrideSelector>) new HashSet<>(redeclareMethods.values())::contains).negate())
                    .collect(Collectors.groupingBy(Function.identity()));
            final Map<ClassMemberModel.OverrideSelector, Map<com.github.nahratzah.jser_plus_plus.model.Type, List<ClassMemberModel.OverrideSelector>>> ambiguities = new HashMap<>();
            allKeptParentMethodsTmp.forEach((key, methods) -> {
                final Map<com.github.nahratzah.jser_plus_plus.model.Type, List<ClassMemberModel.OverrideSelector>> returnTypeMap = methods.stream()
                        .collect(Collectors.groupingBy(method -> method.getReturnType()));
                if (returnTypeMap.size() != 1)
                    ambiguities.put(key, returnTypeMap);
            });

            // Report on all ambiguities and then throw an error.
            if (!ambiguities.isEmpty()) {
                ambiguities.forEach((key, returnTypeMap) -> {
                    LOG.log(Level.SEVERE, "{0} has ambiguous methods from super type, with return types {1}: {2}", new Object[]{
                        getName(),
                        returnTypeMap.keySet(),
                        returnTypeMap.values().stream().flatMap(Collection::stream).collect(Collectors.toList())});
                });

                throw new IllegalStateException("Ambiguous methods from super types.");
            }

            allKeptParentMethods = allKeptParentMethodsTmp.values().stream().flatMap(Collection::stream).collect(Collectors.toList());
        }

        // Add redeclared methods to the classMemberFunctions list.
        postProcessingApplyRedeclare(redeclareMethods);
        // Add all my methods to the classMemberFunctions list.
        postProcessingApplyMyMethods(ctx, myMethods);

        allMethods = Stream.of(allKeptParentMethods, redeclareMethods.values(), myMethods.keySet())
                .flatMap(Collection::stream)
                .map(erasedToTemplate)
                .map(method -> new SimpleMapEntry<>(method.getDeclaringClass(), method)) // Create mapping with declaring class.
                .distinct() // Use mapping with declaring class to eliminate duplicates.
                .map(Map.Entry::getValue) // And then restore the mapping to its original.
                .collect(Collectors.toList());

        if (WRITE_POST_PROCESSING_RESULT) {
            System.out.println("========================================================");
            System.out.println(getName() + (getTemplateArgumentNames().isEmpty() ? "" : getTemplateArgumentNames().stream().collect(Collectors.joining(", ", "<", ">"))));
            System.out.println("--------------------------------------------------------");
            System.out.println("all methods:");
            allMethods.forEach(m -> System.out.println("  " + m));
            System.out.println("all suppressed methods:");
            allResolvedMethods.forEach(m -> System.out.println("  " + m.getSelector()));
            System.out.println("--------------------------------------------------------");
            System.out.println("actually emitted methods:");
            classMemberFunctions.forEach(m -> System.out.println("  " + m.getOverrideSelector(ctx).map(Object::toString).orElse(m.toString())));
            System.out.println("--------------------------------------------------------");
        }

//        throw new UnsupportedOperationException("XXX implement class post processing");
    }

    /**
     * Add all redeclare methods to {@link #classMemberFunctions}.
     *
     * @param redeclareMethods Mapping from original method to override selector
     * in current class.
     */
    private void postProcessingApplyRedeclare(Map<MethodModel, ClassMemberModel.OverrideSelector> redeclareMethods) {
        redeclareMethods.forEach((MethodModel underlying, ClassMemberModel.OverrideSelector declare) -> {
            final String body = new ST(StCtx.BUILTINS, REDECLARE_BODY_TEMPLATE)
                    .add("needCast", !Objects.equals(declare.getReturnType(), underlying.getReturnType()))
                    .add("declare", declare)
                    .add("underlying", underlying)
                    .render(Locale.ROOT, 78);

            classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                    this,
                    declare.getName(),
                    new Includes(
                            declare.getUnderlyingMethod().getDeclarationIncludes().collect(Collectors.toList()),
                            Stream.of(declare.getUnderlyingMethod().getImplementationIncludes(), underlying.getDeclarationIncludes(), Stream.of("java/_maybe_cast.h"))
                                    .flatMap(Function.identity())
                                    .collect(Collectors.toList())),
                    declare.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                    Stream.concat(declare.getUnderlyingMethod().getImplementationTypes(), underlying.getDeclarationTypes()).collect(Collectors.toSet()),
                    declare.getReturnType(),
                    declare.getArguments(),
                    declare.getUnderlyingMethod().getArgumentNames(),
                    body,
                    underlying.isStatic(),
                    false,// Accessor delegate is never virtual.
                    false,// Accessor delegate is never pure virtual.
                    false,// Accessor delegate is never override.
                    underlying.isConst(),
                    false,// Accessor delegate is never final.
                    underlying.getNoexcept(),
                    underlying.getVisibility(),
                    underlying.getDocString()));
        });
    }

    /**
     * Apply member methods to classMemberFunction list.
     *
     * @param myMethods Mapping of function, to all functions it overrides.
     */
    private void postProcessingApplyMyMethods(Context ctx, Map<ClassMemberModel.OverrideSelector, List<ClassMemberModel.OverrideSelector>> myMethods) {
        myMethods.forEach((myFn, overrideFns) -> {
            final boolean covariantReturn = myFn.getUnderlyingMethod().isCovariantReturn();

            // Ensure all methods agree on whether they use covariant return or not.
            {
                final List<ClassMemberModel.OverrideSelector> mismatchedCovariantReturnParents = overrideFns.stream()
                        .filter(fn -> fn.getUnderlyingMethod().isCovariantReturn() != covariantReturn)
                        .collect(Collectors.toList());
                if (!mismatchedCovariantReturnParents.isEmpty()) {
                    LOG.log(Level.INFO, "{0} has {1}", new Object[]{myFn, covariantReturn ? "covariant return" : "non-covariant return"});
                    mismatchedCovariantReturnParents.forEach(fn -> {
                        LOG.log(Level.SEVERE, "{0} has conflicting covariance with {1}", new Object[]{myFn, fn});
                    });
                    throw new IllegalStateException("Override method and overriden method have conflicting ideas about covariant returns.");
                }
            }

            final boolean someReturnTypesMatch = !overrideFns.isEmpty()
                    && overrideFns.stream()
                            .anyMatch(fn -> Objects.equals(fn.getReturnType(), myFn.getReturnType()));
            final boolean allReturnTypesMatch = !overrideFns.isEmpty()
                    && overrideFns.stream()
                            .allMatch(fn -> Objects.equals(fn.getReturnType(), myFn.getReturnType()));

            if (allReturnTypesMatch)
                suppressedAccessorMethods.add(myFn.getUnderlyingMethod());

            // If the method is not virtual, add it directly.
            if (!myFn.getUnderlyingMethod().isVirtual()) {
                if (overrideFns.stream()
                        .map(ClassMemberModel.OverrideSelector::getUnderlyingMethod)
                        .anyMatch(method -> method.isVirtual()))
                    LOG.log(Level.WARNING, "{0} not declared virtual, but overrides virtual methods!", myFn);
                if (!overrideFns.isEmpty() && !myFn.getUnderlyingMethod().isHideOk())
                    LOG.log(Level.WARNING, "{0} hides {1}", new Object[]{myFn, overrideFns});
                classMemberFunctions.add(myFn.getUnderlyingMethod());
                return;
            }

            // Figure out my tag type.
            final com.github.nahratzah.jser_plus_plus.model.Type myTag = new CxxType("$tagType(model)$", new Includes())
                    .prerender(ctx, singletonMap("model", this), EMPTY_LIST);

            // Figure out body of forwarders to private virtual method.
            final String forwardingBody = new ST(StCtx.BUILTINS, FORWARDING_BODY_TEMPLATE)
                    .add("model", this)
                    .add("fn", myFn)
                    .add("covariantReturn", covariantReturn)
                    .render(Locale.ROOT);

            // Figure out which super methods we actually have to replace.
            final List<ClassMemberModel.OverrideSelector> superMethods = overrideFns.stream()
                    .map(overrideFn -> new SimpleMapEntry<>(overrideFn.getDeclaringClass(), overrideFn)) // Introduce declaring class.
                    .distinct() // Eliminate duplicates, using the declaring class as support.
                    .map(Map.Entry::getValue) // Undo declaring class introduction.
                    .filter(fn -> fn.getUnderlyingMethod().isVirtual())
                    .sorted(Comparator.comparing(fn -> fn.getDeclaringType()))
                    .collect(Collectors.toList());

            if (!myFn.getUnderlyingMethod().isPureVirtual() && covariantReturn) {
                // Emit overrideFns with extra tag argument.
                superMethods.forEach(superMethod -> {
                    final ClassMemberModel.OverrideSelector overrideFn = superMethod.getErasedMethod(); // We're going to override the original function.

                    final com.github.nahratzah.jser_plus_plus.model.Type overrideTag = new CxxType("$tagType(model)$", new Includes())
                            .prerender(ctx, singletonMap("model", overrideFn.getDeclaringClass()), EMPTY_LIST);

                    classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                            this,
                            VIRTUAL_FUNCTION_PREFIX + overrideFn.getName(),
                            new Includes(
                                    overrideFn.getUnderlyingMethod().getDeclarationIncludes().collect(Collectors.toList()),
                                    Stream.concat(Stream.of("java/_maybe_cast.h"), myFn.getUnderlyingMethod().getDeclarationIncludes()).collect(Collectors.toList())),
                            overrideFn.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                            myFn.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                            overrideFn.getReturnType(),
                            Stream.concat(Stream.of(overrideTag), overrideFn.getArguments().stream()).collect(Collectors.toList()), // Prepend tag type, for tagged dispatch.
                            Stream.concat(Stream.of("_tag_"), myFn.getUnderlyingMethod().getArgumentNames().stream()).collect(Collectors.toList()), // Prepend argument name for tag type.
                            forwardingBody, // Apply forwarding rule.
                            overrideFn.getUnderlyingMethod().isStatic(),
                            overrideFn.getUnderlyingMethod().isVirtual(),
                            false, // We're not pure virtual: we implement an override, forwarding to the new override.
                            true, // We do override the base implementation.
                            overrideFn.getUnderlyingMethod().isConst(),
                            overrideFn.getUnderlyingMethod().isFinal(),
                            overrideFn.getUnderlyingMethod().getNoexcept(),
                            Visibility.PRIVATE, // Make actual virtual method private: it's only accessed via the untagged forwarding function.
                            null));
                });
            }

            if (covariantReturn && (!allReturnTypesMatch || overrideFns.size() > 1)) {
                // Emit normal forwarder for overrideFns.
                // We only emit this if the function is covariant return and either has mismatched return types, or would be ambiguous.
                // XXX overrideFns.size() > 1 does not do the correct job of figuring out ambiguity: it trips too often.
                classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                        this,
                        myFn.getName(),
                        new Includes(
                                myFn.getUnderlyingMethod().getDeclarationIncludes().collect(Collectors.toList()),
                                singletonList("java/_maybe_cast.h")),
                        myFn.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                        myFn.getUnderlyingMethod().getImplementationTypes().collect(Collectors.toSet()),
                        myFn.getReturnType(),
                        myFn.getArguments(),
                        myFn.getUnderlyingMethod().getArgumentNames(),
                        forwardingBody,
                        myFn.getUnderlyingMethod().isStatic(),
                        false, // Forwarding function is never virtual, so we can apply method hiding.
                        false, // Forwarding function is never pure virtual.
                        false, // Forwarding function is not overriding, but hiding instead.
                        myFn.getUnderlyingMethod().isConst(),
                        false, // Forwarding function is not final, because it is not virtual.
                        myFn.getUnderlyingMethod().getNoexcept(),
                        myFn.getUnderlyingMethod().getVisibility(),
                        myFn.getUnderlyingMethod().getDocString()));
            }

            // Emit actual virtual method.
            if (covariantReturn || !myFn.getUnderlyingMethod().isPureVirtual() || !allReturnTypesMatch)
                classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                        this,
                        (covariantReturn ? VIRTUAL_FUNCTION_PREFIX : "") + myFn.getName(),
                        new Includes(
                                myFn.getUnderlyingMethod().getDeclarationIncludes().collect(Collectors.toList()),
                                myFn.getUnderlyingMethod().getImplementationIncludes().collect(Collectors.toList())),
                        myFn.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                        myFn.getUnderlyingMethod().getImplementationTypes().collect(Collectors.toSet()),
                        myFn.getReturnType(),
                        Stream.concat((covariantReturn ? Stream.of(myTag) : Stream.empty()), myFn.getArguments().stream()).collect(Collectors.toList()), // Prepend tag type, for tagged dispatch, iff covariant.
                        Stream.concat((covariantReturn ? Stream.of("_tag_") : Stream.empty()), myFn.getUnderlyingMethod().getArgumentNames().stream()).collect(Collectors.toList()), // Prepend argument name for tag type, iff covariant.
                        myFn.getUnderlyingMethod().getBody(),
                        myFn.getUnderlyingMethod().isStatic(),
                        myFn.getUnderlyingMethod().isVirtual(),
                        myFn.getUnderlyingMethod().isPureVirtual(),
                        !covariantReturn && (myFn.getUnderlyingMethod().isOverride() || someReturnTypesMatch),
                        myFn.getUnderlyingMethod().isConst(),
                        myFn.getUnderlyingMethod().isFinal(),
                        myFn.getUnderlyingMethod().getNoexcept(),
                        (covariantReturn ? Visibility.PRIVATE : myFn.getUnderlyingMethod().getVisibility()), // Make actual virtual method private: it's only accessed via the untagged forwarding function.
                        myFn.getUnderlyingMethod().getDocString()));
        });
    }

    /**
     * Retrieve members which need to be re-declared due to template type
     * bindings.
     *
     * @param ctx Type lookup context.
     * @param variablesMap Template variables that this class is bound to.
     * @return Collection of
     * {@link ClassMemberModel.OverrideSelector override selectors} for methods
     * which had their arguments or return type change due to template
     * specializations.
     */
    public Stream<ClassMemberModel.OverrideSelector> redeclareMethods(Context ctx, Map<String, ? extends BoundTemplate> variablesMap) {
        final EnumSet<Visibility> visibilitySet = EnumSet.of(Visibility.PROTECTED, Visibility.PUBLIC);

        return getClassMembers().stream()
                .filter(member -> visibilitySet.contains(member.getVisibility()))
                .map(member -> member.getOverrideSelector(ctx))
                .filter(Optional::isPresent)
                .map(Optional::get)
                .map(overrideSelector -> overrideSelector.rebind(variablesMap))
                .filter(ClassMemberModel.OverrideSelector::hasAlteredTypes);
    }

    /**
     * Retrieve all inherited and local virtual methods.
     *
     * @return List of all methods in this class that can be overriden.
     */
    public synchronized List<ClassMemberModel.OverrideSelector> getAllMethods() {
        if (!postProcessingDone)
            throw new IllegalStateException("Must have completed post processing stage.");

        return allMethods;
    }

    /**
     * Documentation of the class.
     *
     * @return String containing class documentation.
     */
    public String getDocString() {
        return docString;
    }

    /**
     * Test if this class has development mode enabled.
     *
     * If development mode is enabled, certain compile checks in the C++ code
     * are turned off.
     *
     * @return True if development mode is enabled, false if disabled.
     */
    public boolean isDevMode() {
        return devMode;
    }

    /**
     * Retrieve the {@link ClassConfig.CfgSuperType CfgSuperType} for this type.
     *
     * @return Instance of {@link ClassConfig.CfgSuperType CfgSuperType} backed
     * by this {@link ClassType ClassType}.
     */
    private ClassConfig.CfgSuperType cfgType() {
        return new CfgType();
    }

    /**
     * Build a simple identity map for the given type variables.
     *
     * @param vars List of type variables.
     * @return Identity mapping of java generics name to config name.
     */
    private static Map<String, String> buildRenameMap(List<? extends TypeVariable> vars) {
        return vars.stream()
                .map(TypeVariable::getName)
                .collect(Collectors.toMap(Function.identity(), Function.identity()));
    }

    /**
     * Build a mapping from vars to cfgVars.
     *
     * @param vars List of type variables.
     * @param cfgVars List of variable names to map to.
     * @return Mapping of java generics name to config name.
     */
    private static Map<String, String> buildRenameMap(List<? extends TypeVariable> vars, List<? extends String> cfgVars) {
        final Map<String, String> renameMap = new HashMap<>();

        if (vars.size() != vars.stream()
                .map(TypeVariable::getName)
                .distinct()
                .count())
            throw new IllegalArgumentException("Duplicate generics variable in Java type.");
        if (cfgVars.size() != cfgVars.stream()
                .distinct()
                .count())
            throw new IllegalArgumentException("Duplicate generics variable in configured type.");

        final Iterator<? extends TypeVariable> varIter = vars.iterator();
        final Iterator<? extends String> cfgIter = cfgVars.iterator();
        while (varIter.hasNext() && cfgIter.hasNext()) {
            renameMap.put(varIter.next().getName(), cfgIter.next());
        }

        if (varIter.hasNext() || cfgIter.hasNext())
            throw new IllegalArgumentException("Number of java generics does not match number of configured generics.");
        return renameMap;
    }

    /**
     * Type visitor that figures out bounds of template arguments.
     */
    private static class BoundsMapping implements ReflectUtil.Visitor<BoundTemplate> {
        public BoundsMapping(Context ctx, Map<? super String, ? extends String> argRename) {
            this.ctx = requireNonNull(ctx);
            this.argRename = requireNonNull(argRename);
        }

        @Override
        public BoundTemplate apply(Class<?> var) {
            if (var.isArray())
                return new BoundTemplate.ArrayBinding(apply(var.getComponentType()), 1);

            final JavaType cVar = ctx.resolveClass(var);
            final List<BoundTemplate> cArgs = Stream.generate(BoundTemplate.Any::new)
                    .limit(cVar.getNumTemplateArguments())
                    .collect(Collectors.toList());
            return new BoundTemplate.ClassBinding<>(cVar, cArgs);
        }

        @Override
        public BoundTemplate apply(TypeVariable<?> var) {
            return new BoundTemplate.VarBinding(rename(var.getName()));
        }

        @Override
        public BoundTemplate apply(GenericArrayType var) {
            return new BoundTemplate.ArrayBinding(ReflectUtil.visitType(var.getGenericComponentType(), this), 1);
        }

        @Override
        public BoundTemplate apply(WildcardType var) {
            final List<BoundTemplate> extendTypes = Arrays.stream(var.getUpperBounds())
                    .map(t -> ReflectUtil.visitType(t, this))
                    .collect(Collectors.toList());
            final List<BoundTemplate> superTypes = Arrays.stream(var.getLowerBounds())
                    .map(t -> ReflectUtil.visitType(t, this))
                    .collect(Collectors.toList());
            return new BoundTemplate.Any(superTypes, extendTypes);
        }

        @Override
        public BoundTemplate apply(ParameterizedType var) {
            final List<BoundTemplate> cArgs = Arrays.stream(var.getActualTypeArguments())
                    .map(x -> ReflectUtil.visitType(x, this))
                    .collect(Collectors.toList());

            return ReflectUtil.visitType(var.getRawType(), new ReflectUtil.Visitor<BoundTemplate>() {
                @Override
                public BoundTemplate apply(Class<?> var) {
                    if (var.isArray())
                        return new BoundTemplate.ArrayBinding(apply(var.getComponentType()), 1);

                    return new BoundTemplate.ClassBinding<>(ctx.resolveClass(var), cArgs);
                }

                @Override
                public BoundTemplate apply(TypeVariable<?> var) {
                    throw new IllegalStateException("Template type can not be a generics variable.");
                }

                @Override
                public BoundTemplate apply(GenericArrayType var) {
                    return new BoundTemplate.ArrayBinding(ReflectUtil.visitType(var.getGenericComponentType(), this), 1);
                }

                @Override
                public BoundTemplate apply(WildcardType var) {
                    throw new IllegalStateException("Template type can not be a generics wildcard.");
                }

                @Override
                public BoundTemplate apply(ParameterizedType var) {
                    throw new IllegalStateException("Template type can not be a template specialization.");
                }
            });
        }

        private String rename(String arg) {
            final String renamedArg = argRename.get(arg);
            if (renamedArg != null) return renamedArg;

            final String args = argRename.keySet().stream()
                    .map(Object::toString)
                    .sorted()
                    .collect(Collectors.joining(", "));
            throw new IllegalStateException("Generics argument \"" + arg + "\" referenced, but is not in set <" + args + ">");
        }

        private final Context ctx;
        private final Map<? super String, ? extends String> argRename;
    }

    /**
     * Type visitor that resolves a parent type to a bound template.
     */
    private static class ParentTypeVisitor implements ReflectUtil.Visitor<BoundTemplate.ClassBinding<ClassType>> {
        public ParentTypeVisitor(Context ctx, Map<? super String, ? extends String> argRename) {
            this.ctx = requireNonNull(ctx);
            this.argRename = requireNonNull(argRename);
        }

        @Override
        public BoundTemplate.ClassBinding<ClassType> apply(Class<?> var) {
            if (var.isArray())
                throw new IllegalStateException("Parent type can not be an array.");

            return new BoundTemplate.ClassBinding<>(ctx.resolveClass(var), EMPTY_LIST);
        }

        @Override
        public BoundTemplate.ClassBinding<ClassType> apply(TypeVariable<?> var) {
            throw new IllegalStateException("Parent type can not be a generics variable.");
        }

        @Override
        public BoundTemplate.ClassBinding<ClassType> apply(GenericArrayType var) {
            throw new IllegalStateException("Parent type can not be an array.");
        }

        @Override
        public BoundTemplate.ClassBinding<ClassType> apply(WildcardType var) {
            throw new IllegalStateException("Parent type can not be a wildcard.");
        }

        @Override
        public BoundTemplate.ClassBinding<ClassType> apply(ParameterizedType var) {
            final BoundsMapping boundsMapping = new BoundsMapping(ctx, argRename);

            final List<BoundTemplate> cArgs = Arrays.stream(var.getActualTypeArguments())
                    .map(x -> ReflectUtil.visitType(x, boundsMapping))
                    .collect(Collectors.toList());

            return ReflectUtil.visitType(var.getRawType(), new ReflectUtil.Visitor<BoundTemplate.ClassBinding<ClassType>>() {
                @Override
                public BoundTemplate.ClassBinding<ClassType> apply(Class<?> var) {
                    final JavaType resolvedType = ctx.resolveClass(var);
                    try {
                        return new BoundTemplate.ClassBinding<>((ClassType) resolvedType, cArgs);
                    } catch (ClassCastException ex) {
                        throw new IllegalStateException("Class inherits from non-class type " + resolvedType);
                    }
                }

                @Override
                public BoundTemplate.ClassBinding<ClassType> apply(TypeVariable<?> var) {
                    throw new IllegalStateException("Type variable is not a valid parent class.");
                }

                @Override
                public BoundTemplate.ClassBinding<ClassType> apply(GenericArrayType var) {
                    throw new IllegalStateException("Array is not a valid parent class.");
                }

                @Override
                public BoundTemplate.ClassBinding<ClassType> apply(WildcardType var) {
                    throw new IllegalStateException("Wildcard is not a valid parent class.");
                }

                @Override
                public BoundTemplate.ClassBinding<ClassType> apply(ParameterizedType var) {
                    throw new IllegalStateException("Bound template type is not a valid base for a template.");
                }
            });
        }

        private final Context ctx;
        private final Map<? super String, ? extends String> argRename;
    }

    /**
     * Models a method that has been overriden and replaced by a more-derived
     * class or interface.
     *
     * This is used to suppress the same method, when diamond inheritance would
     * otherwise re-introduce it.
     */
    private static class ResolvedMethod {
        public ResolvedMethod(ClassMemberModel.OverrideSelector selector) {
            requireNonNull(selector);
            this.selector = requireNonNull(selector.getErasedMethod());
            this.declaringClass = requireNonNull(selector.getDeclaringClass());
        }

        /**
         * Retrieve the class on which this method appeared.
         *
         * @return The class declaring this method.
         */
        public ClassType getDeclaringClass() {
            return declaringClass;
        }

        /**
         * Get the type-erased selector of the method.
         *
         * The erasure is based on the declaring class, only.
         *
         * @return The method selector.
         */
        public ClassMemberModel.OverrideSelector getSelector() {
            return selector;
        }

        @Override
        public int hashCode() {
            int hash = 5;
            hash = 71 * hash + Objects.hashCode(this.declaringClass);
            hash = 71 * hash + Objects.hashCode(this.selector);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final ResolvedMethod other = (ResolvedMethod) obj;
            if (!Objects.equals(this.declaringClass, other.declaringClass))
                return false;
            if (!Objects.equals(this.selector, other.selector)) return false;
            return true;
        }

        private final ClassType declaringClass;
        private final ClassMemberModel.OverrideSelector selector;
    }

    private class CfgType implements ClassConfig.CfgSuperType {
        @Override
        public String getName() {
            return ClassType.this.getName();
        }

        @Override
        public Optional<ClassConfig.CfgSuperType> getSuperClass() {
            return Optional.ofNullable(ClassType.this.getSuperClass())
                    .map(BoundTemplate.ClassBinding::getType)
                    .map(ClassType::cfgType);
        }

        @Override
        public List<ClassConfig.CfgSuperType> getInterfaces() {
            return ClassType.this.getInterfaces().stream()
                    .map(BoundTemplate.ClassBinding::getType)
                    .map(ClassType::cfgType)
                    .collect(Collectors.toList());
        }

        private ClassType classType() {
            return ClassType.this;
        }

        @Override
        public int hashCode() {
            return classType().hashCode();
        }

        @Override
        public boolean equals(Object other) {
            if (other == null) return false;
            if (!Objects.equals(getClass(), other.getClass())) return false;
            return classType() == ((CfgType) other).classType();
        }
    }

    /**
     * Underlying java class.
     */
    protected final Class<?> c;
    /**
     * Template arguments for this class.
     */
    private List<ClassTemplateArgument> templateArguments;
    /**
     * Super type of this class. May be null, in which case this type has no
     * super class.
     */
    private BoundTemplate.ClassBinding<? extends ClassType> superType;
    /**
     * List of interfaces used by this class.
     */
    private List<BoundTemplate.ClassBinding<? extends ClassType>> interfaceTypes;
    /**
     * The serial version UID of the class.
     */
    private long serialVersionUID = 0;
    /**
     * List of fields.
     */
    protected List<FieldType> fields;
    /**
     * If the class is a final class.
     */
    private boolean finalVar;
    /**
     * Class members for the class.
     */
    protected Collection<ClassMemberModel> classMembers;
    /**
     * Default variable type for a class.
     *
     * If null, then use this.
     */
    private com.github.nahratzah.jser_plus_plus.model.Type varType;
    /**
     * List of friend types.
     */
    private List<com.github.nahratzah.jser_plus_plus.model.Type> friends;
    /**
     * Marker to prevent us from post processing twice.
     */
    private boolean postProcessingDone = false;
    /**
     * All methods of this class.
     *
     * Filled in by
     * {@link #postProcess(com.github.nahratzah.jser_plus_plus.input.Context) post processing}
     * logic.
     */
    private List<ClassMemberModel.OverrideSelector> allMethods;
    /**
     * All methods that have been resolved by this class. Those methods are
     * inherited from the {@link #getSuperClass() super class} and
     * {@link #getInterfaces() interfaces}, but have been overriden.
     *
     * Filled in by
     * {@link #postProcess(com.github.nahratzah.jser_plus_plus.input.Context) post processing}
     * logic.
     */
    private Set<ResolvedMethod> allResolvedMethods;
    /**
     * List of member functions for the class.
     *
     * Filled in by
     * {@link #postProcess(com.github.nahratzah.jser_plus_plus.input.Context) post processing}
     * logic.
     */
    private List<MethodModel> classMemberFunctions = new ArrayList<>();
    /**
     * Documentation of the class.
     */
    private String docString;
    /**
     * Flag indicating if development mode is enabled.
     *
     * In development mode, certain code checks are disabled to allow for most
     * things to compile.
     */
    private boolean devMode = false;
    /**
     * List of methods which, due to overriding from base and not changing
     * signature, do not require redeclaration.
     */
    private final Set<MethodModel> suppressedAccessorMethods = new HashSet<>();
}
