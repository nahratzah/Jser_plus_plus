package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Collection;
import static java.util.Collections.unmodifiableCollection;
import static java.util.Collections.unmodifiableMap;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * A type from C++.
 *
 * @author ariane
 */
public final class CxxType implements Type {
    public CxxType(Context ctx, String template, Includes includes, Map<String, ?> renderArgs, List<String> variables, BoundTemplate.ClassBinding<?> thisType) {
        this(
                ctx,
                template,
                includes,
                renderArgs,
                variables.stream().collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new)),
                thisType);
    }

    public CxxType(Context ctx, String template, Includes includes, Map<String, ?> renderArgs, Map<String, ? extends BoundTemplate> variables, BoundTemplate.ClassBinding<?> thisType) {
        this.ctx = requireNonNull(ctx);
        this.includes = requireNonNull(includes);
        this.variables = unmodifiableMap(new HashMap<>(requireNonNull(variables)));
        this.template = requireNonNull(template);
        this.renderArgs = unmodifiableMap(requireNonNull(renderArgs));
        this.thisType = thisType;

        final Collection<Type> registry = new HashSet<>();
        final ST stringTemplate = new ST(StCtx.contextGroup(ctx, this.variables, this.thisType, registry::add), this.template);
        this.renderArgs.forEach(stringTemplate::add);
        this.preRendered = stringTemplate.render(Locale.ROOT).trim();
        this.declTypes = unmodifiableCollection(registry);
    }

    private CxxType(CxxType cxxType, Map<String, ? extends BoundTemplate> bindings) {
        this(
                cxxType.ctx,
                cxxType.template,
                cxxType.includes,
                cxxType.renderArgs,
                cxxType.variables.entrySet().stream().collect(Collectors.toMap(Map.Entry::getKey, variableEntry -> variableEntry.getValue().rebind(bindings))),
                (cxxType.thisType == null ? null : cxxType.thisType.rebind(bindings)));
    }

    /**
     * Rebind unbound template argument names.
     *
     * @param bindings Mapping of name to bound type.
     * @return A copy of this bound template, where each occurance of unresolved
     * template variabe in bindings has been replaced with its mapping.
     */
    @Override
    public CxxType rebind(Map<String, ? extends BoundTemplate> bindings) {
        return new CxxType(this, bindings);
    }

    @Override
    public Set<String> getUnresolvedTemplateNames() {
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

    @Override
    public final boolean isJavaType() {
        return false;
    }

    public String getTemplate() {
        return template;
    }

    /**
     * Retrieve prerendered text.
     *
     * @return Prerendered text.
     */
    public String getPreRendered() {
        return preRendered;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 83 * hash + Objects.hashCode(this.preRendered);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final CxxType other = (CxxType) obj;
        if (!Objects.equals(this.preRendered, other.preRendered)) return false;
        if (!Objects.equals(this.getUnresolvedTemplateNames(), other.getUnresolvedTemplateNames()))
            return false;
        return true;
    }

    @Override
    public String toString() {
        return preRendered;
    }

    private final Context ctx;
    private final Includes includes;
    private final Map<String, BoundTemplate> variables;
    private final Map<String, ?> renderArgs;
    private final BoundTemplate.ClassBinding<?> thisType;
    private final String template;
    private final String preRendered;
    private final Collection<? extends Type> declTypes;
}
