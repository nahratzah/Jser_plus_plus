package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Stream;

/**
 * A type from C++.
 *
 * @author ariane
 */
public class CxxType implements Type {
    public CxxType(String template, Includes includes) {
        this.template = requireNonNull(template);
        this.includes = requireNonNull(includes);
    }

    @Override
    public Stream<String> getIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        if (publicOnly)
            return includes.getDeclarationIncludes().stream();
        return Stream.concat(
                includes.getDeclarationIncludes().stream(),
                includes.getImplementationIncludes().stream());
    }

    @Override
    public Stream<JavaType> getAllJavaTypes() {
        return Stream.empty();
    }

    public String getTemplate() {
        return template;
    }

    public void setTemplate(String template) {
        this.template = template;
    }

    private String template;
    private final Includes includes;
}
