package com.github.nahratzah.jser_plus_plus.model;

import java.util.ArrayList;
import static java.util.Collections.unmodifiableList;
import java.util.List;
import static java.util.Objects.requireNonNull;

/**
 * Models a single class template argument.
 *
 * Class template arguments only allow named arguments of the form '? extends
 * ...'.
 *
 * @author ariane
 */
public class ClassTemplateArgument {
    public ClassTemplateArgument() {
    }

    public ClassTemplateArgument(String name) {
        this.name = requireNonNull(name);
    }

    public ClassTemplateArgument(String name, Iterable<? extends BoundTemplate> extendBounds) {
        this.name = requireNonNull(name);
        extendBounds.forEach(this.extendBounds::add);
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public List<BoundTemplate> getExtendBounds() {
        return unmodifiableList(extendBounds);
    }

    public void addExtendBounds(BoundTemplate extendBound) {
        this.extendBounds.add(extendBound);
    }

    public void addExtendBounds(Iterable<? extends BoundTemplate> extendBound) {
        extendBound.forEach(this::addExtendBounds);
    }

    private String name;
    private final List<BoundTemplate> extendBounds = new ArrayList<>();
}
