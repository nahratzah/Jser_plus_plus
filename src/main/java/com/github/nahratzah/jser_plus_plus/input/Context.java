package com.github.nahratzah.jser_plus_plus.input;

import com.github.nahratzah.jser_plus_plus.model.JavaType;
import com.github.nahratzah.jser_plus_plus.model.PrimitiveType;
import java.util.Arrays;

/**
 * Context in which classes are defined.
 *
 * @author ariane
 */
public interface Context {
    public ClassLoader getClassLoader();

    public JavaType resolveClass(Class<?> c);

    public default JavaType resolveClass(String c) {
        return Arrays.stream(PrimitiveType.values())
                .filter(primitiveType -> primitiveType.getName().equals(c))
                .map(primitiveType -> {
                    // Stream gets confused without the conversion.
                    final JavaType javaType = primitiveType;
                    return javaType;
                })
                .findAny()
                .orElseGet(() -> {
                    try {
                        return resolveClass(getClassLoader().loadClass(c));
                    } catch (ClassNotFoundException ex) {
                        throw new IllegalStateException("unresolveable class " + c);
                    }
                });
    }

    /**
     * Context that never resolves anything, but instead always throws
     * {@link UnsupportedOperationException}.
     */
    public static Context UNIMPLEMENTED_CONTEXT = new Context() {
        @Override
        public ClassLoader getClassLoader() {
            throw new UnsupportedOperationException("Unimplemented context.");
        }

        @Override
        public JavaType resolveClass(Class<?> c) {
            throw new UnsupportedOperationException("Unimplemented context.");
        }

        @Override
        public JavaType resolveClass(String c) {
            throw new UnsupportedOperationException("Unimplemented context.");
        }
    };
}
