package com.github.nahratzah.jser_plus_plus.misc;

import java.util.Map;

/**
 * A simple map entry.
 *
 * @author ariane
 */
public class SimpleMapEntry<K, V> implements Map.Entry<K, V> {
    public SimpleMapEntry(K key, V value) {
        this.key = key;
        this.value = value;
    }

    @Override
    public K getKey() {
        return key;
    }

    @Override
    public V getValue() {
        return value;
    }

    @Override
    public V setValue(V value) {
        throw new UnsupportedOperationException(getClass().getSimpleName() + " is read only.");
    }

    /**
     * Hash code for the this, according to the specification in
     * {@link Map.Entry#hashCode() Map.Entry}.
     */
    @Override
    public int hashCode() {
        return (key == null ? 0 : key.hashCode()) ^ (value == null ? 0 : value.hashCode());
    }

    /**
     * Equality comparison, according to the specification in
     * {@link Map.Entry#equals(java.lang.Object) Map.Entry}.
     *
     * Surprisingly, the specification says identity comparison is not
     * performed.
     *
     * @param o The object to compare against, for equality.
     * @return True if the two map entries have the same key and value.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null) return false;
        if (!(o instanceof Map.Entry)) return false;

        final Map.Entry other = (Map.Entry) o;
        return (key == null ? other.getKey() == null : key.equals(other.getKey()))
                && (value == null ? other.getValue() == null : value.equals(other.getValue()));
    }

    private final K key;
    private final V value;
}
