package com.github.nahratzah.jser_plus_plus.model.impl;

import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassTemplateArgument;
import com.google.common.collect.Collections2;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.stream.Collectors;

/**
 * Utilities for dealing with class generics.
 *
 * @author ariane
 */
public class ClassGenerics {
    private final Map<String, List<BoundTemplate>> generics;
    private final List<String> resolutionOrder;

    public ClassGenerics(List<ClassTemplateArgument> generics) {
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

    public List<BoundTemplate> getResolutionTypes() {
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
                .collect(Collectors.toList());
    }

    public List<BoundTemplate> getConstraints() {
        return generics.keySet().stream()
                .map((name) -> {
                    final BoundTemplate.Any type = new BoundTemplate.Any(EMPTY_LIST, requireNonNull(generics.get(name)));
                    return type
                            .rebind(singletonMap(name, new BoundTemplate.Any()));
                })
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
        return "__" + name + "__";
    }

    private Map<String, String> mangledNames() {
        return generics.keySet().stream()
                .collect(Collectors.toMap(Function.identity(), ClassGenerics::mangledName));
    }
}
