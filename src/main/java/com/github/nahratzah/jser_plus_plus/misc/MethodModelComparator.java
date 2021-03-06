package com.github.nahratzah.jser_plus_plus.misc;

import static com.github.nahratzah.jser_plus_plus.model.ClassType.VIRTUAL_FUNCTION_PREFIX;
import com.github.nahratzah.jser_plus_plus.model.MethodModel;
import com.github.nahratzah.jser_plus_plus.model.Type;
import java.util.Comparator;
import java.util.Objects;

/**
 * Compare two method models.
 *
 * @author ariane
 */
public class MethodModelComparator implements Comparator<MethodModel> {
    @Override
    public int compare(MethodModel o1, MethodModel o2) {
        int cmp = 0;
        if (cmp == 0)
            cmp = Boolean.compare(o1.getName().startsWith(VIRTUAL_FUNCTION_PREFIX), o2.getName().startsWith(VIRTUAL_FUNCTION_PREFIX));
        if (cmp == 0)
            cmp = Comparator.<String>naturalOrder().compare(o1.getDeclaringClass().getName(), o2.getDeclaringClass().getName());
        if (cmp == 0)
            cmp = o1.getName().compareTo(o2.getName());
        if (cmp == 0)
            cmp = o1.getVisibility().compareTo(o2.getVisibility());
        if (cmp == 0)
            cmp = Comparator.<Boolean>reverseOrder().compare(o1.isStatic(), o2.isStatic());
        if (cmp == 0)
            cmp = Comparator.<Boolean>reverseOrder().compare(o1.isConst(), o2.isConst());
        if (cmp == 0)
            cmp = argumentComparator.compare(o1.getArgumentTypes(), o2.getArgumentTypes());
        if (cmp == 0)
            cmp = typeComparator.compare(o1.getArgumentDeclaringClass(), o2.getArgumentDeclaringClass());
        if (cmp == 0)
            cmp = typeComparator.compare(o1.getReturnType(), o2.getReturnType());
        if (cmp == 0 && !Objects.equals(o1, o2))
            throw new IllegalStateException("Comparator is missing a distinguishing quality of methods...");
        return cmp;
    }

    private final TypeComparator typeComparator = new TypeComparator();
    private final ListComparator<Type> argumentComparator = new ListComparator<>(typeComparator);
}
