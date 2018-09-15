package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Config {
    public static Config fromFile(File cfgFile) throws IOException {
        try (InputStream configStream = new FileInputStream(cfgFile)) {
            final ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
            return mapper.readValue(configStream, Config.class);
        }
    }

    /**
     * Retrieve the package scan configuration.
     *
     * @return Package scanner configuration.
     */
    public Scan getScan() {
        return scan;
    }

    /**
     * Set the package scan configuration.
     *
     * @param scan New package scanner configuration.
     */
    public void setScan(Scan scan) {
        this.scan = scan;
    }

    public List<MatchMethod> getMatchMethods() {
        return matchMethods;
    }

    public void setMatchMethods(List<MatchMethod> matchMethods) {
        this.matchMethods = matchMethods;
    }

    public Stream<MatchMethod> getMatchMethods(Class<?> c) {
        return matchMethods.stream()
                .filter(mm -> mm.getPredicate().test(c));
    }

    @JsonProperty(value = "scan", required = true)
    private Scan scan;

    @JsonProperty("match_methods")
    private List<MatchMethod> matchMethods = new ArrayList<>();
}
