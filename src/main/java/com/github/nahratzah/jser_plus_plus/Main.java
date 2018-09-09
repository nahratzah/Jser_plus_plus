package com.github.nahratzah.jser_plus_plus;

import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.input.Processor;
import com.github.nahratzah.jser_plus_plus.input.Scanner;
import static com.github.nahratzah.jser_plus_plus.input.Scanner.Options.ADD_BOOT_CLASSPATH;
import com.github.nahratzah.jser_plus_plus.output.CmakeModule;
import java.io.File;
import static java.util.Collections.EMPTY_SET;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 *
 * @author ariane
 */
public class Main {
    public static void main(String[] args) throws Exception {
        Config cfg = new Config();

        try (final Scanner s = new Scanner(ADD_BOOT_CLASSPATH)) {
            final Processor p = new Processor(s.getClassLoader(), cfg);

            final Class<?> serializable = s.getClassLoader().loadClass(java.io.Serializable.class.getName());
            try (final Stream<Class<?>> classStream = s.getClassesChecked()
                    .filter(serializable::isAssignableFrom)
                    .filter(c -> !c.isPrimitive())
                    .filter(c -> !c.isAnonymousClass())) {
                p.addClasses(classStream.collect(Collectors.toList()));
            }

            try (final CmakeModule cmake = new CmakeModule("jvm", new File("/tmp/jvm"), EMPTY_SET, EMPTY_SET, EMPTY_SET)) {
                cmake.addSupplied(
                        new File("/usr/home/ariane/programming/JSer++-2/jvm_supplied/include"),
                        new File("/usr/home/ariane/programming/JSer++-2/jvm_supplied/src"));
                p.emit(cmake);
            }
        }
    }
}
