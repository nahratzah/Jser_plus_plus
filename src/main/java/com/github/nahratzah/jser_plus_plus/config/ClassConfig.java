package com.github.nahratzah.jser_plus_plus.config;

import java.util.Collection;
import static java.util.Collections.unmodifiableMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Queue;
import java.util.Set;
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
        requireNonNull(matchMethods, "ClassConfig.updateWithSuperTypes() has not been run!");
        return Stream.concat(cfgClass.getMembers().stream(), matchMethods.stream());
    }

    public CfgType getVarType() {
        return cfgClass.getVarType();
    }

    public List<CfgType> getFriends() {
        return cfgClass.getFriends();
    }

    public String getDocString() {
        return cfgClass.getDocString();
    }

    public boolean isDevMode() {
        return cfgClass.getDevMode() == null ? cfg.isDevMode() : cfgClass.getDevMode();
    }

    public void updateWithSuperTypes(boolean isAbstract, boolean isInterface, boolean isEnum, CfgSuperType thisType) {
        matchMethods = cfg.getMatchMethods().stream()
                .filter(matchMethod -> matchMethod.getPredicate().test(isAbstract, isInterface, isEnum, thisType.getAllParentTypes().stream().map(x -> x.getName()).collect(Collectors.toSet())))
                .map(matchMethod -> matchMethod.getMembers())
                .flatMap(Collection::stream)
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
    private List<ClassMember> matchMethods;
}
