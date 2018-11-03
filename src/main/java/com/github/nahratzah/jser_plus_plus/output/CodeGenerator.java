package com.github.nahratzah.jser_plus_plus.output;

import com.github.nahratzah.jser_plus_plus.misc.ListComparator;
import com.github.nahratzah.jser_plus_plus.model.Accessor;
import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ClassType;
import com.github.nahratzah.jser_plus_plus.model.JavaType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import com.google.common.collect.Collections2;
import com.google.common.collect.Streams;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import static java.nio.charset.StandardCharsets.UTF_8;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.unmodifiableCollection;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableSet;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.TreeSet;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.STGroup;
import org.stringtemplate.v4.STGroupString;

/**
 * Writes a header and source file.
 *
 * @author ariane
 */
public class CodeGenerator {
    private static final Set<String> PRE_INCLUDES = unmodifiableSet(new HashSet<>(Arrays.asList(
            "cstddef",
            "java/_accessor.h",
            "java/ref.h",
            "java/inline.h",
            "string_view"
    )));
    private static final Comparator<String> INCLUDE_SORTER = CodeGenerator::pathComparison;
    private static final STGroup CODE_GENERATOR_TEMPLATE;
    private static final STGroup FILES_TEMPLATE;
    private static final int LINE_WRAP = 80;

    public CodeGenerator(List<String> baseType) {
        requireNonNull(baseType);
        if (baseType.isEmpty())
            throw new IllegalArgumentException("empty base type");

        this.baseType = unmodifiableList(new ArrayList<>(baseType));
        this.namespace = this.baseType.subList(0, this.baseType.size() - 1);
    }

    public boolean add(ClassType c) {
        // Input validation.
        if (!Objects.equals(this.baseType, computeBaseType(c)))
            throw new IllegalArgumentException("mismatching base type");

        return this.types.add(c);
    }

    public boolean addAll(Collection<? extends ClassType> c) {
        // Input validation.
        c.forEach(cItem -> {
            if (!Objects.equals(this.baseType, computeBaseType(cItem)))
                throw new IllegalArgumentException("mismatching base type");
        });

        return this.types.addAll(c);
    }

    public String tagFwdHeaderFile() {
        final Collection<String> includes = Stream.of("java/generics_arity_.h", "type_traits", "cstddef")
                .distinct()
                .sorted(INCLUDE_SORTER)
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("fwdTagFile")
                .add("codeGen", this)
                .add("includes", includes)
                .render(Locale.ROOT, LINE_WRAP);
    }

    public String tagHeaderFile() {
        final Collection<String> includes = types.stream()
                .flatMap(type -> {
                    return Streams.<BoundTemplate>concat(
                            Streams.stream(Optional.ofNullable(type.getSuperClass())),
                            type.getInterfaces().stream(),
                            type.getClassGenerics().getGenerics().values().stream());
                })
                .flatMap(Type::getAllJavaTypes)
                .map(CodeGenerator::computeBaseType)
                .map(CodeGenerator::tagFwdHeaderName)
                .filter(Predicate.isEqual(getTagFwdHeaderName()).negate())
                .distinct()
                .sorted(INCLUDE_SORTER)
                .collect(Collectors.toList());

        final Collection<String> includesAfter = types.stream()
                .flatMap(type -> {
                    return Streams.<BoundTemplate>concat(
                            Streams.stream(Optional.ofNullable(type.getSuperClass())),
                            type.getInterfaces().stream(),
                            type.getClassGenerics().getGenerics().values().stream());
                })
                .flatMap(Type::getAllJavaTypes)
                .map(CodeGenerator::computeBaseType)
                .map(CodeGenerator::tagHeaderName)
                .filter(Predicate.isEqual(getTagHeaderName()).negate())
                .distinct()
                .sorted(INCLUDE_SORTER)
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("tagFile")
                .add("codeGen", this)
                .add("includes", includes)
                .add("includesAfter", includesAfter)
                .render(Locale.ROOT, LINE_WRAP);
    }

