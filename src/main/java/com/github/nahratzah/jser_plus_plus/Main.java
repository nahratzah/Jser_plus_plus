package com.github.nahratzah.jser_plus_plus;

import com.github.nahratzah.jser_plus_plus.config.ClassName;
import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Processor;
import com.github.nahratzah.jser_plus_plus.input.Scanner;
import static com.github.nahratzah.jser_plus_plus.input.Scanner.Options.ADD_BOOT_CLASSPATH;
import com.github.nahratzah.jser_plus_plus.output.CmakeModule;
import java.io.File;
import java.io.PrintWriter;
import java.io.StringWriter;
import static java.util.Collections.EMPTY_SET;
import java.util.logging.ConsoleHandler;
import java.util.logging.Formatter;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
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
    private static final File CONFIG_DIR = new File("/home/ariane/programming/JSer++-2/config");
    private static final File OUTPUT_DIR = new File("/tmp/jvm");

    public static void main(String[] args) throws Exception {
        configureLogging();
        if (ENABLE_DEBUG_LOGS) enableDebugLog(Level.FINE);

        LOG.log(Level.INFO, "Loading configuration...");
        final Config cfg = Config.fromDir(CONFIG_DIR);

        LOG.log(Level.INFO, "Collecting classes...");
        try (final Scanner s = new Scanner(ADD_BOOT_CLASSPATH)) {
            final Processor p = new Processor(s.getClassLoader(), cfg);

            LOG.log(Level.INFO, "Adding {0} explicitly configured classes...", cfg.getClasses().size());
            for (ClassName name : cfg.getClasses().keySet()) {
                final Class<?> c = s.getClassLoader().loadClass(name.getName());
                LOG.log(Level.FINE, "Adding explicitly mentioned {0}", c);
                p.addClass(c);
            }

            LOG.log(Level.INFO, "Applying scanner...");
            final Class<?> serializable = s.getClassLoader().loadClass(java.io.Serializable.class.getName());
            try (final Stream<Class<?>> classStream = s.getClassesChecked()
                    .filter(c -> !c.isPrimitive())
                    .filter(c -> !c.isAnonymousClass())
                    .filter(cfg.getScan().filter().and(serializable::isAssignableFrom))
                    .peek(c -> LOG.log(Level.FINE, "Adding scanned {0}", c))) {
                p.addClasses(classStream.collect(Collectors.toList()));
            }

            LOG.log(Level.INFO, "{0} classes selected.", p.getNumClasses());

            LOG.log(Level.INFO, "Postprocessing...");
            p.postProcess();

            LOG.log(Level.INFO, "Updating files...");
            try (final CmakeModule cmake = new CmakeModule(cfg.getModule(), OUTPUT_DIR, EMPTY_SET, EMPTY_SET, EMPTY_SET)) {
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
        for (Handler handler : rootLogger.getHandlers()) {
            if (handler instanceof ConsoleHandler)
                handler.setLevel(level);
        }
    }

    private static void configureLogging() {
        Formatter formatter = new FormatterImpl();

        final Logger rootLogger = Logger.getLogger("");
        for (Handler handler : rootLogger.getHandlers()) {
            if (handler instanceof ConsoleHandler)
                handler.setFormatter(formatter);
        }
    }

    private static class FormatterImpl extends Formatter {
        @Override
        public synchronized String format(LogRecord record) {
            final String level = levelToString(record.getLevel());
            final String message = formatMessage(record);

            final String throwable;
            if (record.getThrown() != null) {
                final StringWriter sw = new StringWriter();
                try (final PrintWriter pw = new PrintWriter(sw)) {
                    pw.println();
                    record.getThrown().printStackTrace(pw);
                }
                throwable = sw.toString();
            } else {
                throwable = "";
            }

            return level + ": " + message + "\n" + throwable;
        }

        private static String levelToString(Level level) {
            if (level.intValue() >= Level.SEVERE.intValue()) return "ERR";
            if (level.intValue() >= Level.WARNING.intValue()) return "WRN";
            if (level.intValue() >= Level.INFO.intValue()) return "INF";
            if (level.intValue() >= Level.CONFIG.intValue()) return "CFG";
            return "DBG";
        }
    }
}
