package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import java.util.ArrayList;
import java.util.Arrays;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.unmodifiableList;
import java.util.List;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.function.Predicate;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * A name of a class, with named template arguments.
 *
 * @author ariane
 */
public final class ClassName {
    private static final Predicate<String> TEMPLATE_ARGUMENT_PREDICATE = Pattern.compile("^[_A-Z]([_A-Z0-9])*$", Pattern.CASE_INSENSITIVE).asPredicate();

    public ClassName(String name, List<String> templateArgumentNames) {
        this.name = requireNonNull(name);
        this.templateArgumentNames.addAll(templateArgumentNames);

        templateArgumentNames.forEach(arg -> {
            requireNonNull(arg);
            if (!TEMPLATE_ARGUMENT_PREDICATE.test(arg))
                throw new IllegalArgumentException("\"" + arg + "\" is not a valid template argument identifier.");
        });
    }

    /**
     * The name of the class.
     *
     * @return The name of the class.
     */
    public String getName() {
        return name;
    }

    /**
     * List of template argument names.
     *
     * @return List of template argument names.
     */
    public List<String> getTemplateArgumentNames() {
        return unmodifiableList(templateArgumentNames);
    }

    /**
     * Create class name from String.
     *
     * @param value A string of the form "package.path.to.ClassName" or
     * "package.path.to.ClassName<Template, Arguments>".
     * @return An instance of ClassName, parsed from the supplied string.
     */
    @JsonCreator
    public static ClassName valueOf(String value) {
        value = value.trim(); // Prevent spaces at end from harming the algorithm.

        final int templateStart = value.indexOf('<');
        if (templateStart == -1) return new ClassName(value, EMPTY_LIST);

        if (!value.endsWith(">"))
            throw new IllegalArgumentException("Invalid template specifier.");
        final String name = value.substring(0, templateStart).trim();
        final List<String> args = Arrays.stream(value.substring(templateStart + 1, value.length() - 1).split(Pattern.quote(",")))
                .map(String::trim)
                .collect(Collectors.toList());
        return new ClassName(name, args);
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 79 * hash + Objects.hashCode(this.name);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        final ClassName other = (ClassName) obj;
        if (!Objects.equals(this.name, other.name)) return false;
        if (!Objects.equals(this.templateArgumentNames, other.templateArgumentNames))
            return false;
        return true;
    }

    /**
     * Retrieve the string representation of the class name.
     *
     * The string representation is parseable by
     * {@link ClassName#valueOf(java.lang.String) valueOf(String)}.
     *
     * @return String representation of this class name.
     */
    @Override
    @JsonValue
    public String toString() {
        if (getTemplateArgumentNames().isEmpty())
            return getName();
        return getName() + getTemplateArgumentNames().stream()
                .collect(Collectors.joining(", ", "<", ">"));
    }

    private final String name;
    private final List<String> templateArgumentNames = new ArrayList<>();
}
