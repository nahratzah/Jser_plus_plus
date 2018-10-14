package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.input.Context;
import java.util.Arrays;
import static java.util.Collections.EMPTY_LIST;
import java.util.Objects;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class OverrideSelectorTest {
    @Mock
    private Context ctx;
    @Mock
    private MethodModel method, anotherMethod;
    @Mock
    private ClassType methodDeclaringClass, argumentDeclaringClass, returnClass,
            anotherMethodDeclaringClass, anotherArgumentDeclaringClass, anotherReturnClass;
    @Mock
    private JavaType xType, yType;

    private Type argumentX, argumentY,
            anotherArgumentX, anotherArgumentY;

    private BoundTemplate.ClassBinding<ClassType> argumentDeclaringType, returnType,
            anotherArgumentDeclaringType, anotherReturnType;

    private void unused_setup_code_() {
        argumentDeclaringType = new BoundTemplate.ClassBinding<>(argumentDeclaringClass, EMPTY_LIST);
        anotherArgumentDeclaringType = new BoundTemplate.ClassBinding<>(anotherArgumentDeclaringClass, EMPTY_LIST);
        returnType = new BoundTemplate.ClassBinding<>(returnClass, EMPTY_LIST);
        anotherReturnType = new BoundTemplate.ClassBinding<>(anotherReturnClass, EMPTY_LIST);

        argumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        argumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);
        anotherArgumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        anotherArgumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);

        Mockito.when(method.getDeclaringClass()).thenReturn(methodDeclaringClass);
        Mockito.when(method.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) argumentDeclaringType);
        Mockito.when(method.getReturnType()).thenReturn(returnType);
        Mockito.when(method.getArgumentTypes()).thenReturn(Arrays.asList(argumentX, argumentY));
        Mockito.when(method.getArgumentNames()).thenReturn(Arrays.asList("x", "y"));

        Mockito.when(anotherMethod.getDeclaringClass()).thenReturn(anotherMethodDeclaringClass);
        Mockito.when(anotherMethod.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) anotherArgumentDeclaringType);
        Mockito.when(anotherMethod.getReturnType()).thenReturn(anotherReturnType);
        Mockito.when(anotherMethod.getArgumentTypes()).thenReturn(Arrays.asList(anotherArgumentX, anotherArgumentY));
        Mockito.when(anotherMethod.getArgumentNames()).thenReturn(Arrays.asList("otherX", "otherY"));
    }

    @Test
    public void argumentEquality() {
        argumentDeclaringType = new BoundTemplate.ClassBinding<>(argumentDeclaringClass, EMPTY_LIST);
        anotherArgumentDeclaringType = new BoundTemplate.ClassBinding<>(anotherArgumentDeclaringClass, EMPTY_LIST);
        returnType = new BoundTemplate.ClassBinding<>(returnClass, EMPTY_LIST);
        anotherReturnType = new BoundTemplate.ClassBinding<>(anotherReturnClass, EMPTY_LIST);

        argumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        argumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);
        anotherArgumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        anotherArgumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);

        Mockito.when(method.getDeclaringClass()).thenReturn(methodDeclaringClass);
        Mockito.when(method.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) argumentDeclaringType);
        Mockito.when(method.getReturnType()).thenReturn(returnType);
        Mockito.when(method.getArgumentTypes()).thenReturn(Arrays.asList(argumentX, argumentY));
        Mockito.when(method.getArgumentNames()).thenReturn(Arrays.asList("x", "y"));

        Mockito.when(anotherMethod.getDeclaringClass()).thenReturn(anotherMethodDeclaringClass);
        Mockito.when(anotherMethod.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) anotherArgumentDeclaringType);
        Mockito.when(anotherMethod.getReturnType()).thenReturn(anotherReturnType);
        Mockito.when(anotherMethod.getArgumentTypes()).thenReturn(Arrays.asList(anotherArgumentX, anotherArgumentY));
        Mockito.when(anotherMethod.getArgumentNames()).thenReturn(Arrays.asList("otherX", "otherY"));

        final OverrideSelector selector = new OverrideSelector(ctx, method);
        final OverrideSelector anotherSelector = new OverrideSelector(ctx, anotherMethod);

        assertTrue(Objects.equals(selector, anotherSelector));
    }

    @Test
    public void constEquality() {
        argumentDeclaringType = new BoundTemplate.ClassBinding<>(argumentDeclaringClass, EMPTY_LIST);
        anotherArgumentDeclaringType = new BoundTemplate.ClassBinding<>(anotherArgumentDeclaringClass, EMPTY_LIST);
        returnType = new BoundTemplate.ClassBinding<>(returnClass, EMPTY_LIST);
        anotherReturnType = new BoundTemplate.ClassBinding<>(anotherReturnClass, EMPTY_LIST);

        argumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        argumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);
        anotherArgumentX = new BoundTemplate.ClassBinding<>(xType, EMPTY_LIST);
        anotherArgumentY = new BoundTemplate.ClassBinding<>(yType, EMPTY_LIST);

        Mockito.when(method.getDeclaringClass()).thenReturn(methodDeclaringClass);
        Mockito.when(method.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) argumentDeclaringType);
        Mockito.when(method.getReturnType()).thenReturn(returnType);
        Mockito.when(method.getArgumentTypes()).thenReturn(Arrays.asList(argumentX, argumentY));
        Mockito.when(method.getArgumentNames()).thenReturn(Arrays.asList("x", "y"));

        Mockito.when(anotherMethod.getDeclaringClass()).thenReturn(anotherMethodDeclaringClass);
        Mockito.when(anotherMethod.getArgumentDeclaringClass()).thenReturn((BoundTemplate.ClassBinding) anotherArgumentDeclaringType);
        Mockito.when(anotherMethod.getReturnType()).thenReturn(anotherReturnType);
        Mockito.when(anotherMethod.getArgumentTypes()).thenReturn(Arrays.asList(anotherArgumentX, anotherArgumentY));
        Mockito.when(anotherMethod.getArgumentNames()).thenReturn(Arrays.asList("otherX", "otherY"));

        Mockito.when(method.isConst()).thenReturn(true);
        Mockito.when(anotherMethod.isConst()).thenReturn(false);

        final OverrideSelector selector = new OverrideSelector(ctx, method);
        final OverrideSelector anotherSelector = new OverrideSelector(ctx, anotherMethod);

        assertFalse(Objects.equals(selector, anotherSelector));
    }
}
