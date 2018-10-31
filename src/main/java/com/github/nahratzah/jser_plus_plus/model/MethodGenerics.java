package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.misc.SimpleMapEntry;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.MoreCollectors;
import com.google.common.collect.Multimap;
import com.google.common.collect.Multimaps;
import com.google.common.collect.Streams;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.EMPTY_MAP;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableMap;
import java.util.HashMap;
import java.util.HashSet;
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
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Class that manages generics.
 */
public class MethodGenerics {
    private static final String ENABLER_TEMPLATE_ARGUMENT = "__Enabler__";

    public MethodGenerics(Map<String, BoundTemplate> methodGenerics, List<Type> argumentTypes) {
        this.methodGenerics = unmodifiableMap(requireNonNull(methodGenerics));
        // Validate that methodGenerics doesn't hold null values.
        this.methodGenerics.forEach((k, v) -> {
            requireNonNull(k);
            requireNonNull(v);
        });

        // If there are no generics, initialize this as completely empty.
        if (this.methodGenerics.isEmpty()) {
            this.bindingsOverrides = EMPTY_LIST;
            this.derivationMap = EMPTY_MAP;
            this.erasedMethodGenerics = EMPTY_MAP;
            this.functionGenericsDefault = EMPTY_LIST;
            this.functionGenericsNames = EMPTY_LIST;
            this.templatedArgumentTypes = unmodifiableList(new ArrayList<>(argumentTypes));
            this.genericNameToDerivationNameRebindMap = EMPTY_MAP;
            return;
        }

        this.erasedMethodGenerics = unmodifiableMap(squashMethodTypes(this.methodGenerics));

        // Compute rebind map.
        // This map is exposed, so that methods can rebind their return values.
        this.genericNameToDerivationNameRebindMap = this.methodGenerics.keySet().stream()
                .collect(Collectors.collectingAndThen(
                        Collectors.toMap(Function.identity(), name -> new BoundTemplate.VarBinding(genericNameToDerivationName(name))),
                        Collections::unmodifiableMap));

        // Figure out selectors provides by each variable.
        final List<Multimap<String, TemplateSelector>> selectors = argumentTypes.stream()
                .map(type -> selectorsForArg(type))
                .collect(Collectors.collectingAndThen(
                        Collectors.toList(),
                        Collections::unmodifiableList));

        // Figure out binding overrides:
        // to make the generics work, we need to be able to bind any reference.
        // If we didn't, we would not be able to handle field pointers, due to the
        // pointer implementation not matching.
        this.bindingsOverrides = Streams.mapWithIndex(selectors.stream(), (selectorSet, idx) -> {
            if (selectorSet.isEmpty())
                return Optional.<BoundTemplate.VarBinding>empty();
            return Optional.of(new BoundTemplate.VarBinding(String.format(Locale.ROOT, "__Arg_%d__", idx)));
        }).collect(Collectors.collectingAndThen(Collectors.toList(), Collections::unmodifiableList));

        // Create a parallel argument types list, with any template-binding types replaced with template argument.
        this.templatedArgumentTypes = Streams.zip(
                argumentTypes.stream(),
                bindingsOverrides.stream(),
                (Type type, Optional<BoundTemplate.VarBinding> optIndexedArg) -> {
                    final Optional<Type> result = optIndexedArg
                            .map(argVar -> String.format(Locale.ROOT, "%s&&", argVar.getName())) // Write argument such that we can use perfect forwarding.
                            .map(argVarInCxxStr -> new CxxType(Context.UNIMPLEMENTED_CONTEXT, argVarInCxxStr, new Includes(), EMPTY_MAP, EMPTY_LIST, null)); // Wrap statement in appropriate type.
                    return result.orElse(type); // Fall back to declared type if the argument is irrelevant for template deduction.
                })
                .collect(Collectors.collectingAndThen(Collectors.toList(), Collections::unmodifiableList));

        // Figure out the derivation map.
        // The map orders variables based on resolution order.
        this.derivationMap = resolutionOrder(this.methodGenerics).stream()
                .map(name -> {
                    final Stream<String> computationStatements = Streams.zip(
                            bindingsOverrides.stream()
                                    .map(optBinding -> { // Turn binding into type selection statement.
                                        return optBinding
                                                .map(BoundTemplate.VarBinding::getName)
                                                .map(bindingName -> String.format(Locale.ROOT, "::std::decay_t<%s>", bindingName));
                                    }),
                            selectors.stream()
                                    .map(s -> s.get(name)),
                            (optBinding, selectorSet) -> {
                                assert selectorSet.isEmpty() || optBinding.isPresent() : "if selectors are present, the binding must be declared";
                                return selectorSet.stream()
                                        .map(selector -> selector.selector(optBinding.get(), name, false))
                                        .map(selectorStmt -> "typename " + selectorStmt);
                            })
                            .flatMap(Function.identity());
                    return new SimpleMapEntry<>(name, computationStatements);
                })
                .collect(Collectors.collectingAndThen(
                        Collectors.toMap(
                                Map.Entry::getKey,
                                entry -> entry.getValue().collect(Collectors.collectingAndThen(
                                        Collectors.toList(),
                                        Collections::unmodifiableList)),
                                (u, v) -> {
                                    throw new IllegalStateException(String.format("Duplicate key %s", u));
                                },
                                LinkedHashMap::new),
                        Collections::unmodifiableMap));
        assert Arrays.equals(this.derivationMap.keySet().toArray(), resolutionOrder(this.methodGenerics).toArray()) : "derivation map must list elements in order of resolution";

        this.functionGenericsNames = Streams.concat(
                this.methodGenerics.keySet().stream(),
                this.bindingsOverrides.stream().flatMap(Streams::stream).map(BoundTemplate.VarBinding::getName),
                this.derivationMap.keySet().stream().map(name -> genericNameToDerivationName(name)),
                Stream.of(ENABLER_TEMPLATE_ARGUMENT))
                .collect(Collectors.collectingAndThen(
                        Collectors.toList(),
                        Collections::unmodifiableList));
        assert this.functionGenericsNames.size() == this.functionGenericsNames.stream().distinct().count() : "there may be no duplicate names";

        // Compute the enabler.
        // This statement ensures the method only resolves if the type derivation matches the constraints placed on the type variables.
        final Stream<String> satisfiedPredicates = this.methodGenerics.entrySet().stream()
                .map(methodGeneric -> {
                    return new ST(StCtx.BUILTINS, "::java::type_traits::is_satisfied_by_v<$boundTemplateType(genericType, \"style=type\")$, $genericName$>")
                            .add("genericName", genericNameToDerivationName(methodGeneric.getKey()))
                            .add("genericType", methodGeneric.getValue())
                            .render(Locale.ROOT);
                });
        final Stream<String> assignmentPredicates = Streams.zip(
                this.bindingsOverrides.stream(),
                argumentTypes.stream(),
                (override, generic) -> {
                    return override
                            .map(BoundTemplate.VarBinding::getName)
                            .map(bindingName -> {
                                final boolean wrapConst;
                                final BoundTemplate genericTemplate;
                                if (generic instanceof ConstType) {
                                    genericTemplate = (BoundTemplate) ((ConstType) generic).getType();
                                    wrapConst = true;
                                } else {
                                    genericTemplate = (BoundTemplate) generic;
                                    wrapConst = false;
                                }

                                final BoundTemplate reboundType = genericTemplate.rebind(genericNameToDerivationNameRebindMap);
                                return new ST(StCtx.BUILTINS, "::std::is_constructible_v<$boundTemplateType(dstType, \"style=type, class\")$, ::std::add_rvalue_reference_t<$argName$>>")
                                        .add("argName", bindingName)
                                        .add("dstType", (wrapConst ? new ConstType(reboundType) : reboundType))
                                        .render(Locale.ROOT);
                            });
                })
                .flatMap(Streams::stream);
        final String enablerStatement = Stream.concat(satisfiedPredicates, assignmentPredicates)
                .collect(Collectors.joining(" && ", "::std::enable_if_t<", ">"));

        // Compute the list of template argument defaults.
        // The template arguments derived from the function arguments are left unspecified.
        functionGenericsDefault = Streams.concat(
                this.methodGenerics.keySet().stream().map(genericName -> "void"), // We default the front arguments to `void`, to signal they should be derived.
                this.bindingsOverrides.stream().flatMap(Streams::stream).map(boundTemplate -> null), // We need the null values to signal absense of default. These are derived from the method.
                this.derivationMap.values().stream().map(derivationRules -> {
                    // XXX Specialize a case for if derivationRules is empty: the type derivation should be `any`.
                    // XXX Specialize a case for if derivationRules has 2 or more rules: the type derivation should be the `common type`.
                    return derivationRules.stream()
                            .collect(MoreCollectors.onlyElement());
                }),
                Stream.of(enablerStatement))
                .collect(Collectors.collectingAndThen(
                        Collectors.toList(),
                        Collections::unmodifiableList));
    }

