package com.github.nahratzah.jser_plus_plus.output;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import static java.nio.charset.StandardCharsets.UTF_8;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.unmodifiableList;
import static java.util.Collections.unmodifiableSet;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.TreeSet;
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
    private static final List<String> TAG_NS = unmodifiableList(Arrays.asList("java", "_tags"));
    private static final List<String> ERASED_TYPE_NS = unmodifiableList(Arrays.asList("java", "_erased"));
    private static final String CSTDDEF_INCLUDE = "cstddef";
    private static final String ACCESSOR_INCLUDE = "java/_accessor.h";
    private static final String JAVA_REF_INCLUDE = "java/ref.h";
    private static final Set<String> PRE_INCLUDES = unmodifiableSet(new HashSet<>(Arrays.asList(
            CSTDDEF_INCLUDE,
            ACCESSOR_INCLUDE,
            JAVA_REF_INCLUDE
    )));
    private static final Comparator<String> INCLUDE_SORTER = CodeGenerator::pathComparison;
    private static final STGroup CODE_GENERATOR_TEMPLATE;

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

    private String erasedTypeNs() {
        return Stream.concat(ERASED_TYPE_NS.stream(), namespace.stream())
                .collect(Collectors.joining("::"));
    }

    public <A extends Appendable> A writeFwdHeaderFile(A w) throws IOException {
        final String tagNs = Stream.concat(TAG_NS.stream(), namespace.stream())
                .collect(Collectors.joining("::"));
        final String erasedTypeNs = erasedTypeNs();
        final String inclusionGuard = "JAVA_FWD_" + String.join("_", baseType).toUpperCase(Locale.ROOT) + "_H";

        // Render inclusion guard and doxygen header documentation.
        w
                .append("#ifndef ").append(inclusionGuard).append('\n')
                .append("#define ").append(inclusionGuard).append('\n')
                .append('\n')
                .append("///\\file\n")
                .append("///\\brief Forward declarations for ").append(String.join(".", baseType)).append('\n')
                .append("///\\note Include <").append(headerName()).append("> instead.")
                .append('\n');

        // Include header, so that std::size_t is defined.
        // (std::size_t is used in the tag description, to hold the arity.)
        w.append("#include <").append(CSTDDEF_INCLUDE).append(">\n\n");

        w.append("// Forward declare all erased types.\n");
        w.append("namespace ").append(erasedTypeNs).append(" {\n");
        for (final JavaClass type : types)
            w.append("class ").append(type.getClassName()).append(";\n");
        w.append("} /* namespace ").append(erasedTypeNs).append(" */\n");

        w.append("// Declare all tags.\n");
        w.append("namespace ").append(tagNs).append(" {\n");
        for (final JavaClass type : types)
            w
                    .append("struct ").append(type.getClassName()).append(" {\n")
                    .append("  static constexpr ::std::size_t generics_arity = ").append(Integer.toString(type.getTemplateArguments().size())).append(";\n")
                    .append("  using erased_type = ::").append(erasedTypeNs).append("::").append(type.getClassName()).append(";\n")
                    .append("};\n");
        w.append("} /* namespace ").append(tagNs).append(" */\n");

        // Include header for _accessor.
        w.append('\n'); // Add a new line before the includes.
        w.append("#include <").append(ACCESSOR_INCLUDE).append(">\n\n");

        w.append("// Forward declare accessors.\n");
        w.append("namespace java {\n\n");
        for (final JavaClass type : types) {
            final String tag = "::" + String.join("::", tagNs) + "::" + type.getClassName();
            final String accessorTemplate = Stream.concat(
                    Stream.of("typename _Base"),
                    type.getTemplateArguments().stream().map(n -> "typename " + n))
                    .collect(Collectors.joining(", ", "template<", ">"));
            final String accessorType = Stream.concat(
                    Stream.of("_Base", tag),
                    type.getTemplateArguments().stream())
                    .collect(Collectors.joining(", ", "_accessor<", ">"));

            w
                    .append("///\\brief Accessor for ").append(namespace.isEmpty() ? "" : String.join(".", namespace) + ".").append(type.getClassName()).append(type.getTemplateArguments().isEmpty() ? "" : "<" + String.join(", ", type.getTemplateArguments()) + ">").append('\n')
                    .append(accessorTemplate).append('\n')
                    .append("class ").append(accessorType).append("; // Forward declaration.\n")
                    .append('\n');
        }
        w.append("} /* namespace java */\n");

        // Include header for basic_ref.
        w.append('\n'); // Add a new line before the includes.
        w.append("#include <").append(JAVA_REF_INCLUDE).append(">\n");

        // Declare the types.
        w.append('\n'); // Add a new line before the types.
        if (!namespace.isEmpty())
            w.append("namespace ").append(String.join("::", namespace)).append(" {\n\n");
        for (final JavaClass type : types) {
            w
                    .append("/**\n")
                    .append(" * \\brief Variable reference for ").append(namespace.isEmpty() ? "" : String.join(".", namespace) + ".").append(type.getClassName()).append(type.getTemplateArguments().isEmpty() ? "" : "<" + String.join(", ", type.getTemplateArguments()) + ">").append('\n')
                    .append(" */\n");

            final String extraTmplArgs;
            if (!type.getTemplateArguments().isEmpty()) {
                final String templateArguments = type.getTemplateArguments().stream()
                        .map(name -> "typename " + name)
                        .collect(Collectors.joining(", "));
                w.append("template<").append(templateArguments).append(">\n");
                extraTmplArgs = type.getTemplateArguments().stream()
                        .collect(Collectors.joining(", ", ",\n    ", ""));
            } else {
                extraTmplArgs = "";
            }
            w
                    .append("using ").append(type.getClassName()).append(" = ")
                    .append("::java::type<\n")
                    .append("    ::").append(tagNs).append("::").append(type.getClassName()).append(extraTmplArgs).append(">;\n")
                    .append('\n');
        }
        if (!namespace.isEmpty())
            w.append("} /* namespace ").append(String.join("::", namespace)).append(" */\n");

        // Render includes
        {
            final Iterator<String> includeIter = types.stream()
                    .unordered()
                    .flatMap(type -> {
                        return Stream.concat(
                                type.getIncludes(true).stream().unordered(),
                                type.getDependentTypes(true).stream().map(CodeGenerator::fwdHeaderName).unordered());
                    })
                    .distinct()
                    .sorted(INCLUDE_SORTER)
                    .filter(include -> !PRE_INCLUDES.contains(include))
                    .filter(include -> !Objects.equals(fwdHeaderName(), include))
                    .filter(include -> !Objects.equals(headerName(), include))
                    .iterator();
            if (includeIter.hasNext()) w.append('\n'); // Separator.
            while (includeIter.hasNext())
                w.append("#include <").append(includeIter.next()).append(">\n");
        }

        w.append("#include <java/inline.h>\n\n");

        w.append("// render accessors\n");
        w.append("namespace java {\n\n");
        {
            for (final JavaClass type : types) {
                w
                        .append(CODE_GENERATOR_TEMPLATE.getInstanceOf("accessor")
                                .add("cdef", type)
                                .render())
                        .append('\n');
            }
        }
        w.append("} /* namespace java */\n");

        // Render end of inclusion guard.
        w.append("#endif /* ").append(inclusionGuard).append(" */\n");

        return w;
    }

    public <A extends Appendable> A writeHeaderFile(A w) throws IOException {
        final String erasedTypeNs = erasedTypeNs();
        final String inclusionGuard = String.join("_", baseType).toUpperCase(Locale.ROOT) + "_H";

        // Render inclusion guard and doxygen header documentation.
        w
                .append("#ifndef ").append(inclusionGuard).append('\n')
                .append("#define ").append(inclusionGuard).append('\n')
                .append('\n')
                .append("#include <").append(fwdHeaderName()).append(">\n")
                .append('\n')
                .append("///\\file\n")
                .append("///\\brief Code for ").append(String.join(".", baseType)).append('\n');

        // Render includes
        {
            final Iterator<String> includeIter = Stream.concat(
                    Stream.of("java/inline.h", "java/object_intf.h"),
                    types.stream()
                            .unordered()
                            .flatMap(type -> {
                                return Stream.concat(
                                        type.getIncludes(false).stream().unordered(),
                                        type.getDependentTypes(false).stream().map(CodeGenerator::headerName).unordered());
                            }))
                    .distinct()
                    .sorted(INCLUDE_SORTER)
                    .filter(include -> !PRE_INCLUDES.contains(include))
                    .filter(include -> !Objects.equals(fwdHeaderName(), include))
                    .filter(include -> !Objects.equals(headerName(), include))
                    .iterator();
            if (includeIter.hasNext()) w.append('\n'); // Separator.
            while (includeIter.hasNext())
                w.append("#include <").append(includeIter.next()).append(">\n");
        }

        w.append('\n');

        // render erased type
        w.append("namespace ").append(erasedTypeNs).append(" {\n\n");
        {
            for (final JavaClass type : types) {
                w
                        .append(CODE_GENERATOR_TEMPLATE.getInstanceOf("class")
                                .add("cdef", type)
                                .render())
                        .append('\n');
            }
        }
        w.append("} /* namespace ").append(erasedTypeNs).append(" */\n");

        {
            // XXX implement accessor members
        }

        w.append("#endif /* ").append(inclusionGuard).append(" */\n");

        return w;
    }

    public <A extends Appendable> A writeSourceFile(A w) throws IOException {
        w.append("#include <").append(headerName()).append(">\n\n");

        {
            // XXX implement erased type members
        }

        return w;
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
         * The includes contain types used for super types, types used in
         * fields, types used in methods.
         *
         * @param publicOnly If set, only publicly used types are returned.
         * @return List of dependent types.
         */
        public Collection<JavaClass> getDependentTypes(boolean publicOnly);

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

    public String fwdHeaderName() {
        return fwdHeaderName(baseType);
    }

    public String headerName() {
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
