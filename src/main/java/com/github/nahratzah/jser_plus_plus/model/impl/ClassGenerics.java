package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassTemplateArgument;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.PrimitiveType;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Collections2;
import com.google.common.collect.Streams;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * Utilities for dealing with class generics.
 *
 * @author ariane
 */
public class ClassGenerics {
    private final ClassType model;
    private final Map<String, List<BoundTemplate>> generics;
    private final List<String> resolutionOrder;

    public ClassGenerics(ClassType model, List<ClassTemplateArgument> generics) {
        this.model = requireNonNull(model);
        this.generics = generics.stream()
                .collect(Collectors.collectingAndThen(
                        Collectors.toMap(
                                ClassTemplateArgument::getName,
                                ClassTemplateArgument::getExtendBounds,
                                (u, v) -> {
                                    throw new IllegalStateException(String.format("Duplicate key %s", u));
                                },
                                LinkedHashMap::new),
                        Collections::unmodifiableMap));
        this.resolutionOrder = unmodifiableList(computeResolutionOrder(this.generics));
    }

    public Map<String, BoundTemplate> getGenerics() {
        return generics.entrySet().stream()
                .collect(Collectors.toMap(
                        Map.Entry::getKey,
                        argument -> BoundTemplate.MultiType.maybeMakeMultiType(argument.getValue()),
                        (u, v) -> {
                            throw new IllegalStateException(String.format("Duplicate key %s", u));
                        },
                        LinkedHashMap::new));
    }

    public List<String> getGenericsNames() {
        return new ArrayList<>(generics.keySet());
    }

    public List<String> getMangledGenericsNames() {
        return generics.keySet().stream()
                .map(ClassGenerics::mangledName)
                .collect(Collectors.toList());
    }

    public List<String> getMangledResolutionNames() {
        return resolutionOrder.stream()
                .map(ClassGenerics::mangledName)
                .collect(Collectors.toList());
    }

    public List<String> getResolutionNames() {
        return resolutionOrder;
    }

    public List<String> getResolutionTypes() {
        // We need to prerender the entire thing, so we can implement things
        // by sidestepping `java::G::is<>`, which would recurse into the tag
        // declaration.
        final Map<String, BoundTemplate.VarBinding> replacements = mangledNames().entrySet().stream()
                .collect(Collectors.toMap(Map.Entry::getKey, entry -> new BoundTemplate.VarBinding(entry.getValue())));

        return resolutionOrder.stream()
                .map((name) -> {
                    final Collection<BoundTemplate> typeSet = Collections2.filter(
                            requireNonNull(generics.get(name)),
                            template -> {
                                // Remove references to java.lang.Object.
                                if (template instanceof BoundTemplate.ClassBinding) {
                                    final String templateClassName = ((BoundTemplate.ClassBinding) template).getType().getName();
                                    return !Objects.equals(Object.class.getName(), templateClassName);
                                }
                                return true;
                            });
                    if (typeSet.isEmpty()) return null;
                    final BoundTemplate type = new BoundTemplate.Any(EMPTY_LIST, typeSet);
                    return type
                            .rebind(singletonMap(name, new BoundTemplate.Any()))
                            .rebind(replacements);
                })
                .map(template -> template.visit(BypassVisitor.INSTANCE))
                .collect(Collectors.toList());
    }

    public List<String> getConstraints() {
        // We need to prerender the entire thing, so we can implement things
        // by sidestepping `java::G::is<>`, which would recurse into the tag
        // declaration.
        final Map<String, BoundTemplate.VarBinding> replacements = generics.keySet().stream()
                .collect(Collectors.toMap(Function.identity(), BoundTemplate.VarBinding::new));

        return generics.keySet().stream()
                .map((name) -> {
                    final BoundTemplate.Any type = new BoundTemplate.Any(EMPTY_LIST, requireNonNull(generics.get(name)));
                    return type.rebind(replacements);
                })
                .map(template -> template.visit(BypassVisitor.INSTANCE))
                .collect(Collectors.toList());
    }

    public List<String> getParentTypesForTag() {
        return Stream.concat(
                Streams.stream(Optional.ofNullable(model.getSuperClass())),
                model.getInterfaces().stream())
                .map(type -> type.visit(BypassVisitor.INSTANCE))
                .collect(Collectors.toList());
    }

