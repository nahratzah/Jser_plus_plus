package com.github.nahratzah.jser_plus_plus.misc;

import java.util.Collection;
import java.util.Comparator;
import java.util.Iterator;
import static java.util.Objects.requireNonNull;

/**
 * Invokes a comparator on elements in a list.
 *
 * @author ariane
 * @param <T> Type of the elements in the collection.
 */
public class ListComparator<T> implements Comparator<Collection<? extends T>> {
    public ListComparator() {
        this.inner = (Comparator<? super T>) Comparator.naturalOrder();
    }

    public ListComparator(Comparator<? super T> inner) {
        this.inner = requireNonNull(inner);
    }

    @Override
    public int compare(Collection<? extends T> x, Collection<? extends T> y) {
        final Iterator<? extends T> xIter = x.iterator();
        final Iterator<? extends T> yIter = y.iterator();

        while (xIter.hasNext() && yIter.hasNext()) {
            final int cmp = inner.compare(xIter.next(), yIter.next());
            if (cmp != 0) return cmp;
        }

        if (xIter.hasNext()) return 1;
        if (yIter.hasNext()) return -1;
        return 0;
    }

    private final Comparator<? super T> inner;
}
