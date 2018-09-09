package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassTemplateArgument;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.JavaType;
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
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Resolver for classes to their C++ counterpart.
 *
 * @author ariane
 */
public class Processor implements Context {
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
        synchronized (classes) {
            JavaType v = classes.get(c);
            if (v != null) return v;

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
                return Stream.concat(Stream.of(jc.getSuperClass()).filter(Objects::nonNull), jc.getInterfaces().stream())
                        .map(c -> {
                            return c.visit(new BoundTemplate.Visitor<JavaType>() {
                                @Override
                                public JavaType apply(BoundTemplate.VarBinding b) {
                                    return null;
                                }

                                @Override
                                public JavaType apply(BoundTemplate.ClassBinding b) {
                                    return b.getType();
                                }

                                @Override
                                public JavaType apply(BoundTemplate.ArrayBinding b) {
                                    return b.getType().visit(this);
                                }

                                @Override
                                public JavaType apply(BoundTemplate.Any b) {
                                    return null;
                                }
                            });
                        })
                        .filter(Objects::nonNull)
                        .map(Processor::jcToCg)
                        .collect(Collectors.toList());
            }

            @Override
            public Collection<String> getIncludes(boolean publicOnly) {
                return Stream.concat(Stream.of(jc.getSuperClass()).filter(Objects::nonNull), jc.getInterfaces().stream())
                        .flatMap(c -> {
                            return c.visit(new BoundTemplate.Visitor<Stream<String>>() {
                                @Override
                                public Stream<String> apply(BoundTemplate.VarBinding b) {
                                    return Stream.empty();
                                }

                                @Override
                                public Stream<String> apply(BoundTemplate.ClassBinding b) {
                                    return Stream.empty();
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
                            });
                        })
                        .collect(Collectors.toList());
            }
        };
    }

    private final ClassLoader classLoader;
    private final Map<Class, JavaType> classes = new HashMap<>();
    private final Config cfg;
}