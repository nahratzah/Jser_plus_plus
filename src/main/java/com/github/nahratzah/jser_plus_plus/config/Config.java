package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Config {
    @JsonProperty("match_methods")
    private List<MatchMethod> matchMethods = new ArrayList<>();

    public Stream<MatchMethod> getMatchMethods(Class<?> c) {
        return matchMethods.stream()
                .filter(mm -> mm.getPredicate().test(c));
    }
}
