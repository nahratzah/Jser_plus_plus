package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassTemplateArgument;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.EnumType;
import com.github.nahratzah.jser_plus_plus.model.JavaType;
import com.github.nahratzah.jser_plus_plus.model.PrimitiveType;
import com.github.nahratzah.jser_plus_plus.output.CmakeModule;
import com.github.nahratzah.jser_plus_plus.output.CodeGenerator;
import static com.github.nahratzah.jser_plus_plus.output.Util.setFileContents;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Resolver for classes to their C++ counterpart.
 *
 * @author ariane
 */
public class Processor implements Context {
    private static final Logger LOG = Logger.getLogger(Processor.class.getName());

    public Processor(ClassLoader classLoader, Config cfg) {
        this.classLoader = requireNonNull(classLoader);
        this.cfg = requireNonNull(cfg);
    }

    @Override
    public ClassLoader getClassLoader() {
        return classLoader;
    }

    @Override
    public JavaType resolveClass(Class<?> c) {
        if (c.isPrimitive()) return PrimitiveType.fromClass(c);

        LOG.log(Level.FINE, "resolving {0}", c);

        if (c.isArray())
            throw new IllegalArgumentException("Arrays are not a resolvable type.");

        synchronized (classes) {
            /* scope */
            {
                final JavaType v = classes.get(c);
                if (v != null) return v;
            }

            final JavaType v;
            if (c.isEnum())
                v = new EnumType(c);
            else
                v = new ClassType(c);
            classes.put(c, v);
            v.init(this, cfg);
            return v;
        }
    }

    public void addClass(Class<?> c) {
        resolveClass(c);
    }

    public void addClass(String c) {
        resolveClass(c);
    }

    public void addClasses(Collection<Class<?>> c) {
        c.forEach(this::resolveClass);
    }

    public void addClassNames(Collection<String> c) {
        c.forEach(this::resolveClass);
    }

    /**
     * Registers all files that are to be emitted with the module and emits
     * those files.
     *
     * @param module The cmake module that will contain the project.
     * @throws IOException If any of the file write operations fails.
     */
    public void emit(CmakeModule module) throws IOException {
        final Map<List<String>, CodeGenerator> cgMap = new HashMap<>();
        classes.values().forEach(jc -> {
            final CodeGenerator.JavaClass cgJc = jcToCg(jc);
            cgMap.computeIfAbsent(CodeGenerator.computeBaseType(cgJc), baseType -> new CodeGenerator(baseType))
                    .add(cgJc);
        });

        for (final CodeGenerator cg : cgMap.values()) {
            setFileContents(
                    module.addHeader(cg.fwdHeaderName()).toPath(),
                    cg.writeFwdHeaderFile(new StringBuilder()).toString());
            setFileContents(
                    module.addHeader(cg.headerName()).toPath(),
                    cg.writeHeaderFile(new StringBuilder()).toString());
            setFileContents(
                    module.addSource(cg.sourceName()).toPath(),
                    cg.writeSourceFile(new StringBuilder()).toString());
        }
    }

