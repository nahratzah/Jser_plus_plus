package com.github.nahratzah.jser_plus_plus.config;

import static java.util.Collections.unmodifiableMap;
import java.util.Map;
import static java.util.Objects.requireNonNull;

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

    private final CfgClass cfgClass;
}