    public String fwdHeaderFile() {
        final Collection<String> includes = types.stream()
                .flatMap(type -> {
                    return Streams.concat(
                            type.getAccessor().getDeclarationIncludes(),
                            type.getAccessor().getDeclarationTypes().map(CodeGenerator::fwdHeaderName),
                            type.getImplementationIncludes(true),
                            type.getForwardDeclarationJavaTypes().map(CodeGenerator::fwdHeaderName));
                })
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(Predicate.isEqual(getFwdHeaderName()).negate())
                .filter(Predicate.isEqual(getHeaderName()).negate())
                .filter(Predicate.isEqual(getTagFwdHeaderName()).negate())
                .filter(Predicate.isEqual(getTagHeaderName()).negate())
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("fwdHeaderFile")
                .add("codeGen", this)
                .add("includes", includes)
                .render(Locale.ROOT, LINE_WRAP);
    }

    public String headerFile() {
        final Collection<String> includesForTypes = Stream.concat(
                Stream.of("java/inline.h", "java/object_intf.h", "java/_maybe_cast.h"),
                types.stream()
                        .flatMap(type -> {
                            return Streams.concat(
                                    type.getImplementationIncludes(true),
                                    type.getDeclarationCompleteJavaTypes().map(CodeGenerator::headerName),
                                    type.getDeclarationForwardJavaTypes().map(CodeGenerator::fwdHeaderName));
                        }))
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .collect(Collectors.toCollection(LinkedHashSet::new));

        final Collection<String> includesForAccessors = types.stream()
                .map(ClassType::getAccessor)
                .flatMap(accessor -> {
                    return Streams.concat(
                            accessor.getImplementationIncludes(),
                            accessor.getDeclarationTypes().map(CodeGenerator::fwdHeaderName),
                            accessor.getImplementationTypes().map(CodeGenerator::fwdHeaderName));
                })
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .filter(include -> !includesForTypes.contains(include))
                .collect(Collectors.toCollection(LinkedHashSet::new));

        final Collection<String> includesForCompletion = types.stream()
                .map(ClassType::getAccessor)
                .flatMap(accessor -> accessor.getImplementationTypes().map(CodeGenerator::headerName))
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .filter(include -> !includesForTypes.contains(include))
                .filter(include -> !includesForAccessors.contains(include))
                .collect(Collectors.toCollection(LinkedHashSet::new));

        return FILES_TEMPLATE.getInstanceOf("headerFile")
                .add("codeGen", this)
                .add("includesForTypes", includesForTypes)
                .add("includesForAccessors", includesForAccessors)
                .add("includesForCompletion", includesForCompletion)
                .render(Locale.ROOT, LINE_WRAP);
    }

    public String sourceFile() {
        final Collection<String> includes = types.stream()
                .flatMap(type -> {
                    return Stream.of(
                            type.getImplementationIncludes(false),
                            type.getDeclarationCompleteJavaTypes().map(CodeGenerator::headerName),
                            type.getDeclarationForwardJavaTypes().map(CodeGenerator::headerName),
                            type.getImplementationJavaTypes().map(CodeGenerator::headerName))
                            .flatMap(Function.identity());
                })
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("srcFile")
                .add("codeGen", this)
                .add("includes", includes)
                .render(Locale.ROOT, LINE_WRAP);
    }

    private static String renderModule(ST template, String moduleName, Collection<? extends JavaType> types) {
        final List<String> includes = types.stream()
                .map(c -> headerName(c))
                .distinct()
                .sorted(INCLUDE_SORTER)
                .collect(Collectors.toList());

        final List<JavaType> orderedTypes = types.stream()
                .sorted(Comparator.comparing(JavaType::getNamespace, new ListComparator<>()).thenComparing(Comparator.comparing(JavaType::getClassName)))
                .collect(Collectors.toList());

        return template
                .add("name", moduleName)
                .add("headers", includes)
                .add("types", orderedTypes)
                .render(Locale.ROOT, LINE_WRAP);
    }

    public static String moduleHeaderFilename(String moduleName) {
        return "java/modules/" + moduleName + ".h";
    }

    public static String moduleSourceFilename(String moduleName) {
        return "java/modules/" + moduleName + ".cc";
    }

    public static String moduleHeader(String moduleName, Collection<? extends JavaType> types) {
        return renderModule(FILES_TEMPLATE.getInstanceOf("moduleHeader"), moduleName, types);
    }