    public MethodGenerics(Context ctx, Collection<String> variables, Map<String, String> rawMethodGenerics, BoundTemplate.ClassBinding<ClassType> thisType, List<Type> argumentTypes) {
        this(initializer(requireNonNull(ctx), requireNonNull(variables), requireNonNull(rawMethodGenerics), thisType), argumentTypes);
    }

    /**
     * Test if this has generics.
     *
     * @return True if this uses generics, false otherwise.
     */
    public boolean isGenericsMethod() {
        return !methodGenerics.isEmpty();
    }

    /**
     * Get generics declared by this method.
     *
     * @return Map of generics declared by this method.
     */
    public final Map<String, BoundTemplate> getMethodGenerics() {
        return methodGenerics;
    }

    /**
     * Get erased generics declared by this method.
     *
     * @return Map of generics, with any variable resolved to exclude variables
     * in the method generics.
     */
    public final Map<String, BoundTemplate> getErasedMethodGenerics() {
        return erasedMethodGenerics;
    }

    public final List<String> getFunctionGenericsNames() {
        return functionGenericsNames;
    }

    public final List<String> getFunctionGenericsDefault() {
        return functionGenericsDefault;
    }

    public final Map<String, BoundTemplate.VarBinding> getGenericNameToDerivationNameRebindMap() {
        return genericNameToDerivationNameRebindMap;
    }

