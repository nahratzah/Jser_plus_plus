package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.function.Predicate;
import java.util.stream.Collectors;

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

    /**
     * Retrieve mapping of all configured classes.
     *
     * @return Mapping of class configurations.
     */
    public Map<ClassName, CfgClass> getClasses() {
        return classes;
    }

    /**
     * Set mapping of all configured classes.
     *
     * @param classes Mapping of class configurations.
     */
    public void setClasses(Map<ClassName, CfgClass> classes) {
        this.classes = classes;
    }

    public Optional<List<String>> getTemplateArguments(String className) {
        return getClasses().keySet().stream()
                .filter(key -> Objects.equals(key.getName(), className))
                .map(key -> key.getTemplateArgumentNames())
                .findAny();
    }

    /**
     * Predicate for filtering if a class is declared explicitly in the config.
     *
     * @return Predicate that tests if a class is explicitly mentioned in the
     * config.
     */
    public Predicate<Class<?>> hasConfigForClass() {
        return getClasses().keySet().stream()
                .map(ClassName::getName)
                .collect(Collectors.toSet())::contains;
    }

    public ClassConfig getConfigForClass(Class<?> c) {
        final CfgClass cfgClass = getClasses().entrySet().stream()
                .filter(entry -> Objects.equals(entry.getKey().getName(), c.getName()))
                .map(entry -> entry.getValue())
                .filter(Objects::nonNull)
                .findAny()
                .orElseGet(CfgClass::new);

        return new ClassConfig(this, cfgClass);
    }

    public List<MatchMethod> getMatchMethods() {
        return matchMethods;
    }

    public void setMatchMethods(List<MatchMethod> matchMethods) {
        this.matchMethods = matchMethods;
    }

    public boolean isDevMode() {
        return devMode;
    }

    public void setDevMode(boolean devMode) {
        this.devMode = devMode;
    }

    @JsonProperty(value = "scan", required = true)
    private Scan scan;

    @JsonProperty(value = "classes")
    private Map<ClassName, CfgClass> classes = new HashMap<>();

    @JsonProperty("rules")
    private List<MatchMethod> matchMethods = new ArrayList<>();

    @JsonProperty("dev_mode")
    private boolean devMode = false;
}
