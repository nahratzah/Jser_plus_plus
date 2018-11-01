package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.CfgField;
import com.github.nahratzah.jser_plus_plus.config.CfgType;
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
import com.github.nahratzah.jser_plus_plus.misc.MethodModelComparator;
import com.github.nahratzah.jser_plus_plus.misc.SimpleMapEntry;
import static com.github.nahratzah.jser_plus_plus.model.JavaType.getAllTypeParameters;
import static com.github.nahratzah.jser_plus_plus.model.Type.typeFromCfgType;
import com.github.nahratzah.jser_plus_plus.model.impl.AccessorConstructor;
import com.github.nahratzah.jser_plus_plus.model.impl.AccessorMethod;
import com.github.nahratzah.jser_plus_plus.model.impl.ClassDelegateMethod;
import com.github.nahratzah.jser_plus_plus.model.impl.ClassVirtualDelegateMethod;
import com.github.nahratzah.jser_plus_plus.output.builtins.ConstTypeRenderer;
import com.github.nahratzah.jser_plus_plus.output.builtins.FunctionAttrMap;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Collections2;
import com.google.common.collect.Maps;
import com.google.common.collect.MoreCollectors;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamClass;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singletonList;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableCollection;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableMap;
import java.util.Comparator;
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
import java.util.TreeSet;
import java.util.function.BiConsumer;
import java.util.function.BiPredicate;
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
    public static final String VIRTUAL_FUNCTION_PREFIX = "_virtual_";
    private static final String REDECLARE_BODY_TEMPLATE = "return "
            + "$if (needCast)$::java::cast<$boundTemplateType(declare.returnType, \"style=type, class\")$>($endif$"
            + "this->$erasedType(underlying.underlyingMethod.declaringClass)$::$underlying.name$"
            + "($declare.underlyingMethod.argumentNames: { name | ::std::forward<decltype($name$)>($name$)}; anchor, wrap, separator = \", \"$)"
            + "$if (needCast)$)$endif$;";
    private static final String FORWARDING_BODY_TEMPLATE = "return "
            + "this->"
            + "$if (covariantReturn)$" + VIRTUAL_FUNCTION_PREFIX + "$endif$"
            + "$fn.name$"
            + "("
            + "$if (covariantReturn)$$tagType(tagModel)$()$endif$$if (covariantReturn && fn.underlyingMethod.argumentNames)$, $endif$"
            + "$fn.underlyingMethod.argumentNames: { name | ::java::_maybe_cast(::std::forward<decltype($name$)>($name$))}; anchor, wrap, separator = \", \"$"
            + ")"
            + ";";

    public ClassType(Class<?> c) {
        this.c = requireNonNull(c);
    }

    /**
     * Retrieve the accessor for this type.
     *
     * @return The accessor for this type.
     */
    public Accessor getAccessor() {
        return accessor;
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

        // Figure out if the class has a write method.
        try {
            final java.lang.reflect.Method method = this.c.getDeclaredMethod("writeObject", new Class<?>[]{ObjectOutputStream.class});
            final int mods = method.getModifiers();
            this.writeMethod = (method.getReturnType() == Void.TYPE)
                    && ((mods & Modifier.STATIC) == 0)
                    && ((mods & Modifier.PRIVATE) != 0);
        } catch (NoSuchMethodException ex) {
            this.writeMethod = false;
        }

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
            this.varType = typeFromCfgType(classCfg.getVarType(), ctx, singletonMap("model", this), argRename.values(), getBoundType());
        }

        this.docString = classCfg.getDocString();
        this.devMode = classCfg.isDevMode();
        this.srcRaw = classCfg.getSrcRaw();
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
            public IntermediateFieldDescr(String name, Class<?> classType, java.lang.reflect.Type reflectType, Field reflectField, boolean shared) {
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
            public final java.lang.reflect.Type reflectType;
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
                    java.lang.reflect.Type reflectType;
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
                    final java.lang.reflect.Type visitType = iField.reflectType != null ? iField.reflectType : iField.classType;
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
        Map<String, FieldType> fieldMapping = Stream.concat(reflectedFields.stream(), syntheticFields.stream())
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
                        iField.setType(typeFromCfgType(fieldCfg.getType(), ctx, singletonMap("model", this), argRename.values(), getBoundType()));
                    if (fieldCfg.getDefault() != null) {
                        // Render the default initializer.
                        final Map<String, BoundTemplate.VarBinding> variablesMap = getTemplateArgumentNames().stream()
                                .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));
                        final Collection<Type> newDeclTypes = new HashSet<>(); // XXX use
                        iField.setDefault(new ST(StCtx.contextGroup(ctx, variablesMap, getBoundType(), newDeclTypes::add), fieldCfg.getDefault())
                                .add("model", this)
                                .render(Locale.ROOT));
                    }
                    if (fieldCfg.getRename() != null)
                        iField.setName(fieldCfg.getRename());
                    iField.setCompleteInit(fieldCfg.isCompleteInit());
                    if (fieldCfg.getDecodeStage() != null)
                        iField.setDecodeStage(fieldCfg.getDecodeStage());
                    iField.setOmit(fieldCfg.isOmit());
                    if (fieldCfg.getEncoderExpr() != null)
                        iField.setEncoderExpr(fieldCfg.getEncoderExpr());
                })
                .collect(Collectors.toMap(
                        FieldType::getSerializationName,
                        Function.identity()));
        this.fields = Stream
                .concat(
                        classCfg.getFields().keySet().stream()
                                .map(fieldMapping::get),
                        fieldMapping.entrySet().stream()
                                .filter(entry -> !classCfg.getFields().containsKey(entry.getKey()))
                                .sorted(Comparator.comparing(Map.Entry::getKey))
                                .map(Map.Entry::getValue))
                .peek(Objects::requireNonNull)
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

        if (classCfg.isInheritConstructors()) {
            if (getSuperClass() == null) {
                LOG.log(Level.WARNING, "inheriting constructor, but no super class present");
            } else {
                final BoundTemplate.ClassBinding<? extends ClassType> superClass = getSuperClass();
                superClass.getType().classMembers.stream()
                        .filter(ClassMemberModel.ClassConstructor.class::isInstance)
                        .map(ClassMemberModel.ClassConstructor.class::cast)
                        .filter(ClassMemberModel.ClassConstructor::isInheritable)
                        .filter(constructor -> constructor.getVisibility() != Visibility.PRIVATE)
                        .map(constructor -> new ClassMemberModel.ClassConstructor(ctx, this, constructor))
                        .peek(constructor -> LOG.log(Level.FINE, "Adding {0} with name {1}", new Object[]{constructor, getName()}))
                        .forEach(classMembers::add);
            }
        }
    }

    private void initSerializationConstructor(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        final Predicate<FieldType> isPrimitive = field -> field.getSerializationType().isPrimitive() && !field.isSerializationArray();

        final ClassType superClass = (getSuperClass() == null ? null : getSuperClass().getType());
        final boolean superClassIsSerializable = superClass != null
                && superClass.findAllInherits()
                        .anyMatch(type -> ctx.resolveClass(java.io.Serializable.class.getName()) == type);

        // Build `__decoder__` argument.
        CfgType ctxType = new CfgType("const ::java::serialization::class_decoder<$tagType(model)$>&", null);
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
        classMembers.add(new ClassMemberModel.ClassConstructor(ctx, this, constructorCfg, false));
    }

    private void initFriendTypes(Context ctx, ClassConfig classCfg, Map<String, String> argRename) {
        this.friends = classCfg.getFriends().stream()
                .map(cfgType -> typeFromCfgType(cfgType, ctx, singletonMap("model", this), argRename.values(), getBoundType()))
                .distinct()
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
                    getFunctions().stream()
                            .flatMap(member -> {
                                final Stream<String> implStream;
                                if (member.getInline() == null)
                                    implStream = Stream.empty();
                                else
                                    implStream = member.getImplementationIncludes();
                                return Stream.concat(implStream, member.getDeclarationIncludes());
                            }),
                    getNonFunctions().stream()
                            .flatMap(member -> {
                                final Stream<String> implStream;
                                if (member.getInline() == null)
                                    implStream = Stream.empty();
                                else
                                    implStream = member.getImplementationIncludes();
                                return Stream.concat(implStream, member.getDeclarationIncludes());
                            }));
            friendIncludes = friends.stream()
                    .flatMap(friend -> friend.getIncludes(true));
        } else {
            fieldIncludes = getFields().stream()
                    .flatMap(field -> Stream.of(field.getType(), field.getVarType()))
                    .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));
            memberIncludes = Stream.concat(
                    getFunctions().stream()
                            .flatMap(member -> member.getImplementationIncludes()),
                    getNonFunctions().stream()
                            .flatMap(member -> member.getImplementationIncludes()));
            friendIncludes = friends.stream()
                    .flatMap(friend -> friend.getIncludes(false));
        }

        return Stream.of(parentTypes, fieldIncludes, memberIncludes, friendIncludes)
                .flatMap(Function.identity());
    }

    @Override
    public Stream<JavaType> getForwardDeclarationJavaTypes() {
        final Stream<Type> superTypes = Stream.concat(
                Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());
        final Stream<Type> publicFields = getFields().stream()
                .filter(field -> field.isGetterFn() || field.isSetterFn())
                .map(field -> field.getVarType());
        final Stream<Type> memberTypes = getClassMembers().stream()
                .filter(member -> member.getVisibility() == Visibility.PUBLIC)
                .flatMap(member -> member.getDeclarationTypes());

        return Stream.of(superTypes, publicFields, memberTypes)
                .flatMap(Function.identity())
                .flatMap(Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getDeclarationCompleteJavaTypes() {
        final Stream<Type> superTypes = Stream.concat(
                Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());

        return Stream.of(superTypes)
                .flatMap(Function.identity())
                .flatMap(Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getDeclarationForwardJavaTypes() {
        final Stream<Type> templateTypes = getTemplateArguments().stream()
                .map(ClassTemplateArgument::getExtendBounds)
                .flatMap(Collection::stream);
        final Stream<Type> fieldTypes = getFields().stream()
                .map(field -> field.getVarType());
        final Stream<Type> memberTypes = Stream.concat(
                getFunctions().stream()
                        .flatMap(member -> member.getDeclarationTypes()),
                getNonFunctions().stream()
                        .flatMap(member -> member.getDeclarationTypes()));

        return Stream.of(templateTypes, fieldTypes, memberTypes)
                .flatMap(Function.identity())
                .flatMap(Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    @Override
    public Stream<JavaType> getImplementationJavaTypes() {
        final Stream<Type> superTypes = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull),
                getInterfaces().stream());
        final Stream<Type> fieldTypes = getFields().stream()
                .flatMap(field -> Stream.of(field.getType(), field.getVarType()));
        final Stream<Type> memberTypes = Stream.concat(
                getFunctions().stream()
                        .flatMap(member -> member.getImplementationTypes()),
                getNonFunctions().stream()
                        .flatMap(member -> member.getImplementationTypes()));

        return Stream.of(superTypes, fieldTypes, memberTypes)
                .flatMap(Function.identity())
                .flatMap(Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    public long getSerialVersionUID() {
        return serialVersionUID;
    }

    public boolean isWriteMethod() {
        return writeMethod;
    }

    public String getSerialVersionUID_string() {
        return String.format("%#016xULL", getSerialVersionUID());
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

    /**
     * Retrieve the fields that have encoder for serialization enabled.
     *
     * @return List of fields for which serialization is enabled.
     */
    public List<FieldType> getEncoderSerializationFields() {
        return fields.stream()
                .filter(f -> f.isEncodeEnabled())
                .filter(f -> !f.isOmit() || f.getEncoderExpr() != null)
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

    public List<MethodModel> getFunctions() {
        return classMemberFunctions.stream()
                .collect(Collectors.toList());
    }

    public Collection<ClassMemberModel> getNonFunctions() {
        return Collections2.filter(classMembers, classMember -> !(classMember instanceof ClassMemberModel.ClassMethod));
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
     * Get a version of this type with all template arguments bound to their own
     * variables.
     *
     * @return A bound type for this class.
     */
    public BoundTemplate.ClassBinding<ClassType> getBoundType() {
        return new BoundTemplate.ClassBinding<>(
                this,
                getTemplateArgumentNames().stream().map(BoundTemplate.VarBinding::new).collect(Collectors.toList()));
    }

    /**
     * Get a version of this type with all template arguments bound to the
     * wildcard generic.
     *
     * @return A bound type for this class.
     */
    public Type getPackType() {
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
    public Type getConstPackType() {
        return new ConstType(getPackType());
    }

    @Override
    public Type getVarType() {
        return varType;
    }

    public List<Type> getFriends() {
        return friends;
    }

    @Override
    public synchronized void postProcess(Context ctx) {
        if (postProcessingDone) return;
        postProcessingDone = true;

        final Map<OverrideSelector, Collection<ImplementedClassMethod>> virtualClassMembersWithParentMethods, nonVirtualClassMembersWithParentMethods;
        Collection<ImplementedClassMethod> keptParentMethodsWithChangedTypes;
        final Collection<ImplementedClassMethod> keptParentMethodsWithoutChangedTypes;

        {
            final List<ImplementedClassMethod> parentMethods = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull), getInterfaces().stream())
                    .flatMap(parentType -> {
                        final ClassType parentClass = parentType.getType();
                        parentClass.postProcess(ctx); // Ensure post processing of parent class has completed.
                        return parentClass.implementedMethods.stream()
                                .map(implementedMethod -> implementedMethod.rebind(parentType.getBindingsMap()));
                    })
                    .collect(Collectors.toList());
            LOG.log(Level.FINE, () -> {
                if (parentMethods.isEmpty())
                    return getName() + " has no parent methods";
                return parentMethods.stream()
                        .map(parentMethod -> getName() + " parent method: " + parentMethod)
                        .collect(Collectors.joining());
            });

            final Map<String, BoundTemplate> erasedClassBindings = getErasedTemplateArguments().stream()
                    .collect(Collectors.collectingAndThen(
                            Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue),
                            Collections::unmodifiableMap));

            final Map<OverrideSelector, Collection<ImplementedClassMethod>> classMembersWithParentMethods = classMembers.stream()
                    .peek(member -> LOG.log(Level.FINE, "{0}: examining {1}", new Object[]{getName(), member}))
                    .filter(ClassMemberModel.ClassMethod.class::isInstance)
                    .map(ClassMemberModel.ClassMethod.class::cast)
                    .filter(classMember -> !classMember.isStatic())
                    .map(classMember -> classMember.getOverrideSelector(ctx)) // This optional does not exist for static methods!
                    .filter(Optional::isPresent)
                    .map(Optional::get)
                    .collect(Collectors.collectingAndThen(
                            Collectors.toMap(
                                    Function.identity(),
                                    classMemberSelector -> {
                                        return parentMethods.stream()
                                                .filter(parentMethod -> {
                                                    final boolean same = Objects.equals(parentMethod.getSelector().rebind(erasedClassBindings), classMemberSelector.rebind(erasedClassBindings));
                                                    if (Objects.equals(parentMethod.getSelector().getName(), classMemberSelector.getName()))
                                                        LOG.log(Level.FINE, "{0}:\n  {1}\n  {2}\n  have {3} signature", new Object[]{getName(), parentMethod, classMemberSelector, same ? "the same" : "a different"});
                                                    return same;
                                                })
                                                .collect(Collectors.collectingAndThen(Collectors.toList(), Collections::unmodifiableCollection));
                                    },
                                    (x, y) -> {
                                        LOG.log(Level.SEVERE, "{0}: method is declared twice\n  {1}\n  {2}", new Object[]{getName(), x, y});
                                        throw new IllegalStateException("duplicate key");
                                    }),
                            Collections::unmodifiableMap));

            final Predicate<OverrideSelector> isVirtualPredicate = selector -> selector.getUnderlyingMethod().isVirtual();
            virtualClassMembersWithParentMethods = Maps.filterKeys(classMembersWithParentMethods, isVirtualPredicate::test);
            nonVirtualClassMembersWithParentMethods = Maps.filterKeys(classMembersWithParentMethods, isVirtualPredicate.negate()::test);

            final Set<ImplementedClassMethod> classMembersWithParentMethodsValues = classMembersWithParentMethods.values().stream()
                    .flatMap(Collection::stream)
                    .collect(Collectors.toSet());
            final Collection<ImplementedClassMethod> keptParentMethods = Collections2.filter(
                    parentMethods,
                    parentMethod -> !classMembersWithParentMethodsValues.contains(parentMethod));
            final Predicate<ImplementedClassMethod> implementedMethodHasChangedTypes = method -> method.hasChangedTypes(this);
            keptParentMethodsWithChangedTypes = Collections2.filter(
                    keptParentMethods,
                    implementedMethodHasChangedTypes::test);
            keptParentMethodsWithoutChangedTypes = Collections2.filter(
                    keptParentMethods,
                    implementedMethodHasChangedTypes.negate()::test);
        }

        final List<ImplementedClassMethod> nonVirtualClassMembers = postProcessNonVirtualClassMembers(nonVirtualClassMembersWithParentMethods);
        final List<ImplementedClassMethod> virtualClassMembers = postProcessVirtualClassMembers(virtualClassMembersWithParentMethods);

        // Apply type changes for parent methods.
        keptParentMethodsWithChangedTypes = postProcessCreateAccessorsForChangedTypes(keptParentMethodsWithChangedTypes, ctx);
        // Implement virtual functions.
        postProcessEmitVirtualClassMembers(virtualClassMembers, ctx);
        // Implement non-virtual functions.
        postProcessEmitNonVirtualClassMembers(nonVirtualClassMembers);
        // Implement all constructors.
        postProcessEmitClassConstructors(Collections2.transform(Collections2.filter(classMembers, ClassMemberModel.ClassConstructor.class::isInstance), ClassMemberModel.ClassConstructor.class::cast));
        // Implement all static functions.
        postProcessEmitStaticClassMembers(Collections2.transform(Collections2.filter(classMembers, ClassMemberModel::isStatic), ClassMemberModel.ClassMethod.class::cast));

        // Record all methods we implement.
        implementedMethods = Stream.of(keptParentMethodsWithChangedTypes, keptParentMethodsWithoutChangedTypes, nonVirtualClassMembers, virtualClassMembers)
                .flatMap(Collection::stream)
                .collect(Collectors.toSet());

        // Print out a friendly message on what we did.
        if (WRITE_POST_PROCESSING_RESULT) {
            System.out.println("========================================================");
            System.out.println(getName() + (getTemplateArgumentNames().isEmpty() ? "" : getTemplateArgumentNames().stream().collect(Collectors.joining(", ", "<", ">"))));
            System.out.println("--------------------------------------------------------");
            System.out.println("all methods:");
            implementedMethods.forEach(m -> System.out.println("  " + m.getSelector()));
            System.out.println("--------------------------------------------------------");
            System.out.println("actually emitted methods:");
            classMemberFunctions.forEach(m -> System.out.println("  " + m.getOverrideSelector(ctx).map(Object::toString).orElseGet(() -> m.toString())));
            System.out.println("--------------------------------------------------------");
        }
    }

    /**
     * Create accessor methods for all methods which had a type change.
     *
     * @param keptParentMethodsWithChangedTypes List of all methods with a type
     * change.
     * @return Newly declared implemented methods.
     */
    private List<ImplementedClassMethod> postProcessCreateAccessorsForChangedTypes(Collection<ImplementedClassMethod> keptParentMethodsWithChangedTypes, Context ctx) {
        final List<ImplementedClassMethod> result = new ArrayList<>();
        final Map<String, BoundTemplate> erasureBindingMap = getErasedTemplateArguments().stream()
                .collect(Collectors.collectingAndThen(
                        Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue),
                        Collections::unmodifiableMap));

        keptParentMethodsWithChangedTypes.forEach(method -> {
            final OverrideSelector declare = method.getSelector()
                    .rebind(erasureBindingMap);
            final OverrideSelector underlying = method.getErasedSelector();

            if (declare.getUnderlyingMethod().isVirtual() && !declare.getUnderlyingMethod().isCovariantReturn())
                throw new IllegalStateException("Non-covariant virtual function " + declare + " changed type");
            // XXX
            if (declare.getUnderlyingMethod().isVirtual()) {
                result.add(method);
                return;
            }

            final String body = new ST(StCtx.BUILTINS, REDECLARE_BODY_TEMPLATE)
                    .add("needCast", !Objects.equals(declare.getReturnType(), underlying.getReturnType()))
                    .add("declare", declare)
                    .add("underlying", underlying)
                    .render(Locale.ROOT, 78);

            final MethodModel.SimpleMethodModel newMethod = new MethodModel.SimpleMethodModel(
                    this,
                    declare.getDeclaringType(),
                    declare.getName(),
                    new Includes(
                            Stream.concat(Stream.of("java/inline.h"), declare.getUnderlyingMethod().getDeclarationIncludes())
                                    .collect(Collectors.toList()),
                            Stream.of(declare.getUnderlyingMethod().getImplementationIncludes(), underlying.getUnderlyingMethod().getDeclarationIncludes(), Stream.of("java/_maybe_cast.h"))
                                    .flatMap(Function.identity())
                                    .collect(Collectors.toList())),
                    declare.getUnderlyingMethod().getDeclarationTypes().collect(Collectors.toSet()),
                    Stream.concat(declare.getUnderlyingMethod().getImplementationTypes(), underlying.getUnderlyingMethod().getDeclarationTypes()).collect(Collectors.toSet()),
                    declare.getReturnType(),
                    declare.getArguments(),
                    declare.getUnderlyingMethod().getArgumentNames(),
                    body,
                    underlying.getUnderlyingMethod().isStatic(),
                    false,// delegate is never virtual.
                    false,// delegate is never pure virtual.
                    false,// delegate is never override.
                    underlying.isConst(),
                    false,// delegate is never final.
                    "JSER_INLINE", // Delegate method is inline.
                    underlying.getUnderlyingMethod().getNoexcept(),
                    underlying.getUnderlyingMethod().getVisibility(),
                    underlying.getUnderlyingMethod().getDocString());

            result.add(new ImplementedClassMethod(
                    this,
                    null,
                    newMethod.getOverrideSelector(ctx).orElseThrow(() -> new IllegalStateException("synthesized method did not create override selector")),
                    EMPTY_LIST));

            // We only add this to the
            // {@link #classMemberFunctions class member functions}.
            // We omit the accessor counterpart, since the original accessor
            // will already contain the correct template to access this.
            classMemberFunctions.add(newMethod);
        });

        return result;
    }

    /**
     * Each virtual method must keep track of the methods it overrides.
     *
     * Some diagnostic warnings may be generated.
     *
     * This method also picks which tag to reuse, to keep the number of distinct
     * virtual functions in check.
     *
     * @param virtualClassMembersWithParentMethods All virtual methods that are
     * declared for this class, with the methods they'll hide or override.
     * @return {@link ImplementedClassMethod implemented method} for each of the
     * virtual methods.
     */
    private List<ImplementedClassMethod> postProcessVirtualClassMembers(Map<OverrideSelector, Collection<ImplementedClassMethod>> virtualClassMembersWithParentMethods) {
        final List<ImplementedClassMethod> result = new ArrayList<>();

        final Predicate<ImplementedClassMethod> isVirtualPredicate = method -> method.getSelector().getUnderlyingMethod().isVirtual();
        final BiPredicate<OverrideSelector, ImplementedClassMethod> sameReturnType = (selector, method) -> Objects.equals(selector.getErasedMethod(this).getReturnType(), method.getErasedSelector().getReturnType());

        virtualClassMembersWithParentMethods.forEach((selector, overridenMethods) -> {
            LOG.log(Level.FINE, "{0}: processing virtual class method {1}", new Object[]{getName(), selector});

            final Predicate<ImplementedClassMethod> validationCovariantMismatch = method -> {
                return selector.getUnderlyingMethod().isCovariantReturn()
                        != method.getErasedSelector().getUnderlyingMethod().isCovariantReturn();
            };
            final Predicate<ImplementedClassMethod> validationReturnMismatch = method -> {
                return !selector.getUnderlyingMethod().isCovariantReturn()
                        && !sameReturnType.test(selector, method);
            };

            final Collection<ImplementedClassMethod> virtualOverridenMethods = Collections2.filter(
                    overridenMethods,
                    isVirtualPredicate::test);
            final Collection<ImplementedClassMethod> nonPrivateHiddenMethods = Collections2.filter(
                    overridenMethods,
                    isVirtualPredicate.negate().and(method -> method.getSelector().getUnderlyingMethod().getVisibility() != Visibility.PRIVATE)::test);
            final Collection<ImplementedClassMethod> validVirtualOverridenMethods = Collections2.filter(
                    virtualOverridenMethods,
                    validationCovariantMismatch.and(validationReturnMismatch).negate()::test);
            final Collection<ImplementedClassMethod> tagReuseCandidates = Collections2.filter(
                    validVirtualOverridenMethods,
                    method -> !method.hasChangedTypes(this) && sameReturnType.test(selector, method));
            final Collection<ImplementedClassMethod> covariantMismatchMethods = Collections2.filter(
                    virtualOverridenMethods,
                    validationCovariantMismatch::test);
            final Collection<ImplementedClassMethod> returnMismatchMethods = Collections2.filter(
                    virtualOverridenMethods,
                    validationReturnMismatch::test);

            // Figure out which tag type to use.
            final ClassType tagType;
            if (!selector.getUnderlyingMethod().isCovariantReturn()) {
                tagType = null;
            } else if (tagReuseCandidates.stream()
                    .map(ImplementedClassMethod::getTagType)
                    .distinct()
                    .count() == 1) {
                tagType = tagReuseCandidates.stream()
                        .map(ImplementedClassMethod::getTagType)
                        .distinct()
                        .collect(MoreCollectors.onlyElement());
            } else {
                tagType = this;
            }

            // Declare the method.
            final ImplementedClassMethod selectorImpl = new ImplementedClassMethod(this, tagType, selector, validVirtualOverridenMethods);
            result.add(selectorImpl);

            if (!covariantMismatchMethods.isEmpty()) {
                LOG.log(Level.WARNING,
                        () -> {
                            return selector.toString()
                            + (selector.getUnderlyingMethod().isCovariantReturn() ? " has co-variant return" : " does not have co-variant return")
                            + ", contrary to:\n"
                            + covariantMismatchMethods.stream()
                                    .map(ImplementedClassMethod::getSelector)
                                    .map(OverrideSelector::toString)
                                    .collect(Collectors.joining("\n"));
                        });
            }
            if (!returnMismatchMethods.isEmpty()) {
                LOG.log(Level.WARNING,
                        () -> {
                            return selector.toString()
                            + " has different return type from overriden methods:"
                            + returnMismatchMethods.stream()
                                    .map(ImplementedClassMethod::getSelector)
                                    .map(OverrideSelector::toString)
                                    .collect(Collectors.joining("\n"));
                        });
            }
            if (!selectorImpl.getSelector().getUnderlyingMethod().isHideOk() && !nonPrivateHiddenMethods.isEmpty()) {
                LOG.log(Level.WARNING,
                        () -> {
                            return selector.toString()
                            + " hides:\n"
                            + nonPrivateHiddenMethods.stream()
                                    .map(ImplementedClassMethod::getSelector)
                                    .map(OverrideSelector::toString)
                                    .collect(Collectors.joining("\n"));
                        });
            }
            if (selectorImpl.getSelector().getUnderlyingMethod().isOverride() && virtualOverridenMethods.isEmpty()) {
                LOG.log(Level.WARNING, "{0} is declared override, but does not override anything", selector.toString());
            }
        });

        return result;
    }

    /**
     * Each of the non-virtual methods is emitted as is.
     *
     * Some diagnostic warnings may be generated.
     *
     * @param nonVirtualClassMembersWithParentMethods All class members that are
     * non-virtual.
     * @return {@link ImplementedClassMethod Implemented method} for each of the
     * methods in {@code nonVirtualClassMembersWithParentMethods}.
     */
    private List<ImplementedClassMethod> postProcessNonVirtualClassMembers(Map<OverrideSelector, Collection<ImplementedClassMethod>> nonVirtualClassMembersWithParentMethods) {
        final List<ImplementedClassMethod> result = new ArrayList<>();

        nonVirtualClassMembersWithParentMethods.forEach((selector, hiddenMethods) -> {
            // Exclude private methods from parents from our view.
            final Collection<ImplementedClassMethod> nonPrivateHiddenMethods = Collections2.filter(
                    hiddenMethods,
                    method -> method.getSelector().getUnderlyingMethod().getVisibility() != Visibility.PRIVATE);

            final ImplementedClassMethod selectorImpl = new ImplementedClassMethod(this, null, selector, EMPTY_LIST);
            if (hiddenMethods.stream().anyMatch(method -> method.getSelector().getUnderlyingMethod().isVirtual())) {
                LOG.log(Level.WARNING,
                        () -> {
                            return "Non-virtual " + selector.toString()
                            + " is effectively virtual due to virtual methods from parent type:\n"
                            + hiddenMethods.stream()
                                    .map(ImplementedClassMethod::getSelector)
                                    .map(OverrideSelector::toString)
                                    .collect(Collectors.joining("\n"));
                        });
            } else if (!selectorImpl.getSelector().getUnderlyingMethod().isHideOk() && !nonPrivateHiddenMethods.isEmpty()) {
                LOG.log(Level.WARNING,
                        () -> {
                            return selector.toString()
                            + " hides:\n"
                            + nonPrivateHiddenMethods.stream()
                                    .map(ImplementedClassMethod::getSelector)
                                    .map(OverrideSelector::toString)
                                    .collect(Collectors.joining("\n"));
                        });
            }
            result.add(selectorImpl);
        });

        return result;
    }

    /**
     * Create class members for the virtual functions.
     *
     * This creates the method implementation, as well as the forwarder (if
     * tagging is used). This also creates the forwarders for overriden methods,
     * if the implementation isn't a pure virtual function.
     *
     * @param virtualClassMembers List of virtual methods to emit.
     * @param ctx Context for type resolution.
     */
    private void postProcessEmitVirtualClassMembers(List<ImplementedClassMethod> virtualClassMembers, Context ctx) {
        virtualClassMembers.forEach(method -> {
            final MethodModel underlyingMethod = method.getSelector().getUnderlyingMethod();
            final boolean tagged = underlyingMethod.isCovariantReturn();
            final String virtualMethodName = (tagged ? VIRTUAL_FUNCTION_PREFIX : "") + underlyingMethod.getName();
            final Type myTag;
            if (tagged) {
                myTag = new CxxType(ctx, "$tagType(model)$", new Includes(), singletonMap("model", method.getTagType()), EMPTY_LIST, null);
            } else {
                myTag = null;
            }

            // Figure out body of forwarders to private virtual method.
            final String forwardingBody = new ST(StCtx.BUILTINS, FORWARDING_BODY_TEMPLATE)
                    .add("model", this)
                    .add("tagModel", method.getTagType())
                    .add("fn", method.getSelector())
                    .add("covariantReturn", tagged)
                    .render(Locale.ROOT);

            // Emit the implementation.
            {
                final MethodModel.SimpleMethodModel methodImpl = new MethodModel.SimpleMethodModel(
                        this,
                        this.getBoundType(),
                        virtualMethodName,
                        new Includes(
                                underlyingMethod.getDeclarationIncludes().collect(Collectors.toList()),
                                underlyingMethod.getImplementationIncludes().collect(Collectors.toList())),
                        underlyingMethod.getDeclarationTypes().collect(Collectors.toSet()),
                        underlyingMethod.getImplementationTypes().collect(Collectors.toSet()),
                        method.getSelector().getReturnType(),
                        Stream.concat((tagged ? Stream.of(myTag) : Stream.empty()), method.getSelector().getArguments().stream()).collect(Collectors.toList()), // Prepend tag type, for tagged dispatch, iff covariant.
                        Stream.concat((tagged ? Stream.of("_tag_") : Stream.empty()), underlyingMethod.getArgumentNames().stream()).collect(Collectors.toList()), // Prepend argument name for tag type, iff covariant.
                        underlyingMethod.getBody(),
                        underlyingMethod.isStatic(),
                        underlyingMethod.isVirtual(),
                        underlyingMethod.isPureVirtual(),
                        (tagged ? method.getTagType() != this : underlyingMethod.isOverride()), // We always apply the `override` modifier if we can, but must hide it if we had to re-tag.
                        underlyingMethod.isConst(),
                        underlyingMethod.isFinal(),
                        null, // not inline
                        underlyingMethod.getNoexcept(),
                        (tagged ? Visibility.PRIVATE : underlyingMethod.getVisibility()), // Make actual virtual method private: it's only accessed via the untagged forwarding function.
                        underlyingMethod.getDocString());
                // Always add to class.
                // If the method is new or ambiguous, also to the accessor.
                // As an extra optimization, we also add the method if this class is final,
                // so the compiler has an opportunity to optimize out the vtable dispatch.
                classMemberFunctions.add(methodImpl);
                if (!tagged && (method.getOverridenMethods().size() != 1 || isFinal())) {
                    accessor.add(new AccessorMethod.Impl(
                            this,
                            underlyingMethod.getName(),
                            underlyingMethod.getArgumentTypes(),
                            underlyingMethod.getArgumentNames(),
                            underlyingMethod.getReturnType(),
                            new Includes(underlyingMethod.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                            false, // Not static, because virtual methods can not be static.
                            underlyingMethod.isConst(),
                            underlyingMethod.getNoexcept(),
                            underlyingMethod.getVisibility(),
                            underlyingMethod.getDocString(),
                            underlyingMethod.getMethodGenerics()));
                }
            }

            // Emit forwarder.
            // The forwarder is the untagged version of this function, that
            // then forwards to the implementation, using tag-dispatch.
            //
            // We emit this for tagged functions only.
            // And only if at least one of:
            // - the implementation introduces a new tag,
            // - if we need to disambiguate between forwarders in the parent,
            // - or this is the first declaration of the method.
            // Otherwise, we rely on the existing forwarder in the parent.
            if (tagged && (method.getTagType() == this || method.getOverridenMethods().size() != 1)) {
                final MethodModel.SimpleMethodModel primaryForwarder = new MethodModel.SimpleMethodModel(
                        this,
                        this.getBoundType(),
                        underlyingMethod.getName(),
                        new Includes(
                                underlyingMethod.getDeclarationIncludes().collect(Collectors.toList()),
                                singletonList("java/_maybe_cast.h")),
                        underlyingMethod.getDeclarationTypes().collect(Collectors.toSet()),
                        underlyingMethod.getImplementationTypes().collect(Collectors.toSet()),
                        method.getSelector().getReturnType(),
                        method.getSelector().getArguments(),
                        underlyingMethod.getArgumentNames(),
                        forwardingBody,
                        underlyingMethod.isStatic(),
                        false, // Forwarding function is never virtual, so we can apply method hiding.
                        false, // Forwarding function is never pure virtual.
                        false, // Forwarding function is not overriding, but hiding instead.
                        underlyingMethod.isConst(),
                        false, // Forwarding function is not final, because it is not virtual.
                        "JSER_INLINE", // Forwarding function is inline.
                        underlyingMethod.getNoexcept(),
                        underlyingMethod.getVisibility(),
                        underlyingMethod.getDocString());
                // Add to both class and accessor implementation.
                classMemberFunctions.add(primaryForwarder);
                accessor.add(new AccessorMethod.Impl(
                        this,
                        underlyingMethod.getName(),
                        underlyingMethod.getArgumentTypes(),
                        underlyingMethod.getArgumentNames(),
                        underlyingMethod.getReturnType(),
                        new Includes(underlyingMethod.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                        false, // Not static, because virtual methods can not be static.
                        underlyingMethod.isConst(),
                        underlyingMethod.getNoexcept(),
                        underlyingMethod.getVisibility(),
                        underlyingMethod.getDocString(),
                        underlyingMethod.getMethodGenerics()));
            }

            // Emit the forwarders from other implementations.
            // We only need to emit those if we are using tags.
            //
            // We don't need to emit those if the method is a pure-virtual-function.
            // We could, but then we would have to consider disambiguation between
            // forwarding implementations in parents. And there would be a performance
            // cost by invoking a chain of virtual functions, each requiring a
            // vtable dispatch.
            // Instead, it's easier and (probably) better for performance to emit
            // all overrides. They're not very large, so there won't be much of a
            // penalty either.
            if (tagged && !underlyingMethod.isPureVirtual()) {
                method.getAllOverridenMethods()
                        .filter(overrideFn -> !Objects.equals(overrideFn.getTagType(), method.getTagType()))
                        .collect(Collectors.collectingAndThen(
                                Collectors.toMap(overrideFn -> overrideFn.getTagType(), Function.identity(), (x, y) -> x),
                                mapping -> mapping.values().stream()))
                        .forEach(overridenMethod -> {
                            classMemberFunctions.add(new ClassVirtualDelegateMethod.Impl(
                                    method,
                                    overridenMethod,
                                    new Includes(
                                            overridenMethod.getSelector().getUnderlyingMethod().getIncludes().getDeclarationIncludes(),
                                            method.getSelector().getUnderlyingMethod().getIncludes().getDeclarationIncludes()),
                                    VIRTUAL_FUNCTION_PREFIX + method.getSelector().getName(),
                                    VIRTUAL_FUNCTION_PREFIX + overridenMethod.getSelector().getName()));
                        });
            }
        });
    }

    /**
     * Emit both accessor and class methods for each declared non-virtual
     * method.
     *
     * @param nonVirtualClassMembers All non-virtual methods.
     */
    private void postProcessEmitNonVirtualClassMembers(List<ImplementedClassMethod> nonVirtualClassMembers) {
        nonVirtualClassMembers.stream()
                .map(ImplementedClassMethod::getSelector)
                .map(OverrideSelector::getUnderlyingMethod)
                .peek(method -> {
                    assert !method.isVirtual();
                })
                .forEach(method -> {
                    accessor.add(new AccessorMethod.Impl(
                            this,
                            method.getName(),
                            method.getArgumentTypes(),
                            method.getArgumentNames(),
                            method.getReturnType(),
                            new Includes(method.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                            method.isStatic(),
                            method.isConst(),
                            method.getNoexcept(),
                            method.getVisibility(),
                            method.getDocString(),
                            method.getMethodGenerics()));

                    if (method.getMethodGenerics().isEmpty()) {
                        classMemberFunctions.add(method);
                    } else {
                        classMemberFunctions.add(new ClassDelegateMethod.Impl(
                                this,
                                method.getName(),
                                "__erased_" + method.getName(),
                                method.getArgumentTypes(),
                                method.getArgumentNames(),
                                method.getReturnType(),
                                new Includes(method.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                                method.isStatic(),
                                method.isConst(),
                                method.getNoexcept(),
                                method.getVisibility(),
                                method.getDocString(),
                                method.getMethodGenerics()));

                        final MethodGenerics methodGenerics = new MethodGenerics(method.getMethodGenerics(), method.getArgumentTypes());
                        classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                                this,
                                method.getArgumentDeclaringClass(),
                                "__erased_" + method.getName(),
                                method.getIncludes(),
                                method.getDeclarationTypes().collect(Collectors.toSet()),
                                method.getImplementationTypes().collect(Collectors.toSet()),
                                (method.getReturnType() == null ? null : method.getReturnType().rebind(methodGenerics.getErasedMethodGenerics())),
                                method.getArgumentTypes().stream().map(argType -> argType.rebind(methodGenerics.getErasedMethodGenerics())).collect(Collectors.toList()),
                                method.getArgumentNames(),
                                method.getBody(),
                                method.isStatic(),
                                method.isVirtual(),
                                method.isPureVirtual(),
                                method.isOverride(),
                                method.isConst(),
                                method.isFinal(),
                                method.getInline(),
                                method.getNoexcept(),
                                method.getVisibility(),
                                method.getDocString()));
                    }
                });
    }

    /**
     * Emit accessors for static functions.
     *
     * @param staticMethods Collection of static functions.
     */
    private void postProcessEmitStaticClassMembers(Collection<ClassMemberModel.ClassMethod> staticMethods) {
        staticMethods.forEach(method -> {
            assert method.isStatic() : "Only static methods should be supplied.";

            if (method.getMethodGenerics().isEmpty()) {
                classMemberFunctions.add(method);
            } else {
                classMemberFunctions.add(new ClassDelegateMethod.Impl(
                        this,
                        method.getName(),
                        "__erased_" + method.getName(),
                        method.getArgumentTypes(),
                        method.getArgumentNames(),
                        method.getReturnType(),
                        new Includes(method.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                        method.isStatic(),
                        method.isConst(),
                        method.getNoexcept(),
                        method.getVisibility(),
                        method.getDocString(),
                        method.getMethodGenerics()));

                final MethodGenerics methodGenerics = new MethodGenerics(method.getMethodGenerics(), method.getArgumentTypes());
                classMemberFunctions.add(new MethodModel.SimpleMethodModel(
                        this,
                        method.getArgumentDeclaringClass(),
                        "__erased_" + method.getName(),
                        method.getIncludes(),
                        method.getDeclarationTypes().collect(Collectors.toSet()),
                        method.getImplementationTypes().collect(Collectors.toSet()),
                        (method.getReturnType() == null ? null : method.getReturnType().rebind(methodGenerics.getErasedMethodGenerics())),
                        method.getArgumentTypes().stream().map(argType -> argType.rebind(methodGenerics.getErasedMethodGenerics())).collect(Collectors.toList()),
                        method.getArgumentNames(),
                        method.getBody(),
                        method.isStatic(),
                        method.isVirtual(),
                        method.isPureVirtual(),
                        method.isOverride(),
                        method.isConst(),
                        method.isFinal(),
                        method.getInline(),
                        method.getNoexcept(),
                        method.getVisibility(),
                        method.getDocString()));
            }

            accessor.add(new AccessorMethod.Impl(
                    this,
                    method.getName(),
                    method.getArgumentTypes(),
                    method.getArgumentNames(),
                    method.getReturnType(),
                    new Includes(method.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                    method.isStatic(),
                    method.isConst(),
                    method.getNoexcept(),
                    method.getVisibility(),
                    method.getDocString(),
                    method.getMethodGenerics()));
        });
    }

    /**
     * Emit accessors for class constructors.
     *
     * @param constructors Collection of class constructors.
     */
    private void postProcessEmitClassConstructors(Collection<ClassMemberModel.ClassConstructor> constructors) {
        constructors.forEach(method -> {
            // XXX not yet
            // classConstructors.add(method);
            accessor.add(new AccessorConstructor.Impl(
                    this,
                    method.getArgumentTypes(),
                    method.getArgumentNames(),
                    new Includes(method.getIncludes().getDeclarationIncludes(), EMPTY_LIST),
                    method.getVisibility(),
                    method.getDocString(),
                    method.getFunctionGenericsNames(),
                    method.getFunctionGenericsDefault(),
                    getErasedTemplateArguments().stream().collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue))));
        });
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
     * Get raw source content, that is to be emitted before the includes.
     *
     * @return Raw source content.
     */
    public String getSrcRaw() {
        return srcRaw;
    }

    /**
     * Retrieve the {@link ClassConfig.CfgSuperType CfgSuperType} for this type.
     *
     * @return Instance of {@link ClassConfig.CfgSuperType CfgSuperType} backed
     * by this {@link ClassType ClassType}.
     */
    private ClassConfig.CfgSuperType cfgType() {
        return new CfgSuperType();
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

    private class CfgSuperType implements ClassConfig.CfgSuperType {
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
            return classType() == ((CfgSuperType) other).classType();
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
     * If set, the class has a write method.
     */
    private boolean writeMethod = false;
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
    private Type varType;
    /**
     * List of friend types.
     */
    private List<Type> friends;
    /**
     * Marker to prevent us from post processing twice.
     */
    private boolean postProcessingDone = false;
    /**
     * Member functions for the class.
     *
     * Filled in by
     * {@link #postProcess(com.github.nahratzah.jser_plus_plus.input.Context) post processing}
     * logic.
     */
    private final Collection<MethodModel> classMemberFunctions = new TreeSet<>(new MethodModelComparator());
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
     * Set holding all {@link ImplementedClassMethod implemented methods}.
     *
     * Filled in by
     * {@link #postProcess(com.github.nahratzah.jser_plus_plus.input.Context) post processing}
     * logic.
     */
    private Set<ImplementedClassMethod> implementedMethods;
    /**
     * Raw content for source file, to be emitted prior to any includes.
     */
    private String srcRaw;
    /**
     * Accessor model for this class.
     */
    private final Accessor accessor = new Accessor(this);
}
