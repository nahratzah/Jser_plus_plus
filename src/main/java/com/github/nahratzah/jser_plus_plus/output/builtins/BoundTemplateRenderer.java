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

    @Override
    public String toString(Object o, String formatString, Locale locale) {
        LOG.log(Level.FINEST, "Invoking {0}(Object={1}, formatString={2})", new Object[]{this.getClass(), o, formatString});
        if (formatString == null) formatString = "";
        final Config config = new Config(formatString);
        final BoundTemplate tmpl = (BoundTemplate) o;
        return tmpl.visit(config.style.apply(config));
    }

    private static class Config {
        public static Config typeWithBase(String base) {
            Config config = new Config(Style.TYPE);
            config.base = base;
            return config;
        }

        private Config(Style style) {
            this.style = requireNonNull(style);
        }

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
        }

        public Style style = Style.TYPE;
        public boolean classType = false;
        public boolean accessorType = false;
        public String base;

        private void apply(String key, String value) {
            if (Objects.equals(key, "style"))
                style = Style.valueOf(value.toUpperCase(Locale.ROOT));
            else if (Objects.equals(key, "base"))
                base = value;
            else
                throw new IllegalArgumentException("Invalid key \"" + key + "\"");
        }

        private void apply(String key) {
            if (Objects.equals(key, "class"))
                classType = true;
            else if (Objects.equals(key, "accessor"))
                accessorType = true;
            else
                throw new IllegalArgumentException("Invalid key \"" + key + "\"");
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 97 * hash + Objects.hashCode(this.style);
            hash = 97 * hash + (this.classType ? 1 : 0);
            hash = 97 * hash + (this.accessorType ? 1 : 0);
            hash = 97 * hash + Objects.hashCode(this.base);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Config other = (Config) obj;
            if (this.classType != other.classType) return false;
            if (this.accessorType != other.accessorType) return false;
            if (!Objects.equals(this.base, other.base)) return false;
            if (this.style != other.style) return false;
            return true;
        }

        @Override
        public String toString() {
            return "Config{" + "style=" + style + ", classType=" + classType + ", accessorType=" + accessorType + ", base=" + base + '}';
        }
    }

    private static enum Style implements Function<Config, BoundTemplate.Visitor<String>> {
        TYPE(TypeVisitor::new),
        TAG(TagVisitor::new),
        ERASED(ErasedVisitor::new);

        private Style(Function<Config, BoundTemplate.Visitor<String>> visitor) {
            this.visitor = visitor;
        }

        @Override
        public BoundTemplate.Visitor<String> apply(Config config) {
            return visitor.apply(config);
        }

        private final Function<Config, BoundTemplate.Visitor<String>> visitor;

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
                        + b.getType().getSimpleName();
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
        };

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
                        + b.getType().getSimpleName();
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
        };

        private static class TypeVisitor implements BoundTemplate.Visitor<String> {
            public TypeVisitor(Config config) {
                this.config = requireNonNull(config);
                final Config rawConfig = Config.typeWithBase(config.base);
                if (Objects.equals(rawConfig, config))
                    this.raw = this;
                else
                    this.raw = new TypeVisitor(rawConfig);
            }

            @Override
            public String apply(BoundTemplate.VarBinding b) {
                if (config.classType)
                    return "::java::type<" + b.getName() + ">";
                if (config.accessorType)
                    throw new IllegalArgumentException("Variables do not have accessors.");
                return b.getName();
            }

            @Override
            public String apply(BoundTemplate.ClassBinding b) {
                final String clsType = b.getType().getNamespace().stream().map(nsElem -> "::" + nsElem).collect(Collectors.joining())
                        + "::"
                        + b.getType().getSimpleName();

                final Stream<String> bindings = b.getBindings().stream()
                        .map(tmpl -> tmpl.visit(raw));

                if (config.classType) {
                    if (b.getBindings().isEmpty()) return clsType;
                    return clsType + bindings.collect(Collectors.joining(", ", "<", ">"));
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
                    elementType = Stream.concat(primitiveType.getNamespace().stream(), Stream.of(primitiveType.getSimpleName()))
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

                if (config.classType) return arrayObj;
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

                if (config.classType) return "::java::type<" + type + ">";
                if (config.accessorType)
                    throw new IllegalArgumentException("Unbound typres do not have accessors.");
                return type;
            }

            private final Config config;
            private final TypeVisitor raw;
        };
    }
}
