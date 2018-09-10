/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.github.nahratzah.jser_plus_plus.output.builtins;

import java.util.AbstractMap;
import static java.util.Collections.EMPTY_SET;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.function.Function;

/**
 * Trivial attribute map that wraps a function.
 *
 * Used to provide calls to java functions from within templates.
 *
 * @author ariane
 */
public class FunctionAttrMap extends AbstractMap<String, Object> {
    private final Function<String, ? extends Object> fn;

    public FunctionAttrMap(Function<String, ? extends Object> fn) {
        this.fn = requireNonNull(fn);
    }

    @Override
    public Set<Entry<String, Object>> entrySet() {
        return EMPTY_SET;
    }

    @Override
    public Object get(Object key) {
        requireNonNull(key);
        return fn.apply((String) key);
    }

    @Override
    public boolean containsKey(Object key) {
        requireNonNull(key);
        return (key instanceof String) && get((String) key) != null;
    }
}
