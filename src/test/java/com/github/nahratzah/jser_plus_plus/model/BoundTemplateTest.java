package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Arrays;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.EMPTY_SET;
import static java.util.Collections.singleton;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import org.hamcrest.Matchers;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertThat;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class BoundTemplateTest {
    @Mock
    private Context ctx;
    @Mock
    private ClassType mapType, thisClass;

    private ClassType clsType, strType;
    private Map<String, BoundTemplate> variables;
    private BoundTemplate.ClassBinding<ClassType> thisType;
    private BoundTemplate.VarBinding kVar, vVar;

    @Before
    public void setup() {
        kVar = new BoundTemplate.VarBinding("k var");
        vVar = new BoundTemplate.VarBinding("v var");

        variables = new HashMap<>();
        variables.put("K", kVar);
        variables.put("V", vVar);

        thisType = new BoundTemplate.ClassBinding<>(thisClass, EMPTY_LIST);

        Mockito.when(mapType.getNumTemplateArguments()).thenReturn(2);

        clsType = new ClassType(Class.class);
        strType = new ClassType(String.class);
    }

    @Test
    public void parse() {
        Mockito.when(ctx.resolveClass("java.util.Map")).thenReturn(mapType);

        final Type type = BoundTemplate.fromString("java . util . Map<K, V>", ctx, variables, thisType);
        Mockito.verify(ctx, Mockito.times(1)).resolveClass(Mockito.eq("java.util.Map"));
        assertEquals(
                new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kVar, vVar)),
                type);

        Mockito.verifyNoMoreInteractions(ctx);
    }

    @Test
    public void parseNestedAny() {
        Mockito.when(ctx.resolveClass(Mockito.anyString())).thenAnswer((args) -> {
            if ("java.lang.Class".equals(args.getArgumentAt(0, String.class)))
                return clsType;
            if ("java.lang.String".equals(args.getArgumentAt(0, String.class)))
                return strType;
            throw new AssertionError("Expected Map or String lookup, got " + args.getArgumentAt(0, Object.class));
        });

        final Type type = BoundTemplate.fromString("java.lang.Class<? extends java.lang.String>", ctx, variables, thisType);
        Mockito.verify(ctx, Mockito.times(1)).resolveClass(Mockito.eq("java.lang.Class"));
        Mockito.verify(ctx, Mockito.times(1)).resolveClass(Mockito.eq("java.lang.String"));

        assertThat(type, Matchers.instanceOf(BoundTemplate.ClassBinding.class));
        final BoundTemplate.ClassBinding<JavaType> classType = (BoundTemplate.ClassBinding<JavaType>) type;
        assertSame(clsType, classType.getType());
        assertThat(classType.getBindings().get(0), Matchers.instanceOf(BoundTemplate.Any.class));
        final BoundTemplate.Any lastBinding = (BoundTemplate.Any) classType.getBindings().get(0);
        assertThat(lastBinding.getSuperTypes(), Matchers.empty());
        final BoundTemplate extendBinding = lastBinding.getExtendTypes().iterator().next();
        assertThat(extendBinding, Matchers.instanceOf(BoundTemplate.ClassBinding.class));
        assertThat(((BoundTemplate.ClassBinding<JavaType>) extendBinding).getType(), Matchers.sameInstance(strType));
        assertThat(((BoundTemplate.ClassBinding<JavaType>) extendBinding).getBindings(), Matchers.empty());

        Mockito.verifyNoMoreInteractions(ctx);
    }

    @Test
    public void parseThis() {
        assertSame(
                thisType,
                BoundTemplate.fromString("this", ctx, variables, thisType));

        Mockito.verifyZeroInteractions(ctx);
    }

    @Test
    public void parseConst() {
        Mockito.when(ctx.resolveClass("java.util.Map")).thenReturn(mapType);

        final Type type = BoundTemplate.fromString("const java.util.Map<K, V>", ctx, variables, thisType);
        Mockito.verify(ctx, Mockito.times(1)).resolveClass(Mockito.eq("java.util.Map"));
        assertThat(
                type,
                Matchers.allOf(
                        Matchers.instanceOf(ConstType.class),
                        Matchers.hasProperty("type", Matchers.equalTo(new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kVar, vVar))))
                ));

        Mockito.verifyNoMoreInteractions(ctx);
    }

    @Test
    public void unboundArguments() {
        assertEquals(
                EMPTY_SET,
                new BoundTemplate.Any().getUnresolvedTemplateNames());

        assertEquals(
                singleton("k var"),
                kVar.getUnresolvedTemplateNames());

        assertEquals(
                EMPTY_SET,
                thisType.getUnresolvedTemplateNames());

        assertEquals(
                new HashSet<>(Arrays.asList("k var", "v var")),
                new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kVar, vVar)).getUnresolvedTemplateNames());

        assertEquals(
                new HashSet<>(Arrays.asList("k var", "v var")),
                new BoundTemplate.ArrayBinding(
                        new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kVar, vVar)), 15).getUnresolvedTemplateNames());
    }

    @Test
    public void rebindOnce() {
        final BoundTemplate.ClassBinding<ClassType> kvMap = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(new BoundTemplate.VarBinding("K"), new BoundTemplate.VarBinding("V")));
        final BoundTemplate.ClassBinding<ClassType> vkMap = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(new BoundTemplate.VarBinding("V"), new BoundTemplate.VarBinding("K")));
        final BoundTemplate expect = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kvMap, vkMap));

        final Map<String, BoundTemplate> rebindMap = new HashMap<>();
        rebindMap.put("K", kvMap);
        rebindMap.put("V", vkMap);

        final BoundTemplate.ClassBinding<ClassType> rebound = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(new BoundTemplate.VarBinding("K"), new BoundTemplate.VarBinding("V")))
                .rebind(rebindMap);
        assertEquals(expect, rebound);
    }

    @Test
    public void rebindPartial() {
        final BoundTemplate.ClassBinding<ClassType> kvMap = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(new BoundTemplate.VarBinding("K"), new BoundTemplate.VarBinding("V")));
        final BoundTemplate expect = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(kvMap, new BoundTemplate.VarBinding("V")));

        final Map<String, BoundTemplate> rebindMap = new HashMap<>();
        rebindMap.put("K", kvMap);

        final BoundTemplate.ClassBinding<ClassType> rebound = new BoundTemplate.ClassBinding<>(mapType, Arrays.asList(new BoundTemplate.VarBinding("K"), new BoundTemplate.VarBinding("V")))
                .rebind(rebindMap);
        assertEquals(expect, rebound);
    }

    @Test
    public void intBindingIsEqual() {
        final Type x = new BoundTemplate.ClassBinding<>(PrimitiveType.INT, EMPTY_LIST);
        final Type y = new BoundTemplate.ClassBinding<>(PrimitiveType.INT, EMPTY_LIST);

        assertEquals(x.hashCode(), y.hashCode());
        assertEquals(x, y);
    }
}
