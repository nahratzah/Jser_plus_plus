package com.github.nahratzah.jser_plus_plus.misc;

import com.github.nahratzah.jser_plus_plus.model.BoundTemplate;
import com.github.nahratzah.jser_plus_plus.model.ConstType;
import com.github.nahratzah.jser_plus_plus.model.CxxType;
import com.github.nahratzah.jser_plus_plus.model.Type;
import java.util.Arrays;
import java.util.Comparator;
import java.util.function.ToIntBiFunction;

/**
 * Comparator for types.
 *
 * Performs an arbitrary, but stable, ordering.
 *
 * @author ariane
 */
public class TypeComparator implements Comparator<Type> {
    @Override
    public int compare(Type o1, Type o2) {
        final Distinction d1 = Distinction.find(o1);
        final Distinction d2 = Distinction.find(o2);
        if (d1 == d2) return d1.compare(o1, o2);
        return d1.compareTo(d2);
    }

    private int compareCxx(CxxType o1, CxxType o2) {
        return o1.getPreRendered().compareTo(o2.getPreRendered());
    }

    private static enum Distinction implements Comparator<Type> {
        CONST_TYPE(ConstType.class, Comparator.comparing(ConstType::getType, new TypeComparator())::compare),
        CXX_TYPE(CxxType.class, (o1, o2) -> o1.getPreRendered().compareTo(o2.getPreRendered())),
        TEMPLATE_TYPE(BoundTemplate.class, (o1, o2) -> o1.compareTo(o2));

        private <T extends Type> Distinction(Class<T> cls, ToIntBiFunction<? super T, ? super T> impl) {
            this.cls = cls;
            this.impl = impl;
        }

        public static Distinction find(Type x) {
            return Arrays.stream(values())
                    .filter(d -> d.isClassMatch(x))
                    .findFirst()
                    .orElseThrow(() -> new IllegalStateException("Unrecognized type."));
        }

        public boolean isClassMatch(Type x) {
            return cls.isInstance(x);
        }

        @Override
        public int compare(Type x, Type y) {
            assert isClassMatch(x) && isClassMatch(y);
            return impl.applyAsInt(x, y);
        }

        private final Class cls;
        private final ToIntBiFunction impl;
    }
}
