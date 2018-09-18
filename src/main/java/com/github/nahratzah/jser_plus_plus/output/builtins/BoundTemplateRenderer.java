package com.github.nahratzah.jser_plus_plus.output.builtins;

import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.PrimitiveType;
import java.util.Arrays;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.AttributeRenderer;

/**
 * Renderer for {@link BoundTemplate }.
 *
 * @author ariane
 */
public class BoundTemplateRenderer implements AttributeRenderer {
    private static final Logger LOG = Logger.getLogger(BoundTemplateRenderer.class.getName());
    public static final Class<BoundTemplate> ATTRIBUTE_CLASS = BoundTemplate.class;

    /**
     * Render a {@link BoundTemplate}.
     *
     * @param o An instance of {@link BoundTemplate}.
     * @param formatString A string containing the configuration of the
     * renderer.
     * @param locale Locale in which to render. This argument is unused.
     * @return Rendered form of the {@link BoundTemplate}, according to options
     * specified in the format string.
     */
    @Override
    public String toString(Object o, String formatString, Locale locale) {
        LOG.log(Level.FINEST, "Invoking {0}(Object={1}, formatString={2})", new Object[]{this.getClass(), o, formatString});
        if (formatString == null) formatString = "";
        final Config config = new Config(formatString);
        final BoundTemplate tmpl = (BoundTemplate) o;

        return tmpl.visit(config.style.apply(config));
    }

    /**
     * Configuration of the {@link BoundTemplateRenderer}.
     */
    private static class Config {
        /**
         * Create a {@link Style#TYPE} configuration, with specified accessor
         * base.
         *
         * @param base The base of an accessor.
         * @return Configuration that renders types.
         */
        public static Config typeWithBase(String base) {
            final Config config = new Config(Style.TYPE);
            config.base = base;
            return config;
        }

        /**
         * Construct a configuration with the given style.
         *
         * @param style A configuration style.
         */
        private Config(Style style) {
            this.style = requireNonNull(style);
        }

        /**
         * Populate configuration from format string.
         *
         * @param formatString A format string used by the template engine.
         */
        public Config(String formatString) {
            if (formatString.isEmpty()) return;

            Arrays.stream(formatString.split(Pattern.quote(",")))
                    .map(string -> string.split(Pattern.quote("="), 2))
                    .forEach(args -> {
                        if (args.length == 2)
                            apply(args[0].trim(), args[1].trim());
                        else
                            apply(args[0].trim());
                    });

            LOG.log(Level.FINEST, "Parsed format string \"{0}\" into {1}", new Object[]{formatString, this});
            if (accessorType && classType != null)
                throw new IllegalArgumentException("Must specify at most one of 'accessor' and 'class' in the formatter.");
        }

        /**
         * Style of decoding.
         */
        public Style style = Style.TYPE;
        /**
         * If the style is {@link Style#TYPE}, create class types instead of
         * generics, by applying the wrapper.
         *
         * If this is null, it indicates not to create class types.
         */
        public ClassWrapper classType = null;
        /**
         * If the style is {@link Style#TYPE}, create accessor types instead of
         * generics.
         */
        public boolean accessorType = false;
        /**
         * If rendering accessors, use this as the base template argument to the
         * accessor.
         */
        public String base;
        /**
         * If set and emitting class types, render a reference to const type.
         */
        public boolean emitConst = false;

        /**
         * Apply configuration key-value item.
         *
         * @param key The key of the configuration option.
         * @param value The value of the configuration option.
         */
        private void apply(String key, String value) {
            if (Objects.equals(key, "style"))
                style = Style.valueOf(value.toUpperCase(Locale.ROOT));
            else if (Objects.equals(key, "base"))
                base = value;
            else if (Objects.equals(key, "class"))
                classType = ClassWrapper.valueOf(value.toUpperCase(Locale.ROOT));
            else
                throw new IllegalArgumentException("Invalid key \"" + key + "\"");
        }

        /**
         * Apply configuration item.
         *
         * @param key A no-argument configuration option.
         */
        private void apply(String key) {
            if (Objects.equals(key, "class"))
                classType = ClassWrapper.IDENTITY;
            else if (Objects.equals(key, "accessor"))
                accessorType = true;
            else if (Objects.equals(key, "const"))
                emitConst = true;
            else
                throw new IllegalArgumentException("Invalid key \"" + key + "\"");
        }

