package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.EnumType;
import com.github.nahratzah.jser_plus_plus.model.JavaType;
import com.github.nahratzah.jser_plus_plus.model.PrimitiveType;
import com.github.nahratzah.jser_plus_plus.output.CmakeModule;
import com.github.nahratzah.jser_plus_plus.output.CodeGenerator;
import static com.github.nahratzah.jser_plus_plus.output.Util.setFileContents;
import java.io.IOException;
import java.util.Collection;
import static java.util.Collections.unmodifiableCollection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.logging.Level;
import java.util.logging.Logger;

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

            if (postProcessingStarted)
                throw new IllegalStateException("Required type " + c + " not in list of classes during post processing.");

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

    public synchronized void postProcess() {
        if (postProcessingStarted) {
            LOG.log(Level.WARNING, "Post processing requested a second time; skipping...");
            return;
        }
        postProcessingStarted = true;

        classes.values().forEach(javaType -> javaType.postProcess(this));
    }

    /**
     * Registers all files that are to be emitted with the module and emits
     * those files.
     *
     * @param module The cmake module that will contain the project.
     * @throws IOException If any of the file write operations fails.
     */
    public synchronized void emit(CmakeModule module) throws IOException {
        if (!postProcessingStarted)
            throw new IllegalStateException("Must run post processing phase prior to emitting all classes.");

        final Map<List<String>, CodeGenerator> cgMap = new HashMap<>();
        classes.values().forEach(jc -> {
            cgMap.computeIfAbsent(CodeGenerator.computeBaseType(jc), baseType -> new CodeGenerator(baseType))
                    .add(jc);
        });

        for (final CodeGenerator cg : cgMap.values()) {
            setFileContents(
                    module.addHeader(cg.getFwdHeaderName()).toPath(),
                    cg.fwdHeaderFile());
            setFileContents(
                    module.addHeader(cg.getHeaderName()).toPath(),
                    cg.headerFile());
            setFileContents(
                    module.addSource(cg.sourceName()).toPath(),
                    cg.sourceFile());
        }

        setFileContents(
                module.addHeader(CodeGenerator.moduleHeaderFilename(module.getTargetName())).toPath(),
                CodeGenerator.moduleHeader(module.getTargetName(), unmodifiableCollection(classes.values())));
        setFileContents(
                module.addSource(CodeGenerator.moduleSourceFilename(module.getTargetName())).toPath(),
                CodeGenerator.moduleSource(module.getTargetName(), unmodifiableCollection(classes.values())));
    }

    private final ClassLoader classLoader;
    private final Map<Class, JavaType> classes = new HashMap<>();
    private final Config cfg;
    private boolean postProcessingStarted = false;
}
