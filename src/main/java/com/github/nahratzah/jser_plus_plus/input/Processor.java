package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.JavaClass;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

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
    public JavaClass resolveClass(Class<?> c) {
        return classes.computeIfAbsent(c, this::resolveClass_);
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

    private JavaClass resolveClass_(Class c) {
        final List<String> templateArgNames = Arrays.stream(c.getTypeParameters())
                .map(tv -> tv.getName())
                .collect(Collectors.toList());

        final JavaClass jc = new JavaClass();
        jc.init(this, c, templateArgNames);
        return jc;
    }

    private final ClassLoader classLoader;
    private final Map<Class, JavaClass> classes = new ConcurrentHashMap<>();
    private final Config cfg;
}