        @Override
        public int hashCode() {
            int hash = 5;
            hash = 89 * hash + Objects.hashCode(this.style);
            hash = 89 * hash + Objects.hashCode(this.classType);
            hash = 89 * hash + (this.accessorType ? 1 : 0);
            hash = 89 * hash + Objects.hashCode(this.base);
            hash = 89 * hash + (this.emitConst ? 1 : 0);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Config other = (Config) obj;
            if (this.accessorType != other.accessorType) return false;
            if (this.emitConst != other.emitConst) return false;
            if (!Objects.equals(this.base, other.base)) return false;
            if (this.style != other.style) return false;
            if (this.classType != other.classType) return false;
            return true;
        }

        @Override
        public String toString() {
            return "Config{" + "style=" + style + ", classType=" + classType + ", accessorType=" + accessorType + ", base=" + base + '}';
        }
    }

    private static enum ClassWrapper {
        IDENTITY("", ""),
        RETURN("::java::return_t<", ">"),
        PARAM("::java::param_t<", ">"),
        FIELD("::java::field_t<", ">");

        private ClassWrapper(String prefix, String suffix) {
            this.prefix = requireNonNull(prefix);
            this.suffix = requireNonNull(suffix);
        }

        private final String prefix, suffix;

        public String apply(String s, boolean emitConst) {
            return prefix + (emitConst ? "const " : "") + s + suffix;
        }
    }

    /**
     * Render style information.
     */
    private static enum Style implements Function<Config, BoundTemplate.Visitor<String>> {
        /**
         * Render style for types. The entire {@link BoundTemplate} will be
         * evaluated.
         */
        TYPE(TypeVisitor::new),
        /**
         * Render style for tags. Only the root of the {@link BoundTemplate}
         * will be evaluated.
         */
        TAG(TagVisitor::new),
        /**
         * Render style for erased types. Only the root of the
         * {@link BoundTemplate} will be evaluated.
         */
        ERASED(ErasedVisitor::new);

        private Style(Function<Config, BoundTemplate.Visitor<String>> visitor) {
            this.visitor = visitor;
        }

        /**
         * Retrieve the render implementation with a given configuration.
         *
         * @param config The configuration for the render operation.
         * @return A visitor that renders the {@link BoundTemplate}.
         */
        @Override
        public BoundTemplate.Visitor<String> apply(Config config) {
            return visitor.apply(config);
        }

        /**
         * Visitor factory.
         */
        private final Function<Config, BoundTemplate.Visitor<String>> visitor;
    }

    private static class TagVisitor implements BoundTemplate.Visitor<String> {
        public TagVisitor(Config config) {
            this.config = requireNonNull(config);
        }

        @Override
        public String apply(BoundTemplate.VarBinding b) {
            throw new UnsupportedOperationException("Variable can not be converted to tag.");
        }

        @Override
        public String apply(BoundTemplate.ClassBinding b) {
            return "::java::_tags"
                    + b.getType().getNamespace().stream().map(nsElem -> "::" + nsElem).collect(Collectors.joining())
                    + "::"
                    + b.getType().getClassName();
        }

        @Override
        public String apply(BoundTemplate.ArrayBinding b) {
            throw new UnsupportedOperationException("Arrays can not be converted to tag.");
        }

        @Override
        public String apply(BoundTemplate.Any b) {
            throw new UnsupportedOperationException("Unbound template variable can not be converted to tag.");
        }

        private final Config config;
    }

    private static class ErasedVisitor implements BoundTemplate.Visitor<String> {
        public ErasedVisitor(Config config) {
            this.config = requireNonNull(config);
        }

        @Override
        public String apply(BoundTemplate.VarBinding b) {
            throw new UnsupportedOperationException("Variable can not be converted to erased type.");
        }

        @Override
        public String apply(BoundTemplate.ClassBinding b) {
            return "::java::_erased"
                    + b.getType().getNamespace().stream().map(nsElem -> "::" + nsElem).collect(Collectors.joining())
                    + "::"
                    + b.getType().getClassName();
        }

