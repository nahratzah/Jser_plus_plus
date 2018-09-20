package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgField;
import com.github.nahratzah.jser_plus_plus.config.ClassConfig;
import com.github.nahratzah.jser_plus_plus.config.ClassMember;
import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Destructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.java.ReflectUtil;
import static com.github.nahratzah.jser_plus_plus.model.JavaType.getAllTypeParameters;
import static com.github.nahratzah.jser_plus_plus.model.Type.typeFromCfgType;
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
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.function.BiConsumer;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Models a normal class.
 *
 * @author ariane
 */
public class ClassType implements JavaType {
    private static final Logger LOG = Logger.getLogger(ClassType.class.getName());

    public ClassType(Class<?> c) {
        this.c = requireNonNull(c);
    }

    @Override
    public void init(Context ctx, Config cfg) {
        LOG.log(Level.INFO, "Initializing {0}", c);
        final ClassConfig classCfg = cfg.getConfigForClass(c);

        final List<? extends TypeVariable<? extends Class<?>>> cTypeParameters = getAllTypeParameters(this.c);
        LOG.log(Level.FINE, "Type parameters: {0}", cTypeParameters);
        final Map<String, String> argRename = unmodifiableMap(cfg.getTemplateArguments(c.getName())
                .map(cfgParameters -> buildRenameMap(cTypeParameters, cfgParameters))
                .orElseGet(() -> buildRenameMap(cTypeParameters)));
        LOG.log(Level.INFO, "Rename map for {0} = {1}", new Object[]{c, argRename});

        final ObjectStreamClass streamClass = ObjectStreamClass.lookupAny(this.c);

        this.serialVersionUID = streamClass.getSerialVersionUID();
        initTemplateArguments(ctx, classCfg, cTypeParameters, argRename);
        initClassAttributes(ctx, classCfg, argRename);
        initSuperTypes(ctx, classCfg, argRename);
        initFields(ctx, classCfg, argRename, streamClass);
        initClassMembers(ctx, classCfg, argRename);
        initFriendTypes(ctx, classCfg, argRename);
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
            public IntermediateFieldDescr(String name, Class<?> classType, Type reflectType, Field reflectField) {
                this.name = requireNonNull(name);
                this.classType = requireNonNull(classType);
                this.reflectType = reflectType;
                this.reflectField = reflectField;
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
                    return new IntermediateFieldDescr(name, type, reflectType, reflectField);
                })
                .map(iField -> {
                    final Type visitType = iField.reflectType != null ? iField.reflectType : iField.classType;
                    final BoundTemplate type = ReflectUtil.visitType(visitType, new BoundsMapping(ctx, argRename));

                    final FieldType fieldType = new FieldType(iField.name, type);
                    if (iField.reflectField != null) {
                        fieldType.setFinal(Modifier.isFinal(iField.reflectField.getModifiers()));
                    }
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

                    return new FieldType(name, type, false);
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
                })
                .peek(iField -> {
                    iField.prerender(ctx, singletonMap("model", this), getTemplateArgumentNames());
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
    public ArrayList<Map.Entry<String, List<BoundTemplate>>> getErasedTemplateArguments() {
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
        return new ArrayList<>(mapping.entrySet());
    }

    @Override
    public BoundTemplate.ClassBinding getSuperClass() {
        return this.superType;
    }

    @Override
    public Collection<BoundTemplate.ClassBinding> getInterfaces() {
        return unmodifiableCollection(this.interfaceTypes);
    }

    @Override
    public Stream<String> getImplementationIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        if (!recursionGuard.add(this)) return Stream.empty();

        final Stream<String> parentTypes = Stream.concat(Stream.of(getSuperClass()).filter(Objects::nonNull), getInterfaces().stream())
                .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));

