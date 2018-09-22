package com.github.nahratzah.jser_plus_plus;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Processor;
import com.github.nahratzah.jser_plus_plus.input.Scanner;
import static com.github.nahratzah.jser_plus_plus.input.Scanner.Options.ADD_BOOT_CLASSPATH;
import com.github.nahratzah.jser_plus_plus.output.CmakeModule;
import java.io.File;
import static java.util.Collections.EMPTY_SET;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Main {
    private static final Logger LOG = Logger.getLogger(Main.class.getName());
    private static final boolean ENABLE_DEBUG_LOGS = false;
    private static final File CONFIG_FILE = new File("/home/ariane/programming/JSer++-2/JvmConfig.yaml");

    public static void main(String[] args) throws Exception {
        if (ENABLE_DEBUG_LOGS) enableDebugLog(Level.FINE);

        final Config cfg = Config.fromFile(CONFIG_FILE);

        LOG.log(Level.SEVERE, "cfg.classes = {0}", cfg.getClasses());

        try (final Scanner s = new Scanner(ADD_BOOT_CLASSPATH)) {
            final Processor p = new Processor(s.getClassLoader(), cfg);

            final Class<?> serializable = s.getClassLoader().loadClass(java.io.Serializable.class.getName());
            try (final Stream<Class<?>> classStream = s.getClassesChecked()
                    .filter(serializable::isAssignableFrom)
                    .filter(c -> !c.isPrimitive())
                    .filter(c -> !c.isAnonymousClass())
                    .filter(cfg.hasConfigForClass().or(cfg.getScan().filter()))
                    .peek(c -> LOG.log(Level.FINE, "Adding scanned {0}", c))) {
                p.addClasses(classStream.collect(Collectors.toList()));
            }

            p.postProcess();

            try (final CmakeModule cmake = new CmakeModule("jvm", new File("/tmp/jvm"), EMPTY_SET, EMPTY_SET, EMPTY_SET)) {
                cmake.addSupplied(
                        new File("/usr/home/ariane/programming/JSer++-2/jvm_supplied/include"),
                        new File("/usr/home/ariane/programming/JSer++-2/jvm_supplied/src"));
                p.emit(cmake);
            }
        }
    }

    private static void enableDebugLog(Level level) {
        final Logger rootLogger = Logger.getLogger("");
        rootLogger.setLevel(level);
        for (Handler handler : rootLogger.getHandlers())
            handler.setLevel(level);
    }
}