    public List<Type> getTemplatedArgumentTypes() {
        return templatedArgumentTypes;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 71 * hash + Objects.hashCode(this.methodGenerics);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final MethodGenerics other = (MethodGenerics) obj;
        if (!Objects.equals(this.methodGenerics, other.methodGenerics))
            return false;
        return true;
    }

    /**
     * Compute argument derivation name based on real template name.
     *
     * @param name The name of a method generic variable.
     * @return Mangled name that is used to hold derived type.
     */
    private static String genericNameToDerivationName(String name) {
        return String.format(Locale.ROOT, "__DerivedGeneric_%s__", name);
    }

    /**
     * Figure out initializer from contextual map.
     *
     * @param ctx Context for type lookup.
     * @param variables List of variable names that are in scope (excludes
     * method generics).
     * @param rawMethodGenerics Raw mapping of generics.
     * @param thisType Type used when resolving this.
     * @return Map of method generics.
     */
    public static Map<String, BoundTemplate> initializer(Context ctx, Collection<String> variables, Map<String, String> rawMethodGenerics, BoundTemplate.ClassBinding<ClassType> thisType) {
        final Map<String, BoundTemplate.VarBinding> allVariablesUnbound = Stream.concat(variables.stream(), rawMethodGenerics.keySet().stream())
                .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));

        return rawMethodGenerics.entrySet().stream()
                .collect(
                        Collectors.collectingAndThen(
                                Collectors.toMap(
                                        Map.Entry::getKey,
                                        genericsEntry -> {
                                            final BoundTemplate result;
                                            if (genericsEntry.getValue() == null)
                                                result = new BoundTemplate.Any();
                                            else
                                                result = (BoundTemplate) BoundTemplate.fromString(genericsEntry.getValue(), ctx, allVariablesUnbound, thisType);
                                            return result;
                                        },
                                        (u, v) -> {
                                            throw new IllegalStateException(String.format("Duplicate key %s", u));
                                        },
                                        LinkedHashMap::new),
                                Collections::unmodifiableMap));
    }

    /**
     * Figure out the order in which arguments need to be resolved.
     *
     * @param map Map of method generics.
     * @return List describing the order in which each variable is to be
     * resolved.
     * @throws IllegalStateException if any of the variables require recursion
     * to resolve.
     */
    private static List<String> resolutionOrder(Map<String, ? extends BoundTemplate> map) {
        final List<String> resolutionOrder = new ArrayList<>(map.size());
        map.forEach(new BiConsumer<String, Type>() {
            private final Set<String> active = new HashSet<>();
            private final Set<String> done = new HashSet<>();

            @Override
            public void accept(String name, Type type) {
                requireNonNull(name);
                requireNonNull(type);

                if (!active.add(name))
                    throw new IllegalStateException("recursion while evaluating type " + name);
                try {
                    if (done.add(name)) { // If this variable hasn't been added yet.
                        type.getUnresolvedTemplateNames().stream()
                                .filter(map::containsKey) // Only process method generics. Class or otherwise supplied variables are untouched.
                                .forEach(unresolvedName -> accept(unresolvedName, map.get(unresolvedName)));

                        resolutionOrder.add(name);
                    }
                } finally {
                    active.remove(name);
                }
            }
        });

        assert resolutionOrder.size() == map.size() : "resolutionOrder should contain same number of elements";
        return resolutionOrder;
    }

    /**
     * Resolve method generics, where they depend on eachother.
     *
     * The returned map should have no dependencies on generics declared at the
     * function level.
     *
     * @param map Raw mapping of method generics.
     * @return Same map, but any mappings to variables in the mapping have been
     * resolved.
     */
    private static Map<String, BoundTemplate> squashMethodTypes(Map<String, ? extends BoundTemplate> map) {
        final LinkedHashMap<String, BoundTemplate> resolutionMap = new LinkedHashMap<>(map);
        for (final String name : resolutionOrder(map)) {
            final BoundTemplate template = requireNonNull(map.get(name));
            final BoundTemplate resolvedTemplate = template.rebind(resolutionMap);
            resolutionMap.put(name, resolvedTemplate);
        }

        assert resolutionMap.size() == map.size();
        assert Arrays.equals(resolutionMap.keySet().toArray(), map.keySet().toArray()) : "Input map and output map have the same ordering of keys";
        return resolutionMap;
    }

    /**
     * Figure out which selectors are provided by a given type.
     *
     * Note: this method is called from the constructor. It requires that the
     * {@link #methodGenerics methodGenerics} variable is initialized prior to
     * being invoked.
     *
     * @param type A type which may or may not be using any method generics.
     * @return Multimap containing all method generics provided by this type.
     */
    private Multimap<String, TemplateSelector> selectorsForArg(Type type) {
        if (!type.isJavaType())
            return Multimaps.unmodifiableMultimap(Multimaps.forMap(EMPTY_MAP));

        final BoundTemplate templateType;
        if (type instanceof ConstType)
            templateType = (BoundTemplate) ((ConstType) type).getType();
        else
            templateType = (BoundTemplate) type;

        return this.methodGenerics.keySet().stream()
                .flatMap(methodGeneric -> {
                    return templateType.findSelectorFor(methodGeneric)
                            .map(selector -> new SimpleMapEntry<>(methodGeneric, selector));
                })
                .collect(Collectors.collectingAndThen(
                        Multimaps.toMultimap(Map.Entry::getKey, Map.Entry::getValue, () -> Multimaps.newSetMultimap(new HashMap<>(), HashSet::new)),
                        Multimaps::unmodifiableMultimap));
    }

    /**
     * Raw supplied method generics.
     *
     * Each generic has a name (the key of the map) and constraints (the value
     * of the map).
     */
    private final Map<String, BoundTemplate> methodGenerics;
    /**
     * Method generics, with any reference of the method generics expanded.
     *
     * This map holds no references to variables in its key set.
     */
    private final Map<String, BoundTemplate> erasedMethodGenerics;
    /**
     * Argument types, where each argument that supplies a generic has been
     * replaced with an rvalue template type.
     */
    private final List<Type> templatedArgumentTypes;
    /**
     * Describes the variable that is used for any argument that supplies the
     * input to compute method generics. Empty optionals describe arguments that
     * do no aid in generics resolution.
     */
    private final List<Optional<BoundTemplate.VarBinding>> bindingsOverrides;
    /**
     * Derivation map. Maps a method generic to an expression to derive its
     * generic type.
     */
    private final Map<String, List<String>> derivationMap;
    /**
     * List of method template arguments.
     */
    private final List<String> functionGenericsNames;
    /**
     * Defaults for each template argument.
     *
     * Note that this list will contain {@code null} when there is no default
     * for an argument.
     */
    private final List<String> functionGenericsDefault;
    /**
     * Variable map that rebinds the supplied variable names to their derived
     * counterparts.
     */
    private final Map<String, BoundTemplate.VarBinding> genericNameToDerivationNameRebindMap;
}