    private static CodeGenerator.JavaClass jcToCg(JavaType jc) {
        return new CodeGenerator.JavaClass() {
            @Override
            public List<String> getNamespace() {
                return jc.getNamespace();
            }

            @Override
            public String getClassName() {
                return jc.getSimpleName();
            }

            @Override
            public List<String> getTemplateArguments() {
                return jc.getTemplateArguments().stream()
                        .map(ClassTemplateArgument::getName)
                        .collect(Collectors.toList());
            }

            @Override
            public Collection<CodeGenerator.JavaClass> getDependentTypes(boolean publicOnly) {
                final BoundTemplate.Visitor<Stream<JavaType>> templateVisitor = new BoundTemplate.Visitor<Stream<JavaType>>() {
                    @Override
                    public Stream<JavaType> apply(BoundTemplate.VarBinding b) {
                        return Stream.empty();
                    }

                    @Override
                    public Stream<JavaType> apply(BoundTemplate.ClassBinding b) {
                        return Stream.concat(
                                Stream.of(b.getType()),
                                b.getBindings().stream().flatMap(x -> x.visit(this)));
                    }

                    @Override
                    public Stream<JavaType> apply(BoundTemplate.ArrayBinding b) {
                        return b.getType().visit(this);
                    }

                    @Override
                    public Stream<JavaType> apply(BoundTemplate.Any b) {
                        return Stream.empty();
                    }
                };

                final Stream<BoundTemplate> superTypes = Stream.concat(
                        Stream.of(jc.getSuperClass()).filter(Objects::nonNull),
                        jc.getInterfaces().stream());

                final Stream<BoundTemplate> fields;
                if (publicOnly) {
                    fields = Stream.empty();
                } else {
                    fields = jc.getFields().stream()
                            .flatMap(field -> Stream.of(field.getType(), field.getVarType()));
                }

                return Stream.of(superTypes, fields)
                        .flatMap(Function.identity())
                        .flatMap(c -> c.visit(templateVisitor))
                        .distinct()
                        .filter(c -> !(c instanceof PrimitiveType))
                        .map(Processor::jcToCg)
                        .collect(Collectors.toList());
            }

            @Override
            public Collection<JavaType> getParentModels() {
                return Stream.concat(Stream.of(jc.getSuperClass()).filter(Objects::nonNull), jc.getInterfaces().stream())
                        .flatMap(c -> {
                            return c.visit(new BoundTemplate.Visitor<Stream<JavaType>>() {
                                @Override
                                public Stream<JavaType> apply(BoundTemplate.VarBinding b) {
                                    return Stream.empty();
                                }

                                @Override
                                public Stream<JavaType> apply(BoundTemplate.ClassBinding b) {
                                    return Stream.of(b.getType());
                                }

                                @Override
                                public Stream<JavaType> apply(BoundTemplate.ArrayBinding b) {
                                    return b.getType().visit(this);
                                }

                                @Override
                                public Stream<JavaType> apply(BoundTemplate.Any b) {
                                    return Stream.empty();
                                }
                            });
                        })
                        .collect(Collectors.toSet());
            }

            @Override
            public Collection<String> getIncludes(boolean publicOnly) {
                final BoundTemplate.Visitor<Stream<String>> templateVisitor = new BoundTemplate.Visitor<Stream<String>>() {
                    @Override
                    public Stream<String> apply(BoundTemplate.VarBinding b) {
                        return Stream.empty();
                    }

                    @Override
                    public Stream<String> apply(BoundTemplate.ClassBinding b) {
                        return b.getType().getIncludes(publicOnly).stream();
                    }

                    @Override
                    public Stream<String> apply(BoundTemplate.ArrayBinding b) {
                        return Stream.concat(
                                Stream.of("java/array.h"),
                                b.getType().visit(this));
                    }

                    @Override
                    public Stream<String> apply(BoundTemplate.Any b) {
                        return Stream.empty();
                    }
                };

                final Stream<String> parentTypes = Stream.concat(Stream.of(jc.getSuperClass()).filter(Objects::nonNull), jc.getInterfaces().stream())
                        .flatMap(c -> c.visit(templateVisitor));
                final Stream<String> includes = jc.getIncludes(publicOnly).stream();

                final Stream<String> fields;
                if (publicOnly) {
                    fields = Stream.empty();
                } else {
                    fields = jc.getFields().stream()
                            .flatMap(field -> Stream.of(field.getType(), field.getVarType()))
                            .flatMap(c -> c.visit(templateVisitor));
                }

                return Stream.of(parentTypes, includes, fields)
                        .flatMap(Function.identity())
                        .collect(Collectors.toList());
            }

            @Override
            public JavaType getModel() {
                return jc;
            }
        };
    }

    private final ClassLoader classLoader;
    private final Map<Class, JavaType> classes = new HashMap<>();
    private final Config cfg;
}
