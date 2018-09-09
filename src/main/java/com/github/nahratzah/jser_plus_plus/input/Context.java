package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.model.JavaType;

/**
 * Context in which classes are defined.
 *
 * @author ariane
 */
public interface Context {
    public ClassLoader getClassLoader();

    public JavaType resolveClass(Class<?> c);

    public default JavaType resolveClass(String c) {
        try {
            return resolveClass(getClassLoader().loadClass(c));
        } catch (ClassNotFoundException ex) {
            throw new IllegalStateException("unresolveable class " + c);
        }
    }
}