    public static String moduleSource(String moduleName, Collection<? extends JavaType> types) {
        return renderModule(FILES_TEMPLATE.getInstanceOf("moduleSource"), moduleName, types);
    }

    public static List<String> computeBaseType(JavaType c) {
        // Compute base class name.
        // Anything after the first '$' sign is dropped.
        final String className = c.getClassName();
        final int dollarPos = className.indexOf('$');
        final String baseClassName = (dollarPos == -1 ? className : className.substring(0, dollarPos));

        // Retrieve the name space.
        final List<String> namespace = c.getNamespace();

        // Build result.
        final List<String> result = new ArrayList<>(namespace.size() + 1);
        result.addAll(namespace);
        result.add(baseClassName);
        return result;
    }

    public String getTagFwdHeaderName() {
        return tagFwdHeaderName(baseType);
    }

    public String getTagHeaderName() {
        return tagHeaderName(baseType);
    }

    public String getFwdHeaderName() {
        return fwdHeaderName(baseType);
    }

    public String getHeaderName() {
        return headerName(baseType);
    }

    public static String fwdHeaderName(JavaType c) {
        return fwdHeaderName(computeBaseType(c));
    }

    public static String headerName(JavaType c) {
        return headerName(computeBaseType(c));
    }

    private static String tagFwdHeaderName(List<String> baseType) {
        return "java/fwd/" + String.join("/", baseType) + ".tagfwd";
    }

    private static String tagHeaderName(List<String> baseType) {
        return "java/fwd/" + String.join("/", baseType) + ".tag";
    }

    private static String headerName(List<String> baseType) {
        return String.join("/", baseType) + ".h";
    }

    private static String fwdHeaderName(List<String> baseType) {
        return "java/fwd/" + String.join("/", baseType) + ".h";
    }

    public String sourceName() {
        return String.join("/", baseType) + ".cc";
    }

    /**
     * Helper function to compare path names and order them by path, taking into
     * account depth.
     *
     * @param x An include.
     * @param y Another include.
     * @return 1 is x is a successor of y, -1 is x precedes y, 0 otherwise.
     */
    private static int pathComparison(String x, String y) {
        final List<String> xParts = Arrays.asList(x.split(Pattern.quote("/")));
        final List<String> yParts = Arrays.asList(y.split(Pattern.quote("/")));
        final Iterator<String> xPartsIter = xParts.subList(0, xParts.size() - 1).iterator();
        final Iterator<String> yPartsIter = yParts.subList(0, yParts.size() - 1).iterator();
        final String xTail = xParts.get(xParts.size() - 1);
        final String yTail = yParts.get(yParts.size() - 1);

        while (xPartsIter.hasNext() && yPartsIter.hasNext()) {
            final String xVal = requireNonNull(xPartsIter.next());
            final String yVal = requireNonNull(yPartsIter.next());
            final int cmp = xVal.compareTo(yVal);
            if (cmp != 0) return cmp;
        }

        if (xPartsIter.hasNext()) return 1;
        if (yPartsIter.hasNext()) return -1;
        return xTail.compareTo(yTail);
    }

    /**
     * Get all types of this code generator.
     *
     * @return All types for this code generator.
     */
    public Collection<ClassType> getTypes() {
        return unmodifiableCollection(types);
    }

    public Collection<Accessor> getAccessors() {
        return Collections2.transform(getTypes(), ClassType::getAccessor);
    }

    /**
     * Return the "base" type, on which the file name is based.
     *
     * @return Path of the base type.
     */
    public List<String> getBaseType() {
        return unmodifiableList(baseType);
    }

    public String getIncludeGuard() {
        return String.join("_", getBaseType()).toUpperCase(Locale.ROOT) + "_H";
    }

    /**
     * Return the namespace of the types in this code generator.
     *
     * @return The namespace of the java types generated by this code generator.
     */
    public List<String> getNamespace() {
        return unmodifiableList(namespace);
    }

