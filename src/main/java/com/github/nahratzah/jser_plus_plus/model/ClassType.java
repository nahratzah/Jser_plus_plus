package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.java.ReflectUtil;
import static com.github.nahratzah.jser_plus_plus.model.JavaType.getAllTypeParameters;
import java.io.ObjectStreamClass;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.unmodifiableCollection;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableMap;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
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

        final List<? extends TypeVariable<? extends Class<?>>> cTypeParameters = getAllTypeParameters(this.c);
        LOG.log(Level.FINE, "Type parameters: {0}", cTypeParameters);
        final Map<String, String> argRename = unmodifiableMap(buildRenameMap(cTypeParameters));

        final ObjectStreamClass streamClass = ObjectStreamClass.lookupAny(this.c);

        this.serialVersionUID = streamClass.getSerialVersionUID();
        initTemplateArguments(ctx, cfg, cTypeParameters, argRename);
        initSuperTypes(ctx, cfg, argRename);
        initFields(ctx, cfg, argRename, streamClass);
    }

    private void initTemplateArguments(Context ctx, Config cfg, List<? extends TypeVariable<? extends Class<?>>> cTypeParameters, Map<String, String> argRename) {
        this.templateArguments = cTypeParameters.stream()
                .map(cTypeParameter -> {
                    final String argName = cTypeParameter.getName();
                    final List<BoundTemplate> argBounds = Arrays.stream(cTypeParameter.getBounds())
                            .map(b -> ReflectUtil.visitType(b, new BoundsMapping(ctx, argRename)))
                            .collect(Collectors.toList());
                    return new ClassTemplateArgument(argName, argBounds);
                })
                .collect(Collectors.toList());
    }

    private void initSuperTypes(Context ctx, Config cfg, Map<String, String> argRename) {
        this.superType = Optional.ofNullable(this.c.getGenericSuperclass())
                .map(t -> ReflectUtil.visitType(t, new ParentTypeVisitor(ctx, argRename)))
                .orElse(null);
        this.interfaceTypes = Arrays.stream(this.c.getGenericInterfaces())
                .map(t -> ReflectUtil.visitType(t, new ParentTypeVisitor(ctx, argRename)))
                .collect(Collectors.toList());
    }

    private void initFields(Context ctx, Config cfg, Map<String, String> argRename, ObjectStreamClass streamClass) {
        class IntermediateFieldDescr {
            public IntermediateFieldDescr(String name, Class<?> classType, Type reflectType) {
                this.name = name;
                this.classType = classType;
                this.reflectType = reflectType;
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
        }

        this.fields = Arrays.stream(streamClass.getFields())
                .map(f -> {
                    final String name = f.getName();
                    final Class<?> type = f.getType();

                    // Fill in the reflect type only if the class implementation
                    // and the serialization code agree on the type.
                    Type reflectType = null;
                    try {
                        final Field reflectField = this.c.getField(name);
                        if (Objects.equals(reflectField.getType().getName(), type.getName()))
                            reflectType = reflectField.getGenericType();
                    } catch (NoSuchFieldException ex) {
                        reflectType = null;
                    }
                    return new IntermediateFieldDescr(name, type, reflectType);
                })
                .map(iField -> {
                    final Type visitType = iField.reflectType != null ? iField.reflectType : iField.classType;
                    final BoundTemplate type = ReflectUtil.visitType(visitType, new BoundsMapping(ctx, argRename));
                    return new FieldType(iField.name, type);
                })
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

    @Override
    public BoundTemplate getSuperClass() {
        return this.superType;
    }

    @Override
    public Collection<BoundTemplate> getInterfaces() {
        return unmodifiableCollection(this.interfaceTypes);
    }

    @Override
    public Collection<String> getIncludes(boolean publicOnly) {
        return EMPTY_LIST;
    }

    public long getSerialVersionUID() {
        return serialVersionUID;
    }

    @Override
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
        return Modifier.isFinal(this.c.getModifiers());
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
            return new BoundTemplate.Any();
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
    private static class ParentTypeVisitor implements ReflectUtil.Visitor<BoundTemplate> {
        public ParentTypeVisitor(Context ctx, Map<? super String, ? extends String> argRename) {
            this.ctx = requireNonNull(ctx);
            this.argRename = requireNonNull(argRename);
        }

        @Override
        public BoundTemplate apply(Class<?> var) {
            if (var.isArray())
                throw new IllegalStateException("Parent type can not be an array.");

            return new BoundTemplate.ClassBinding(ctx.resolveClass(var), EMPTY_LIST);
        }

        @Override
        public BoundTemplate apply(TypeVariable<?> var) {
            throw new IllegalStateException("Parent type can not be a generics variable.");
        }

        @Override
        public BoundTemplate apply(GenericArrayType var) {
            throw new IllegalStateException("Parent type can not be an array.");
        }

        @Override
        public BoundTemplate apply(WildcardType var) {
            throw new IllegalStateException("Parent type can not be a wildcard.");
        }

        @Override
        public BoundTemplate apply(ParameterizedType var) {
            final BoundsMapping boundsMapping = new BoundsMapping(ctx, argRename);

            final List<BoundTemplate> cArgs = Arrays.stream(var.getActualTypeArguments())
                    .map(x -> ReflectUtil.visitType(x, boundsMapping))
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
    private BoundTemplate superType;
    /**
     * List of interfaces used by this class.
     */
    private List<BoundTemplate> interfaceTypes;
    /**
     * The serial version UID of the class.
     */
    private long serialVersionUID = 0;
    /**
     * List of fields.
     */
    private List<FieldType> fields;
}
