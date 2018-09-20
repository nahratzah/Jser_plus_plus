package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * A type from C++.
 *
 * @author ariane
 */
public class CxxType implements Type {
    public CxxType(String template, Includes includes) {
        this(template, includes, EMPTY_LIST, false);
    }

    public CxxType(String template, Includes includes, Collection<? extends Type> declTypes, boolean preRendered) {
        this.template = requireNonNull(template);
        this.includes = requireNonNull(includes);
        this.declTypes = requireNonNull(declTypes);
        this.preRendered = preRendered;
    }

    @Override
    public CxxType prerender(Context ctx, Map<String, ?> renderArgs, Collection<String> variables) {
        if (preRendered) return this;

        final Collection<Type> newDeclTypes = new HashSet<>(declTypes);
        final ST stringTemplate = new ST(StCtx.contextGroup(ctx, variables, newDeclTypes::add), getTemplate());
        renderArgs.forEach(stringTemplate::add);
        return new CxxType(stringTemplate.render(Locale.ROOT), includes, newDeclTypes, true);
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
        return declTypes.stream()
                .flatMap(Type::getAllJavaTypes);
    }

    public String getTemplate() {
        return template;
    }

    public boolean isPreRendered() {
        return preRendered;
    }

    private final String template;
    private final Includes includes;
    private final Collection<? extends Type> declTypes;
    private final boolean preRendered;
}