    /**
     * Get reordered types, for rendering erased type implementations.
     *
     * In order to have inheritance function, each parent of the to-be-rendered
     * type must be defined. This function reorders the types, such that this is
     * the case for types used locally.
     *
     * @return Types ordered such that any base types of a Type are before that
     * Type.
     */
    public Collection<JavaType> getReorderTypesForInheritance() {
        final Collection<JavaType> result = new LinkedHashSet<>();

        /**
         * Helper class that cascades recursion, ensuring that if a type is
         * added with priority, its types are also added with priority.
         */
        class ResultAddFn implements Consumer<JavaType> {
            /**
             * Set of types which are to be added, but haven't yet been added,
             * because we are scanning their dependencies.
             */
            final Set<JavaType> inProgress = new HashSet<>();

            @Override
            public void accept(JavaType type) {
                if (result.contains(type)) return;

                if (!inProgress.add(type))
                    throw new IllegalStateException("Recursion in dependencies.");
                try {
                    final Collection<?> parentModels = getParentModels(type);
                    types.stream()
                            .filter(t -> parentModels.contains(t))
                            .forEach(this); // Recursion
                    result.add(type);
                } finally {
                    inProgress.remove(type);
                }
            }
        }

        types.forEach(new ResultAddFn());
        return result;
    }

    private static Collection<JavaType> getParentModels(JavaType model) {
        return Stream.concat(Stream.of(model.getSuperClass()).filter(Objects::nonNull), model.getInterfaces().stream())
                .flatMap(c -> {
                    return c.visit(new BoundTemplate.Visitor<Stream<JavaType>>() {
                        @Override
                        public Stream<JavaType> apply(BoundTemplate.VarBinding b) {
                            return Stream.empty();
                        }

                        @Override
                        public Stream<JavaType> apply(BoundTemplate.ClassBinding<?> b) {
                            return Stream.of(b.getType());
                        }

                        @Override
                        public Stream<JavaType> apply(BoundTemplate.ArrayBinding b) {
                            return b.getType().visit(this);
                        }

                        @Override
                        public Stream<JavaType> apply(BoundTemplate.Any b) {
                            return Stream.empty();
                        }

                        @Override
                        public Stream<JavaType> apply(BoundTemplate.MultiType b) {
                            return b.getTypes().stream()
                                    .flatMap(template -> template.visit(this));
                        }
                    });
                })
                .collect(Collectors.toSet());
    }

    static {
        try (Reader accessorFile = new InputStreamReader(CodeGenerator.class.getResourceAsStream("codeGenerator.stg"), UTF_8)) {
            final char[] cbuf = new char[1024];
            StringBuilder sb = new StringBuilder();
            for (int rlen = accessorFile.read(cbuf); rlen != -1;
                 rlen = accessorFile.read(cbuf))
                sb.append(cbuf, 0, rlen);
            CODE_GENERATOR_TEMPLATE = new STGroupString("codeGenerator.stg", sb.toString(), '$', '$');
        } catch (IOException ex) {
            throw new IllegalStateException("unable to load accessor template", ex);
        }

        try (Reader accessorFile = new InputStreamReader(CodeGenerator.class.getResourceAsStream("files.stg"), UTF_8)) {
            final char[] cbuf = new char[1024];
            StringBuilder sb = new StringBuilder();
            for (int rlen = accessorFile.read(cbuf); rlen != -1;
                 rlen = accessorFile.read(cbuf))
                sb.append(cbuf, 0, rlen);
            FILES_TEMPLATE = new STGroupString("files.stg", sb.toString(), '$', '$');
        } catch (IOException ex) {
            throw new IllegalStateException("unable to load accessor template", ex);
        }

        CODE_GENERATOR_TEMPLATE.importTemplates(StCtx.BUILTINS);
        FILES_TEMPLATE.importTemplates(CODE_GENERATOR_TEMPLATE);
    }

    /**
     * List of types that are to be rendered by this code generator.
     */
    private final Set<ClassType> types = new TreeSet<>(Comparator.comparing(JavaType::getClassName));
    /**
     * Base type.
     *
     * This is a sequence of the package name, followed by the outer-most class
     * name.
     */
    private final List<String> baseType;
    /**
     * Name space of base type.
     *
     * This is a sequence of the package name.
     */
    private final List<String> namespace;
}