        @Override
        public String apply(BoundTemplate.ArrayBinding b) {
            throw new UnsupportedOperationException("Arrays can not be converted to erased type.");
        }

        @Override
        public String apply(BoundTemplate.Any b) {
            throw new UnsupportedOperationException("Unbound template variable can not be converted to erased type.");
        }

        private final Config config;
    }

    private static class TypeVisitor implements BoundTemplate.Visitor<String> {
        public TypeVisitor(Config config) {
            this.config = requireNonNull(config);
            final Config rawConfig = Config.typeWithBase(config.base);
            if (config.classType != null)
                rawConfig.classType = ClassWrapper.IDENTITY;

            if (Objects.equals(rawConfig, config))
                this.raw = this;
            else
                this.raw = new TypeVisitor(rawConfig);
        }

        @Override
        public String apply(BoundTemplate.VarBinding b) {
            if (config.classType != null)
                return config.classType.apply("::java::type<" + b.getName() + ">", config.emitConst);
            if (config.accessorType)
                throw new IllegalArgumentException("Variables do not have accessors.");
            return b.getName();
        }

        @Override
        public String apply(BoundTemplate.ClassBinding b) {
            final String clsType = b.getType().getNamespace().stream().map(nsElem -> "::" + nsElem).collect(Collectors.joining())
                    + "::"
                    + b.getType().getClassName();

            final Stream<String> bindings = b.getBindings().stream()
                    .map(tmpl -> tmpl.visit(raw));

            if (config.classType != null) {
                String result;
                if (b.getBindings().isEmpty())
                    result = clsType;
                else
                    result = clsType + bindings.collect(Collectors.joining(", ", "<", ">"));
                if (b.getType() instanceof PrimitiveType) return result;
                return config.classType.apply(result, config.emitConst);
            }

            final String tagName = "::java::_tags" + clsType;

            if (config.accessorType) {
                return "::java::_accessor" + Stream.concat(Stream.of(config.base, tagName), bindings).collect(Collectors.joining(", ", "<", ">"));
            } else {
                return "::java::G::is" + Stream.concat(Stream.of(tagName), bindings).collect(Collectors.joining(", ", "<", ">"));
            }
        }

        @Override
        public String apply(BoundTemplate.ArrayBinding b) {
            final String elementType;
            if (b.getType() instanceof BoundTemplate.ClassBinding && ((BoundTemplate.ClassBinding) b.getType()).getType() instanceof PrimitiveType) {
                final PrimitiveType primitiveType = (PrimitiveType) ((BoundTemplate.ClassBinding) b.getType()).getType();
                elementType = Stream.concat(primitiveType.getNamespace().stream(), Stream.of(primitiveType.getClassName()))
                        .map(nameElem -> "::" + nameElem)
                        .collect(Collectors.joining());
            } else {
                elementType = b.getType().visit(raw);
            }

            final String arrayObj = "::java::array_type<"
                    + elementType
                    + ", "
                    + b.getExtents()
                    + ">";

            if (config.classType != null)
                return config.classType.apply(arrayObj, config.emitConst);
            if (config.accessorType)
                throw new IllegalArgumentException("Arrays do not have accessors.");
            return "::java::type_of<" + arrayObj + ">";
        }

        @Override
        public String apply(BoundTemplate.Any b) {
            final Stream<String> extendStream = b.getExtendTypes().stream()
                    .map(type -> type.visit(raw))
                    .map(typeStr -> "::java::G::extends<" + typeStr + ">");
            final Stream<String> superStream = b.getSuperTypes().stream()
                    .map(type -> type.visit(raw))
                    .map(typeStr -> "::java::G::super<" + typeStr + ">");
            final String type = "::java::G::pack"
                    + Stream.concat(extendStream, superStream).collect(Collectors.joining(", ", "<", ">"));

            if (config.classType != null)
                return config.classType.apply("::java::type<" + type + ">", config.emitConst);
            if (config.accessorType)
                throw new IllegalArgumentException("Unbound typres do not have accessors.");
            return type;
        }

        private final Config config;
        private final TypeVisitor raw;
    }
}
