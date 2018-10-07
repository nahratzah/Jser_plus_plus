package com.github.nahratzah.jser_plus_plus.output;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Collection;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 *
 * @author ariane
 */
public class CmakeModule implements Closeable {
    private static final String SUB_INSTALL_DIR = "share/cxx_java";
    private static final String HEADER_SUBDIR = "include";
    private static final String SOURCE_SUBDIR = "src";
    private static final String LIB_SUBDIR = "lib";
    private static final String GEN_SUBDIR = "generated";
    private static final String SUPPLIED_SUBDIR = "copied";

    public CmakeModule(String targetName, File outputDir, Set<String> extraPackages, Set<String> extraLibs, Set<String> extraIncludeDirs) throws IOException {
        this.targetName = requireNonNull(targetName);
        this.outputDir = requireNonNull(outputDir);
        this.extraPackages = requireNonNull(extraPackages);
        this.extraLibs = requireNonNull(extraLibs);
        this.extraIncludeDirs = requireNonNull(extraIncludeDirs);
        Files.createDirectories(this.outputDir.toPath());
        Files.createDirectories(new File(outputDir, GEN_SUBDIR + '/' + HEADER_SUBDIR).toPath());
        Files.createDirectories(new File(outputDir, GEN_SUBDIR + '/' + SOURCE_SUBDIR).toPath());
        Files.createDirectories(new File(outputDir, SUPPLIED_SUBDIR + '/' + HEADER_SUBDIR).toPath());
        Files.createDirectories(new File(outputDir, SUPPLIED_SUBDIR + '/' + SOURCE_SUBDIR).toPath());
    }

    private static File fileForGeneratedCodeFile(String subdir, String codeFile) {
        requireNonNull(subdir);
        requireNonNull(codeFile);
        return new File(new File(new File(GEN_SUBDIR), subdir), codeFile);
    }

    public File addHeader(String header) throws IOException {
        final File relativeFileName = fileForGeneratedCodeFile(HEADER_SUBDIR, header);
        final File fileName = new File(outputDir, relativeFileName.getPath());
        headers.put(new Header(header), relativeFileName.getPath());
        Files.createDirectories(fileName.toPath().getParent());
        return fileName;
    }

    public File addSource(String source) throws IOException {
        final File relativeFileName = fileForGeneratedCodeFile(SOURCE_SUBDIR, source);
        final File fileName = new File(outputDir, relativeFileName.getPath());
        sources.add(relativeFileName.getPath());
        Files.createDirectories(fileName.toPath().getParent());
        return fileName;
    }

    private Map<String, String> addSupplied(String genDir, File inputDir) throws IOException {
        requireNonNull(genDir);
        requireNonNull(inputDir);

        final Path dstDir = outputDir.toPath().resolve(genDir);
        return Util.copyRecursivelyIfDifferent(dstDir, inputDir.toPath()).stream()
                .collect(Collectors.toMap(Path::toString, relPath -> dstDir.resolve(relPath).toString()));
    }

    public void addSupplied(File hdrDir, File srcDir) throws IOException {
        if (hdrDir != null) {
            addSupplied(SUPPLIED_SUBDIR + '/' + HEADER_SUBDIR, hdrDir).forEach((cxxPath, cmakePath) -> {
                headers.put(new Header(cxxPath), cmakePath);
            });
        }

        if (srcDir != null) {
            addSupplied(SUPPLIED_SUBDIR + '/' + SOURCE_SUBDIR, srcDir).forEach((cxxPath, cmakePath) -> {
                sources.add(cmakePath);
            });
        }
    }

    public String getContents() throws IOException {
        final StringWriter writer = new StringWriter();
        write(writer);
        return writer.toString();
    }

    public void write(Writer out) throws IOException {
        out
                .append("cmake_minimum_required(VERSION 3.11)\n")
                .append('\n')
                .append("option(BUILD_SHARED_LIBS \"Build shared libraries\" ON)\n")
                .append('\n')
                .append("find_package(cycle_ptr)\n");
        for (String pkg
                     : extraPackages.stream().sorted().collect(Collectors.toList()))
            out.append("find_package(").append(pkg).append(")");

        out
                .append("\n")
                .append("add_library(").append(targetName).append("\n");
        for (String source
                     : sources.stream().sorted().collect(Collectors.toList()))
            out.append("  ").append(source).append('\n');
        out
                .append("  )\n")
                .append('\n')
                .append("target_link_libraries(").append(targetName).append(" PUBLIC ").append("cycle_ptr").append(")\n");
        for (String lib
                     : extraLibs.stream().sorted().collect(Collectors.toList()))
            out.append("target_link_libraries(").append(targetName).append(" PUBLIC ").append(lib).append(")\n");
        for (String incDir
                     : extraIncludeDirs.stream().sorted().collect(Collectors.toList()))
            out.append("target_include_directories(").append(targetName).append(" PUBLIC ").append(incDir).append(")\n");
        out
                .append("target_include_directories (").append(targetName).append(" PUBLIC\n")
                .append("  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/").append(GEN_SUBDIR).append('/').append(HEADER_SUBDIR).append(">\n")
                .append("  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/").append(SUPPLIED_SUBDIR).append('/').append(HEADER_SUBDIR).append(">\n")
                .append("  $<INSTALL_INTERFACE:").append(getInstallDir()).append('/').append(HEADER_SUBDIR).append(">)\n")
                .append('\n')
                .append("install(TARGETS ").append(targetName).append(" DESTINATION ").append(getInstallDir()).append('/').append(LIB_SUBDIR).append(")\n");

        doHeaders(out);
    }

