package com.github.nahratzah.jser_plus_plus.output;

import java.io.IOException;
import java.nio.charset.Charset;
import static java.nio.charset.StandardCharsets.UTF_8;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

/**
 *
 * @author ariane
 */
public class Util {
    private static final Logger LOG = Logger.getLogger(Util.class.getName());

    private Util() {
    }

    /**
     * Performs a binary copy of file.
     *
     * Copying the contents is skipped if the two files match.
     *
     * @param dstFile The file that should contain the same data.
     * @param inputFile The file containing data to be copied.
     * @throws IOException If the read or write operations fail.
     */
    public static void copyFileIfDifferent(Path dstFile, Path inputFile) throws IOException {
        // Test if contents match.
        boolean sameContents;
        try {
            final byte[] inputBytes = Files.readAllBytes(inputFile);
            final byte[] dstBytes = Files.readAllBytes(dstFile);
            sameContents = Arrays.equals(inputBytes, dstBytes);
        } catch (IOException ex) {
            sameContents = false;
        }

        // Only replace contents if contents does not match.
        if (!sameContents) {
            Files.createDirectories(dstFile.getParent());
            Files.copy(inputFile, dstFile, StandardCopyOption.REPLACE_EXISTING);
        }
    }

    /**
     * Perform recursive copy of a directory.
     *
     * Files are only updated if the contents are different.
     *
     * @param dstDir The destination directory of the copy operation.
     * @param inputDir The source directory of the copy operation.
     * @return List of paths of copied files. Names are relative names.
     * @throws IOException If the copy operation fails.
     * @throws IllegalArgumentException If dstDir or inputDir are not
     * directories.
     */
    public static List<Path> copyRecursivelyIfDifferent(Path dstDir, Path inputDir) throws IOException {
        if (!Files.isDirectory(inputDir))
            throw new IllegalArgumentException(inputDir + " must be a directory");
        if (!Files.isDirectory(dstDir))
            throw new IllegalArgumentException(dstDir + " must be a directory");

        // List of relative paths of input files.
        final List<Path> inputFiles = Files.find(inputDir, Integer.MAX_VALUE, (f, attr) -> attr.isRegularFile())
                .map(inputDir::relativize)
                .collect(Collectors.toList());

        for (Path inputFile : inputFiles)
            copyFileIfDifferent(dstDir.resolve(inputFile), inputDir.resolve(inputFile));

        return inputFiles;
    }

    /**
     * Assigns contents to a file.
     *
     * Does not modify the file if the contents already match.
     *
     * @param fileName The name of the file that is to be updated.
     * @param contents The new contents of the file.
     * @param charset The character set that the file should use.
     * @throws IOException If the write operation fails.
     */
    public static void setFileContents(Path fileName, String contents, Charset charset) throws IOException {
        try {
            final String oldContents = new String(Files.readAllBytes(fileName), charset);
            if (Objects.equals(oldContents, contents)) {
                LOG.log(Level.FINER, "Skipping update for {0}: contents already the same", fileName);
                return;
            }
        } catch (IOException ex) {
            // SKIP, just replace the contents anyway
        }

        LOG.log(Level.FINE, "Updating {0}", fileName);
        Files.createDirectories(fileName.getParent());
        Files.write(fileName, contents.getBytes(charset));
    }

    /**
     * Assigns contents to a file.
     *
     * Does not modify the file if the contents already match. The file contents
     * is encoded using the UTF-8 character set.
     *
     * @param fileName The name of the file that is to be updated.
     * @param contents The new contents of the file.
     * @throws IOException If the write operation fails.
     */
    public static void setFileContents(Path fileName, String contents) throws IOException {
        setFileContents(fileName, contents, UTF_8);
    }

    /**
     * Given a target directory, only retains files listed.
     *
     * Any files not in the list are erased. Any directories which are empty are
     * erased (with the exception of dir).
     *
     * @param dir The directory on which to perform the cleanup.
     * @param relNames List of relative file names of files that are to be kept.
     * @throws IOException If the file system operations fail.
     */
    public static void cleanDirExcepting(Path dir, List<Path> relNames) throws IOException {
        // Build the set of files that are to be kept.
        final Set<Path> keep = relNames.stream()
                .map(name -> {
                    // Combine transformation with type checking.
                    if (name.isAbsolute())
                        throw new IllegalArgumentException(name + " must be a relative path");

                    final Path x = dir.resolve(name).normalize();
                    if (!x.startsWith(dir))
                        throw new IllegalArgumentException("after resolving, " + name + " is not in " + dir + " (instead it is " + x + ")");
                    return x;
                })
                .collect(Collectors.toSet());

        // Build the set of files that are to be erased.
        final List<Path> toErase = Files.find(dir, Integer.MAX_VALUE, (f, attr) -> attr.isRegularFile())
                .filter(existingFile -> !keep.contains(existingFile))
                .collect(Collectors.toList());

        // Delete the to-be-erased files.
        for (Path fileName : toErase)
            Files.delete(fileName);

        // Read all directories, ordered by depth-first.
        final List<Path> subdirs = Files.find(dir, Integer.MAX_VALUE, (f, attr) -> attr.isDirectory())
                .sorted(Comparator.comparing(Path::getNameCount).reversed())
                .filter(subdir -> !Objects.equals(dir, subdir))
                .collect(Collectors.toList());

        // Erase any directories that are empty.
        for (Path subdir : subdirs) {
            assert subdir.startsWith(dir);
            try (DirectoryStream<Path> dirContentsStream = Files.newDirectoryStream(subdir)) {
                if (!dirContentsStream.iterator().hasNext()) // empty directory
                    Files.delete(subdir);
            }
        }
    }
}
