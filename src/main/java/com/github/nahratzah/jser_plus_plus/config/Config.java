package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.google.common.collect.Sets;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.function.Predicate;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Config {
    private static final Logger LOG = Logger.getLogger(Config.class.getName());

    public static Config fromFile(File cfgFile) throws IOException {
        try (InputStream configStream = new FileInputStream(cfgFile)) {
            final ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
            return mapper.readValue(configStream, Config.class);
        }
    }

    public static Config fromDir(File dir) throws IOException {
        final Config config = new Config();
        final boolean any;

        try (Stream<Path> pathStream = Files.find(dir.toPath(), Integer.MAX_VALUE, (path, attr) -> {
            return attr.isRegularFile()
                    && path.toString().endsWith(".yaml");
        })) {
            final Iterator<File> fileIter = pathStream
                    .map(Path::toFile)
                    .iterator();
            any = fileIter.hasNext();
            while (fileIter.hasNext())
                config.merge(fromFile(fileIter.next()));
        }

        if (!any) throw new IOException("no config files in directory: " + dir);
        return config;
    }

    /**
     * Merge two configs together into a single, new config.
     *
     * @param x A configuration to merge.
     * @param y Another configuration to merge.
     * @return A newly constructed config that is the merge of the arguments.
     */
    public static Config merge(Config x, Config y) {
        final Config result = new Config();
        result.merge(x);
        result.merge(y);
        return result;
    }

    /**
     * Merge other config into this one.
     *
     * @param y The other config to merge into this one.
     */
    public void merge(Config y) {
        if (module != null && y.module != null && !Objects.equals(module, y.module))
            throw new IllegalArgumentException("conflicting module name");
        if (module == null) module = y.module;

        if (scan != null && y.scan != null)
            throw new IllegalArgumentException("duplicate declaration of scanner");
        if (scan == null) scan = y.scan;

        final Set<ClassName> duplicateClasses = Sets.intersection(classes.keySet(), y.classes.keySet());
        if (!duplicateClasses.isEmpty()) {
            LOG.log(Level.SEVERE, () -> {
                return "duplicate classes:\n"
                        + duplicateClasses.stream()
                                .map(c -> "  " + c)
                                .collect(Collectors.joining("\n"));
            });
            throw new IllegalArgumentException("duplicate classes");
        }
        classes.putAll(y.classes);

        rules.addAll(y.rules);
        devMode |= y.devMode;
    }

    /**
     * Get the name of the module that is being configured.
     *
     * @return Module name.
     */
    public String getModule() {
        return module;
    }

    /**
     * Set the name of the module that is being configured.
     *
     * @param module Module name.
     */
    public void setModule(String module) {
        this.module = module;
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

    public List<Rule> getRules() {
        return rules;
    }

    public void setRules(List<Rule> rules) {
        this.rules = rules;
    }

    public boolean isDevMode() {
        return devMode;
    }

    public void setDevMode(boolean devMode) {
        this.devMode = devMode;
    }

    @JsonProperty(value = "module")
    private String module;

    @JsonProperty(value = "scan")
    private Scan scan;

    @JsonProperty(value = "classes")
    private Map<ClassName, CfgClass> classes = new HashMap<>();

    @JsonProperty("rules")
    private List<Rule> rules = new ArrayList<>();

    @JsonProperty("dev_mode")
    private boolean devMode = false;
}
