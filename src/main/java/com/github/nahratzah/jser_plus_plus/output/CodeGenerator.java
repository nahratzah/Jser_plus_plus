package com.github.nahratzah.jser_plus_plus.output;

import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
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
import java.util.Set;
import java.util.TreeSet;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
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
            "java/inline.h"
    )));
    private static final Comparator<String> INCLUDE_SORTER = CodeGenerator::pathComparison;
    private static final STGroup CODE_GENERATOR_TEMPLATE;
    private static final STGroup FILES_TEMPLATE;

    public CodeGenerator(List<String> baseType) {
        requireNonNull(baseType);
        if (baseType.isEmpty())
            throw new IllegalArgumentException("empty base type");

        this.baseType = unmodifiableList(new ArrayList<>(baseType));
        this.namespace = this.baseType.subList(0, this.baseType.size() - 1);
    }

    public boolean add(JavaClass c) {
        // Input validation.
        if (!Objects.equals(this.baseType, computeBaseType(c)))
            throw new IllegalArgumentException("mismatching base type");

        return this.types.add(c);
    }

    public boolean addAll(Collection<? extends JavaClass> c) {
        // Input validation.
        c.forEach(cItem -> {
            if (!Objects.equals(this.baseType, computeBaseType(cItem)))
                throw new IllegalArgumentException("mismatching base type");
        });

        return this.types.addAll(c);
    }

    public String fwdHeaderFile() {
        final Collection<String> includes = types.stream()
                .unordered()
                .flatMap(type -> {
                    return Stream.concat(
                            type.getIncludes(true).stream().unordered(),
                            Stream.of(type.getDependentSuperTypes(true), type.getDependentNonSuperTypes(true)).flatMap(Collection::stream).map(CodeGenerator::fwdHeaderName).unordered());
                })
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("fwdHeaderFile")
                .add("codeGen", this)
                .add("includes", includes)
                .render();
    }

    public String headerFile() {
        final Collection<String> includes = Stream.concat(
                Stream.of("java/inline.h", "java/object_intf.h"),
                types.stream()
                        .flatMap(type -> {
                            return Stream.of(
                                    type.getIncludes(false).stream(),
                                    type.getDependentSuperTypes(false).stream().map(CodeGenerator::headerName),
                                    type.getDependentNonSuperTypes(false).stream().map(CodeGenerator::fwdHeaderName))
                                    .flatMap(Function.identity());
                        }))
                .distinct()
                .sorted(INCLUDE_SORTER)
                .filter(include -> !PRE_INCLUDES.contains(include))
                .filter(include -> !Objects.equals(getFwdHeaderName(), include))
                .filter(include -> !Objects.equals(getHeaderName(), include))
                .collect(Collectors.toList());

        return FILES_TEMPLATE.getInstanceOf("headerFile")
                .add("codeGen", this)
                .add("includes", includes)
                .render();
    }

    public String sourceFile() {
        final Collection<String> includes = types.stream()
                .flatMap(type -> {
                    return Stream.of(
                            type.getIncludes(false).stream(),
                            type.getDependentSuperTypes(false).stream().map(CodeGenerator::headerName),
                            type.getDependentNonSuperTypes(false).stream().map(CodeGenerator::headerName))
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
                .render();
    }

    public static List<String> computeBaseType(JavaClass c) {
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

    public static interface JavaClass {
        /**
         * Retrieves the namespace of the class.
         *
         * @return List of strings that make up the package name of the class.
         */
        public List<String> getNamespace();

        /**
         * Retrieves the class name.
         *
         * @return Class name of the type, excluding the namespace.
         */
        public String getClassName();

        /**
         * Retrieve the tag name.
         *
         * @return C++ notation of the full path to the tag.
         */
        public default String getTagName() {
            return Stream.of(Stream.of("java", "_tags"), getNamespace().stream(), Stream.of(getClassName()))
                    .flatMap(Function.identity())
                    .map(s -> "::" + s)
                    .collect(Collectors.joining());
        }

        /**
         * Retrieve list of template argument names.
         *
         * @return The template argument names for this type.
         */
        public List<String> getTemplateArguments();

        /**
         * Retrieve list of dependent types required to render the type
         * declaration.
         *
         * The includes contain types used for super types only.
         *
         * @param publicOnly If set, only publicly used types are returned.
         * @return List of dependent super types.
         */
        public Collection<JavaClass> getDependentSuperTypes(boolean publicOnly);

        /**
         * Retrieve list of dependent types required to render the type
         * declaration.
         *
         * The includes contain types used in fields, and types used in methods.
         *
         * @param publicOnly If set, only publicly used types are returned.
         * @return List of dependent non-super types.
         */
        public Collection<JavaClass> getDependentNonSuperTypes(boolean publicOnly);

        /**
         * Retrieve list of all types that this type inherits from.
         *
         * @return Model of super class and models of interfaces.
         */
        public Collection<?> getParentModels();

        /**
         * Retrieve list of includes required to make the type declarations
         * functional.
         *
         * These includes are normal header files. Headers for dependent types
         * are omitted.
         *
         * @param publicOnly If set, only publicly used includes are returned.
         * @return List of includes.
         */
        public Collection<String> getIncludes(boolean publicOnly);

        /**
         * Access the underlying model.
         *
         * @return Object representing the underlying model.
         */
        public Object getModel();
    }

    public String getFwdHeaderName() {
        return fwdHeaderName(baseType);
    }

    public String getHeaderName() {
        return headerName(baseType);
    }

    public static String fwdHeaderName(JavaClass c) {
        return fwdHeaderName(computeBaseType(c));
    }

    public static String headerName(JavaClass c) {
        return headerName(computeBaseType(c));
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
    public Collection<JavaClass> getTypes() {
        return unmodifiableCollection(types);
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
    public Collection<JavaClass> getReorderTypesForInheritance() {
        final Collection<JavaClass> result = new LinkedHashSet<>();

        /**
         * Helper class that cascades recursion, ensuring that if a type is
         * added with priority, its types are also added with priority.
         */
        class ResultAddFn implements Consumer<JavaClass> {
            /**
             * Set of types which are to be added, but haven't yet been added,
             * because we are scanning their dependencies.
             */
            final Set<JavaClass> inProgress = new HashSet<>();

            @Override
            public void accept(JavaClass type) {
                if (result.contains(type)) return;

                if (!inProgress.add(type))
                    throw new IllegalStateException("Recursion in dependencies.");
                try {
                    final Collection<?> parentModels = type.getParentModels();
                    types.stream()
                            .filter(t -> parentModels.contains(t.getModel()))
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
            FILES_TEMPLATE = new STGroupString("codeGenerator.stg", sb.toString(), '$', '$');
        } catch (IOException ex) {
            throw new IllegalStateException("unable to load accessor template", ex);
        }

        CODE_GENERATOR_TEMPLATE.importTemplates(StCtx.BUILTINS);
        FILES_TEMPLATE.importTemplates(CODE_GENERATOR_TEMPLATE);
    }

    /**
     * List of types that are to be rendered by this code generator.
     */
    private final Set<JavaClass> types = new TreeSet<>(Comparator.comparing(JavaClass::getClassName));
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
