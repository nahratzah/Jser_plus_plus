package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.HashSet;
import java.util.OptionalInt;
import java.util.Set;
import java.util.function.Predicate;

/**
 * Scanner configuration.
 *
 * @author ariane
 */
public class Scan {
    public Set<String> getIncludePackages() {
        return includePackages;
    }

    public void setIncludePackages(Set<String> includePackages) {
        this.includePackages = includePackages;
    }

    public Set<String> getExcludePackages() {
        return excludePackages;
    }

    public void setExcludePackages(Set<String> excludePackages) {
        this.excludePackages = excludePackages;
    }

    public Predicate<Class<?>> filter() {
        return (Class<?> c) -> {
            final String name = c.getPackage().getName();

            final OptionalInt includeLen = includePackages.stream()
                    .filter(pkg -> name.startsWith(pkg))
                    .mapToInt(pkg -> pkg.length())
                    .max();
            if (!includeLen.isPresent()) return false;

            final OptionalInt excludeLen = excludePackages.stream()
                    .filter(pkg -> name.startsWith(pkg))
                    .mapToInt(pkg -> pkg.length())
                    .max();
            if (!excludeLen.isPresent()) return true;

            return includeLen.getAsInt() > excludeLen.getAsInt();
        };
    }

    @JsonProperty("include")
    private Set<String> includePackages = new HashSet<>();
    @JsonProperty("exclude")
    private Set<String> excludePackages = new HashSet<>();
}
