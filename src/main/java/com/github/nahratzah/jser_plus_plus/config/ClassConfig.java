package com.github.nahratzah.jser_plus_plus.config;

import static java.util.Collections.unmodifiableMap;
import java.util.List;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.stream.Stream;

/**
 * Synthesized configuration from applying all filters.
 *
 * @author ariane
 */
public class ClassConfig {
    public ClassConfig(CfgClass cfgClass) {
        this.cfgClass = requireNonNull(cfgClass);
    }

    public Map<String, CfgField> getFields() {
        return unmodifiableMap(cfgClass.getFields());
    }

    public Boolean isFinal() {
        return cfgClass.isFinal();
    }

    public Stream<ClassMember> getClassMembers() {
        return cfgClass.getMembers().stream();
    }

    public CfgType getVarType() {
        return cfgClass.getVarType();
    }

    public List<CfgType> getFriends() {
        return cfgClass.getFriends();
    }

    private final CfgClass cfgClass;
}
