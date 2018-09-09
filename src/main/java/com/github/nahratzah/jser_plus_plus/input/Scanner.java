package com.github.nahratzah.jser_plus_plus.input;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Objects;
import java.util.function.Function;
import java.util.jar.JarFile;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Scanner implements Closeable {
    private static final Logger LOG = Logger.getLogger(Scanner.class.getName());
    private static final String CLASS_SUFFIX = ".class";

    public static enum Options {
        ADD_BOOT_CLASSPATH;
    }

    private static void enableDebugLog(Level level) {
        final Logger rootLogger = Logger.getLogger("");
        rootLogger.setLevel(level);
        for (Handler handler : rootLogger.getHandlers())
            handler.setLevel(level);
    }

    public static void main(String[] args) throws Exception {
        enableDebugLog(Level.FINE);

        try (Scanner scanner = new Scanner(Options.ADD_BOOT_CLASSPATH)) {
            scanner.getClassesNothrow()
                    .forEach(System.out::println);
        }
    }

    public static File[] getBootClassPath() {
        return Stream.of(System.getProperty("sun.boot.class.path"))
                .map(bootClassPath -> bootClassPath.split(":"))
                .flatMap(Arrays::stream)
                .map(File::new)
                .map(File::getAbsoluteFile)
                .filter(File::exists)
                .toArray(File[]::new);
    }

    public Scanner() {
        this(ClassLoader.getSystemClassLoader());
    }

    public Scanner(Options... options) {
        this(new URL[]{}, options);
    }

    public Scanner(ClassLoader loader) {
        this(loader, false);
    }

    public Scanner(ClassLoader loader, boolean doClose) {
        this.loader = loader;
    }

    public Scanner(URL urls[], Options... options) {
        this(URLClassLoader.newInstance(applyOptions(urls, options)), true);
    }

    public Scanner(File classPath[], Options... options) throws MalformedURLException {
        this(filesToUrls(classPath), options);
    }

    public ClassLoader getClassLoader() {
        return loader;
    }

    public Stream<String> getClassNames() throws IOException {
        return getClassNames(loader)
                .distinct();
    }

    public Stream<Class<?>> getClassesNothrow() throws IOException {
        return getClassNames()
                .map((name) -> {
                    try {
                        return loader.loadClass(name);
                    } catch (ClassNotFoundException ex) {
                        LOG.log(Level.WARNING, "Class not loadable: " + name, ex);
                        return (Class<?>) null;
                    }
                })
                .filter(Objects::nonNull);
    }

    public Stream<Class<?>> getClassesUnchecked() throws IOException {
        return getClassNames()
                .map((name) -> {
                    try {
                        return Class.forName(name, false, loader);
                    } catch (ClassNotFoundException ex) {
                        LOG.log(Level.WARNING, "Class not loadable: " + name, ex);
                        throw new RuntimeException(ex);
                    }
                });
    }

    public Stream<Class<?>> getClassesChecked() throws IOException, ClassNotFoundException {
        final Collection<Class<?>> result = new ArrayList<>();
        for (String name : getClassNames().collect(Collectors.toList()))
            result.add(Class.forName(name, false, loader));
        return result.stream();
    }

    @Override
    public void close() throws IOException {
        if (doClose && loader instanceof Closeable)
            ((Closeable) loader).close();
    }

    private static URL[] filesToUrls(File classPath[]) {
        URL[] urls = new URL[classPath.length];
        for (int i = 0; i < classPath.length; ++i) {
            try {
                urls[i] = classPath[i].toURI().toURL();
            } catch (MalformedURLException ex) {
                throw new IllegalStateException("file URI is not a valid URL?", ex);
            }
        }
        return urls;
    }

    private static URL[] applyOptions(URL urls[], Options... options) {
        if (Arrays.asList(options).contains(Options.ADD_BOOT_CLASSPATH)) {
            urls = Stream.concat(Arrays.stream(filesToUrls(getBootClassPath())), Arrays.stream(urls))
                    .distinct()
                    .toArray(URL[]::new);
        }
        return urls;
    }

    private static Stream<String> getClassNames(ClassLoader loader) throws IOException {
        final Stream<String> parentClassNames;
        if (loader.getParent() != null)
            parentClassNames = getClassNames(loader.getParent());
        else
            parentClassNames = Stream.empty();

        if (loader instanceof URLClassLoader)
            return Stream.concat(getClassNames((URLClassLoader) loader), parentClassNames);
        else
            throw new UnsupportedOperationException("Cannot process classloader of type " + loader.getClass().getName());
    }

    private static Stream<String> getClassNames(URLClassLoader loader) throws IOException {
        LOG.log(Level.FINE, "Scanning class loader {0}", loader);
        final Collection<Stream<String>> names = new ArrayList<>();
        for (URL url : loader.getURLs())
            names.add(getClassNames(url));
        return names.stream()
                .flatMap(Function.identity());
    }

    private static Stream<String> getClassNames(URL url) throws IOException {
        LOG.log(Level.FINE, "Scanning class loader URL {0}", url);

        if ("file".equals(url.getProtocol())) {
            final File path = new File(url.getPath());
            if (path.isDirectory())
                return getClassNames(new File(url.getPath()));
        }

        final URL jarUrl = new URL("jar:" + url.toString() + "!/");
        LOG.log(Level.FINE, "Replaced URL with {0}", jarUrl);

        final URLConnection conn = jarUrl.openConnection();
        if (conn instanceof JarURLConnection)
            return getClassNames((JarURLConnection) conn);
        else
            throw new UnsupportedOperationException("Cannot process URL " + url);
    }

    private static Stream<String> getClassNames(File dir) throws IOException {
        final Path root = dir.toPath().toAbsolutePath();
        return Files.walk(root)
                .filter(path -> Files.isRegularFile(path))
                .map(path -> root.relativize(path))
                .map(path -> path.toString())
                .filter(filename -> filename.endsWith(CLASS_SUFFIX))
                .map(filename -> filename.substring(0, filename.length() - CLASS_SUFFIX.length()))
                .map(filename -> filename.replace('/', '.'));
    }

    private static Stream<String> getClassNames(JarURLConnection conn) throws IOException {
        conn.connect();
        final JarFile file = conn.getJarFile();
        return file.stream()
                .peek(jarEntry -> {
                    if (jarEntry.isDirectory() || !jarEntry.getName().endsWith(CLASS_SUFFIX))
                        LOG.log(Level.FINE, "{0} contains jar entry {1}", new Object[]{conn.getURL(), jarEntry.getName()});
                })
                .filter(jarEntry -> !jarEntry.isDirectory())
                .map(jarEntry -> jarEntry.getName())
                .filter(filename -> filename.endsWith(CLASS_SUFFIX))
                .map(filename -> filename.substring(0, filename.length() - CLASS_SUFFIX.length()))
                .map(filename -> filename.replace('/', '.'));
    }

    private final ClassLoader loader;
    private final boolean doClose = false;
}
