package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Methods that are applied on classes or interfaces, selected by a predicate.
 *
 * @author ariane
 */
public class MatchMethod {
    @JsonProperty("predicate")
    private Predicate predicate = new Predicate();
    @JsonProperty("members")
    private List<ClassMember> members = new ArrayList<>();

    public Predicate getPredicate() {
        return predicate;
    }

    public List<ClassMember> getMembers() {
        return members;
    }

    public static class Predicate implements java.util.function.Predicate<Class<?>> {
        private static final Logger LOG = Logger.getLogger(Predicate.class.getName());
        @JsonProperty("interface")
        private boolean interface_ = true;
        @JsonProperty("class")
        private boolean class_ = true;
        @JsonProperty("abstract")
        private boolean abstract_ = true;
        @JsonProperty("inherits")
        private List<String> inherits = new ArrayList<>();

        @Override
        public boolean test(Class<?> type) {
            return testModifiers(type) && testInherits(type);
        }

        private boolean testModifiers(Class<?> type) {
            if (type.isEnum() || type.isArray()) return false;
            if ((type.getModifiers() & Modifier.INTERFACE) != 0)
                return interface_;
            if ((type.getModifiers() & Modifier.ABSTRACT) != 0)
                return abstract_;
            return class_;
        }

        private boolean testInherits(Class<?> type) {
            final ClassLoader loader = Optional.ofNullable(type.getClassLoader()).orElseGet(ClassLoader::getSystemClassLoader);

            return inherits.stream()
                    .allMatch(c -> {
                        final Class<?> cls;
                        try {
                            cls = loader.loadClass(c);
                        } catch (ClassNotFoundException ex) {
                            LOG.log(Level.WARNING, "Unable to resolve class " + c, ex);
                            return false;
                        }

                        final boolean result = cls.isAssignableFrom(type);
                        LOG.log(Level.FINE, "({0}).isAssignableFrom({1}) -> {2}", new Object[]{cls, type, result});
                        return result;
                    });
        }
    }
}