        final Stream<String> fieldIncludes;
        final Stream<String> memberIncludes;
        if (publicOnly) {
            fieldIncludes = getFields().stream()
                    .filter(field -> field.isGetterFn() || field.isSetterFn())
                    .map(field -> field.getVarType())
                    .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));
            memberIncludes = getClassMembers().stream()
                    .flatMap(member -> member.getDeclarationIncludes());
        } else {
            fieldIncludes = getFields().stream()
                    .flatMap(field -> Stream.of(field.getType(), field.getVarType()))
                    .flatMap(c -> c.getIncludes(publicOnly, recursionGuard));
            memberIncludes = getClassMembers().stream()
                    .flatMap(member -> member.getImplementationIncludes());
        }

        return Stream.of(parentTypes, fieldIncludes, memberIncludes)
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
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> memberTypes = getClassMembers().stream()
                .flatMap(member -> member.getDeclarationTypes());

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
        final Stream<com.github.nahratzah.jser_plus_plus.model.Type> memberTypes = getClassMembers().stream()
                .flatMap(member -> member.getImplementationTypes());

        return Stream.of(superTypes, fieldTypes, memberTypes)
                .flatMap(Function.identity())
                .flatMap(com.github.nahratzah.jser_plus_plus.model.Type::getAllJavaTypes)
                .filter(c -> !(c instanceof PrimitiveType));
    }

    public long getSerialVersionUID() {
        return serialVersionUID;
    }

    /**
     * Retrieve the fields of this type.
     *
     * @return The fields of this type.
     */
    public List<FieldType> getFields() {
        return fields;
    }

    @Override
    public boolean isInterface() {
        return Modifier.isInterface(this.c.getModifiers());
    }

    @Override
    public boolean isAbstract() {
        return Modifier.isAbstract(this.c.getModifiers());
    }

    @Override
    public boolean isFinal() {
        return finalVar;
    }

    public Collection<ClassMemberModel> getClassMembers() {
        return classMembers;
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

    @Override
    public com.github.nahratzah.jser_plus_plus.model.Type getVarType() {
        return varType;
    }

    public List<com.github.nahratzah.jser_plus_plus.model.Type> getFriends() {
        return friends;
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
            return new BoundTemplate.ClassBinding(cVar, cArgs);
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

                    return new BoundTemplate.ClassBinding(ctx.resolveClass(var), cArgs);
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
    private static class ParentTypeVisitor implements ReflectUtil.Visitor<BoundTemplate.ClassBinding> {
        public ParentTypeVisitor(Context ctx, Map<? super String, ? extends String> argRename) {
            this.ctx = requireNonNull(ctx);
            this.argRename = requireNonNull(argRename);
        }

        @Override
        public BoundTemplate.ClassBinding apply(Class<?> var) {
            if (var.isArray())
                throw new IllegalStateException("Parent type can not be an array.");

            return new BoundTemplate.ClassBinding(ctx.resolveClass(var), EMPTY_LIST);
        }

        @Override
        public BoundTemplate.ClassBinding apply(TypeVariable<?> var) {
            throw new IllegalStateException("Parent type can not be a generics variable.");
        }

        @Override
        public BoundTemplate.ClassBinding apply(GenericArrayType var) {
            throw new IllegalStateException("Parent type can not be an array.");
        }

        @Override
        public BoundTemplate.ClassBinding apply(WildcardType var) {
            throw new IllegalStateException("Parent type can not be a wildcard.");
        }

        @Override
        public BoundTemplate.ClassBinding apply(ParameterizedType var) {
            final BoundsMapping boundsMapping = new BoundsMapping(ctx, argRename);

            final List<BoundTemplate> cArgs = Arrays.stream(var.getActualTypeArguments())
                    .map(x -> ReflectUtil.visitType(x, boundsMapping))
                    .collect(Collectors.toList());

            return (BoundTemplate.ClassBinding) ReflectUtil.visitType(var.getRawType(), new ReflectUtil.Visitor<BoundTemplate>() {
                @Override
                public BoundTemplate apply(Class<?> var) {
                    if (var.isArray())
                        return new BoundTemplate.ArrayBinding(apply(var.getComponentType()), 1);

                    return new BoundTemplate.ClassBinding(ctx.resolveClass(var), cArgs);
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

        private final Context ctx;
        private final Map<? super String, ? extends String> argRename;
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
    private BoundTemplate.ClassBinding superType;
    /**
     * List of interfaces used by this class.
     */
    private List<BoundTemplate.ClassBinding> interfaceTypes;
    /**
     * The serial version UID of the class.
     */
    private long serialVersionUID = 0;
    /**
     * List of fields.
     */
    private List<FieldType> fields;
    /**
     * If the class is a final class.
     */
    private boolean finalVar;
    /**
     * Class members for the class.
     */
    private Collection<ClassMemberModel> classMembers;
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
}
