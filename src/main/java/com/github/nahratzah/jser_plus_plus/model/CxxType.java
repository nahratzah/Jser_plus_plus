package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * A type from C++.
 *
 * @author ariane
 */
public final class CxxType implements Type {
    public CxxType(String template, Includes includes) {
        this(template, includes, EMPTY_LIST, null);
    }

    public CxxType(String template, Includes includes, Collection<? extends Type> declTypes, String preRendered) {
        this.template = requireNonNull(template);
        this.includes = requireNonNull(includes);
        this.declTypes = requireNonNull(declTypes);
        this.preRendered = preRendered;
    }

    @Override
    public CxxType prerender(Context ctx, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables) {
        final Collection<Type> newDeclTypes = new HashSet<>(declTypes);
        final ST stringTemplate = new ST(StCtx.contextGroup(ctx, variables, newDeclTypes::add), getTemplate());
        renderArgs.forEach(stringTemplate::add);
        return new CxxType(template, includes, newDeclTypes, stringTemplate.render(Locale.ROOT));
    }

    @Override
    public Set<String> getUnresolvedTemplateNames() {
        if (preRendered == null)
            throw new IllegalStateException("Unresolved template names are only available on prerendered CxxType.");
        return declTypes.stream()
                .map(Type::getUnresolvedTemplateNames)
                .flatMap(Collection::stream)
                .collect(Collectors.toSet());
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

    /**
     * Retrieve prerendered text.
     *
     * May be null, if there is none.
     *
     * @return Prerendered text or null.
     */
    public String getPreRendered() {
        return preRendered;
    }

    @Override
    public int hashCode() {
        if (this.preRendered == null)
            throw new IllegalStateException("Must prerender prior to hashing.");

        int hash = 7;
        hash = 83 * hash + Objects.hashCode(this.preRendered);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this.preRendered == null)
            throw new IllegalStateException("Must prerender prior to hashing.");

        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final CxxType other = (CxxType) obj;
        if (!Objects.equals(this.preRendered, other.preRendered)) return false;
        return true;
    }

    private final transient String template;
    private final transient Includes includes;
    private final transient Collection<? extends Type> declTypes;
    private final String preRendered;
}