    public String getInstallDir() {
        return SUB_INSTALL_DIR + "/" + targetName;
    }

    private void doHeaders(Writer writer) throws IOException {
        final Iterator<Map.Entry<Optional<String>, List<Header>>> iter = headers.keySet().stream()
                .collect(Collectors.groupingBy(Header::getDir)).entrySet().stream()
                .sorted(Comparator.comparing(
                        Map.Entry::getKey,
                        Comparator.comparing(
                                (Optional<String> dir) -> dir.map(s -> s.split(Pattern.quote("/"))).orElse(new String[]{}),
                                CmakeModule::compareStringArray)))
                .iterator();

        writer.append("\n# Install header\n");
        while (iter.hasNext()) {
            final Map.Entry<Optional<String>, List<Header>> iterItem = iter.next();
            final String headerDir = iterItem.getKey().map(s -> '/' + s).orElse("");

            writer.append("install(FILES\n");
            for (Header hdr
                         : iterItem.getValue().stream().sorted(Comparator.comparing(Header::getFile)).toArray(Header[]::new))
                writer.append("  ").append(headers.get(hdr)).append('\n');
            writer.append("  DESTINATION ").append(getInstallDir()).append('/').append(HEADER_SUBDIR).append(headerDir).append(")\n");
        }
    }

    private static class Header {
        private final String dir;
        private final String file;

        public Header(String header) {
            requireNonNull(header);
            final int lastSlash = header.lastIndexOf('/');
            if (lastSlash == -1) {
                this.dir = null;
                this.file = header;
            } else {
                this.dir = header.substring(0, lastSlash);
                this.file = header.substring(lastSlash + 1);
            }
        }

        public Optional<String> getDir() {
            return Optional.ofNullable(dir);
        }

        public String getFile() {
            return file;
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 37 * hash + Objects.hashCode(this.dir);
            hash = 37 * hash + Objects.hashCode(this.file);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final Header other = (Header) obj;
            if (!Objects.equals(this.dir, other.dir)) return false;
            if (!Objects.equals(this.file, other.file)) return false;
            return true;
        }

        @Override
        public String toString() {
            return (dir == null ? file : dir + "/" + file);
        }
    }

    private static int compareStringArray(String[] x, String[] y) {
        for (int i = 0; i < x.length && i < y.length; ++i) {
            final int cmp = x[i].compareTo(y[i]);
            if (cmp != 0) return cmp;
        }
        return Integer.compare(x.length, y.length);
    }

    public File getCmakeOutputFile() {
        return new File(outputDir, "CMakeLists.txt");
    }

    @Override
    public void close() throws IOException {
        if (closed) return;
        closed = true;

        // Clean up output directories.
        cleanOutputDirectory(outputDir.toPath().resolve(SUPPLIED_SUBDIR).resolve(HEADER_SUBDIR), headers.values());
        cleanOutputDirectory(outputDir.toPath().resolve(GEN_SUBDIR).resolve(HEADER_SUBDIR), headers.values());
        cleanOutputDirectory(outputDir.toPath().resolve(SUPPLIED_SUBDIR).resolve(SOURCE_SUBDIR), sources);
        cleanOutputDirectory(outputDir.toPath().resolve(GEN_SUBDIR).resolve(SOURCE_SUBDIR), sources);

        // Render CMakeLists.txt
        Util.setFileContents(getCmakeOutputFile().toPath(), getContents());
    }

    public String getTargetName() {
        return targetName;
    }

    private void cleanOutputDirectory(Path dir, Collection<String> filesToKeep) throws IOException {
        final List<Path> suppliedFiles = filesToKeep.stream()
                .map(x -> outputDir.toPath().resolve(x))
                .filter(x -> x.startsWith(dir))
                .map(x -> dir.relativize(x))
                .collect(Collectors.toList());
        Util.cleanDirExcepting(dir, suppliedFiles);
    }

    private final String targetName;
    private final File outputDir;
    private final Map<Header, String> headers = new ConcurrentHashMap<>();
    private final Set<String> sources = ConcurrentHashMap.newKeySet();
    private final Set<String> extraPackages;
    private final Set<String> extraLibs;
    private final Set<String> extraIncludeDirs;
    private boolean closed = false;
}
