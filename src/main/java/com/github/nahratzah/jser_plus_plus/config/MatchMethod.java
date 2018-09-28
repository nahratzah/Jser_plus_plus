package com.github.nahratzah.jser_plus_plus.config;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

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

    public static class Predicate {
        @JsonProperty("interface")
        private boolean interface_ = true;
        @JsonProperty("class")
        private boolean class_ = true;
        @JsonProperty("abstract")
        private boolean abstract_ = true;
        @JsonProperty("inherits")
        private List<String> inherits = new ArrayList<>();

        public boolean test(boolean isAbstract, boolean isInterface, boolean isEnum, Set<String> types) {
            return testModifiers(isAbstract, isInterface, isEnum) && testInherits(types);
        }

        private boolean testModifiers(boolean isAbstract, boolean isInterface, boolean isEnum) {
            if (isAbstract) return abstract_;
            if (isInterface) return interface_;
            return class_;
        }

        private boolean testInherits(Set<String> types) {
            return inherits.isEmpty()
                    || inherits.stream()
                            .anyMatch(types::contains);
        }
    }
}