    /**
     * Figure out the order in which to resolve template types.
     *
     * @param generics List of generics.
     * @return Resolved generics.
     */
    private static List<String> computeResolutionOrder(Map<String, List<BoundTemplate>> generics) {
        final List<String> resolutionOrder = new ArrayList<>(generics.size());
        final Consumer<String> resolver = new Consumer<String>() {
            private final Set<String> done = new HashSet<>();
            private final Set<String> inProgress = new HashSet<>();

            @Override
            public void accept(String name) {
                if (!inProgress.add(name))
                    throw new IllegalStateException("Recursion!");
                try {
                    if (done.add(name)) {
                        Objects.requireNonNull(generics.get(name)).stream()
                                .map(BoundTemplate::getUnresolvedTemplateNames)
                                .flatMap(Collection::stream)
                                .filter(Predicate.isEqual(name).negate())
                                .sorted() // To keep ordering deterministic.
                                .forEach(this);
                        resolutionOrder.add(name);
                    }
                } finally {
                    inProgress.remove(name);
                }
            }
        };
        generics.keySet().forEach(resolver);

        return resolutionOrder;
    }

    private static String mangledName(String name) {
        return "__RawArg_" + name + "__";
    }

    private Map<String, String> mangledNames() {
        return generics.keySet().stream()
                .collect(Collectors.toMap(Function.identity(), ClassGenerics::mangledName));
    }

    /**
     * Visitor that creates generics types while bypassing all the generics
     * logic.
     *
     * This is needed to prevent type recursion.
     */
    private static class BypassVisitor implements BoundTemplate.Visitor<String> {
        public static final BypassVisitor INSTANCE = new BypassVisitor("::java::G::is_t", "::java::G::is");
        private static final BypassVisitor EXTENDS = new BypassVisitor("::java::G::extends_t", "::java::G::extends");
        private static final BypassVisitor SUPER = new BypassVisitor("::java::G::super_t", "::java::G::super");
        private final String selection;
        private final String varSelection;

        private BypassVisitor(String selection, String varSelection) {
            this.selection = requireNonNull(selection);
            this.varSelection = requireNonNull(varSelection);
        }

        @Override
        public String apply(BoundTemplate.VarBinding b) {
            if (this.varSelection.equals(INSTANCE.varSelection))
                return b.getName();
            return varSelection + '<' + b.getName() + '>';
        }

        @Override
        public String apply(BoundTemplate.ClassBinding<?> b) {
            final String clsType = new ST(StCtx.BUILTINS, "$tagType(model)$")
                    .add("model", b.getType())
                    .render(Locale.ROOT);

            return Stream.concat(
                    Stream.of(clsType),
                    b.getBindings().stream()
                            .map(tmpl -> tmpl.visit(INSTANCE)))
                    .collect(Collectors.joining(", ", selection + "<", ">"));
        }

        @Override
        public String apply(BoundTemplate.ArrayBinding b) {
            final String elementType;
            if (b.getType() instanceof BoundTemplate.ClassBinding && ((BoundTemplate.ClassBinding<?>) b.getType()).getType() instanceof PrimitiveType) {
                final PrimitiveType primitiveType = (PrimitiveType) ((BoundTemplate.ClassBinding<?>) b.getType()).getType();
                elementType = Stream.concat(primitiveType.getNamespace().stream(), Stream.of(primitiveType.getClassName()))
                        .map(nameElem -> "::" + nameElem)
                        .collect(Collectors.joining());
            } else {
                elementType = b.getType().visit(INSTANCE);
            }

            final StringBuilder arrayObj = new StringBuilder()
                    .append(elementType);
            for (int i = 0; i < b.getExtents(); ++i)
                arrayObj.append('*');
            return arrayObj.toString();
        }

        @Override
        public String apply(BoundTemplate.Any b) {
            final Stream<String> extendStream = b.getExtendTypes().stream()
                    .map(type -> type.visit(EXTENDS));
            final Stream<String> superStream = b.getSuperTypes().stream()
                    .map(type -> type.visit(SUPER));
            final List<String> constraints = Stream.concat(extendStream, superStream)
                    .collect(Collectors.toList());

            if (constraints.size() == 1) {
                return constraints.get(0);
            } else {
                return "::java::G::pack_t<"
                        + String.join(", ", constraints)
                        + '>';
            }
        }

        @Override
        public String apply(BoundTemplate.MultiType b) {
            if (b.getTypes().size() == 1)
                return b.getTypes().iterator().next().visit(this);

            return "::java::G::pack_t"
                    + b.getTypes().stream()
                            .map(t -> t.visit(this))
                            .collect(Collectors.joining(", ", "<", ">"));
        }
    };
}
