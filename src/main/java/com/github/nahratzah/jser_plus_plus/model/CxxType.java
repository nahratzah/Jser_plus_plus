package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import java.util.Collection;
import static java.util.Collections.unmodifiableList;
import static java.util.Objects.requireNonNull;
import java.util.stream.Collectors;
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
    public Collection<String> getIncludes(boolean publicOnly) {
        if (publicOnly)
            return unmodifiableList(includes.getDeclarationIncludes());
        return Stream.concat(includes.getDeclarationIncludes().stream(), includes.getImplementationIncludes().stream())
                .collect(Collectors.toList());
    }

    public String getTemplate() {
        return template;
    }

    private final String template;
    private final Includes includes;
}
