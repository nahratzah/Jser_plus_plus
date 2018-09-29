package com.github.nahratzah.jser_plus_plus.config;

import java.util.Collection;
import static java.util.Collections.unmodifiableMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Queue;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Synthesized configuration from applying all filters.
 *
 * @author ariane
 */
public class ClassConfig {
    public ClassConfig(Config cfg, CfgClass cfgClass) {
        this.cfg = requireNonNull(cfg);
        this.cfgClass = requireNonNull(cfgClass);
    }

    public Map<String, CfgField> getFields() {
        return unmodifiableMap(cfgClass.getFields());
    }

    public Boolean isFinal() {
        return cfgClass.isFinal();
    }

    public Stream<ClassMember> getClassMembers() {
        requireNonNull(rules, "ClassConfig.updateWithSuperTypes() has not been run!");
        return Stream.concat(cfgClass.getMembers().stream(), rules.stream().map(Rule::getMembers).flatMap(Collection::stream));
    }

    public CfgType getVarType() {
        return cfgClass.getVarType();
    }

    public List<CfgType> getFriends() {
        requireNonNull(rules, "ClassConfig.updateWithSuperTypes() has not been run!");
        return Stream.concat(cfgClass.getFriends().stream(), rules.stream().map(Rule::getFriends).flatMap(Collection::stream))
                .collect(Collectors.toList());
    }

    public String getDocString() {
        return cfgClass.getDocString();
    }

    public boolean isDevMode() {
        return cfgClass.getDevMode() == null ? cfg.isDevMode() : cfgClass.getDevMode();
    }

    public void updateWithSuperTypes(boolean isAbstract, boolean isInterface, boolean isEnum, CfgSuperType thisType) {
        final Set<CfgSuperType> allParentTypes = thisType.getAllParentTypes();

        // Decide on which rules have matching predicate.
        final List<Rule> allMatchingRules = cfg.getRules().stream()
                .filter(rule -> {
                    return rule.getPredicate().test(
                            isAbstract, isInterface, isEnum,
                            allParentTypes.stream()
                                    .map(CfgSuperType::getName)
                                    .collect(Collectors.toSet()));
                })
                .collect(Collectors.toList());

        // Decide which rules are supporessed by this type or any type this inherits from.
        final Set<String> suppressedRulesFromTypes = Stream.of(
                cfgClass.getLocalSuppressedRules().stream(),
                cfgClass.getInheritSuppressedRules().stream(),
                allParentTypes.stream()
                        .map(parentType -> parentType.getName())
                        .map(parentName -> {
                            return cfg.getClasses().entrySet().stream()
                                    .filter(cls -> Objects.equals(cls.getKey().getName(), parentName))
                                    .map(Map.Entry::getValue)
                                    .map(CfgClass::getInheritSuppressedRules)
                                    .findAny()
                                    .orElse(null);
                        })
                        .filter(Objects::nonNull)
                        .flatMap(Collection::stream))
                .flatMap(Function.identity())
                .collect(Collectors.toSet());

        // Decide which rules suppress other rules.
        final Set<String> suppressedRulesFromRules = allMatchingRules.stream()
                .map(rule -> rule.getSuppressedRules())
                .flatMap(Collection::stream)
                .collect(Collectors.toSet());

        // Filter out all suppressed rules.
        rules = allMatchingRules.stream()
                .filter(rule -> !suppressedRulesFromTypes.contains(rule.getId()))
                .filter(rule -> !suppressedRulesFromRules.contains(rule.getId()))
                .collect(Collectors.toList());
    }

    /**
     * Helper type for
     * {@link #updateWithSuperTypes(boolean, boolean, boolean, java.util.Optional, java.util.Collection) updateWithSuperTypes}
     * to have a full class tree.
     */
    public static interface CfgSuperType {
        /**
         * Name of the type.
         *
         * @return A string of the form {@code "package.path.to.ClassName"}.
         */
        public String getName();

        /**
         * Get the super class of this type.
         *
         * @return Instance of {@link CfgSuperType} for the super class. Empty
         * instance if this type has no super class.
         */
        public Optional<CfgSuperType> getSuperClass();

        /**
         * Get all interfaces implemented/extended by this type.
         *
         * @return List of {@link CfgSuperType all interfaces} of this type.
         */
        public List<CfgSuperType> getInterfaces();

        /**
         * Recursively collect all super types if this.
         *
         * @return Set containing all super types.
         */
        public default Set<CfgSuperType> getAllParentTypes() {
            final Set<CfgSuperType> superTypes = new HashSet<>();

            final Queue<CfgSuperType> interfaces = Stream.concat(Stream.of(this.getSuperClass()).filter(Optional::isPresent).map(Optional::get), this.getInterfaces().stream())
                    .collect(Collectors.toCollection(LinkedList::new));
            for (CfgSuperType currentInterface = interfaces.poll();
                 currentInterface != null;
                 currentInterface = interfaces.poll()) {
                if (superTypes.add(currentInterface)) {
                    currentInterface.getSuperClass().ifPresent(interfaces::add);
                    interfaces.addAll(currentInterface.getInterfaces());
                }
            }

            return superTypes;
        }
    }

    private final Config cfg;
    private final CfgClass cfgClass;
    private List<Rule> rules;
}
