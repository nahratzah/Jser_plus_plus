package com.github.nahratzah.jser_plus_plus.config.cplusplus;

import java.util.List;
import java.util.Optional;

/**
 * Describes a template argument.
 *
 * @author ariane
 */
public class TemplateArgument {
    /**
     * The name of the template argument.
     */
    private String name;
    /**
     * List of types the filled in parameter must extend.
     */
    private List<BoundTemplate> extendTypes;
    /**
     * List of types the filled in parameter must be a super type of.
     */
    private List<BoundTemplate> superTypes;
    /**
     * If present, the filled in type must be the same as this type.
     *
     * May be null.
     */
    private BoundTemplate same;

    /**
     * @return The name of the template argument.
     */
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    /**
     * @return List of types the filled in parameter must extend.
     */
    public List<BoundTemplate> getExtendTypes() {
        return extendTypes;
    }

    public void setExtendTypes(List<BoundTemplate> extendTypes) {
        this.extendTypes = extendTypes;
    }

    /**
     * @return List of types the filled in parameter must be a super type of.
     */
    public List<BoundTemplate> getSuperTypes() {
        return superTypes;
    }

    public void setSuperTypes(List<BoundTemplate> superTypes) {
        this.superTypes = superTypes;
    }

    /**
     * @return If present, the filled in type must be the same as this type.
     */
    public Optional<BoundTemplate> getSame() {
        return Optional.ofNullable(same);
    }

    public void setSame(Optional<BoundTemplate> same) {
        this.same = same.orElse(null);
    }
}
