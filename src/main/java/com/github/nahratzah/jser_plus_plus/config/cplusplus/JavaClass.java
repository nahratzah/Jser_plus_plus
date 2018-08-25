package com.github.nahratzah.jser_plus_plus.config.cplusplus;

import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.java.ReflectUtil;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 *
 * @author ariane
 */
public class JavaClass {
    public JavaClass() {
        // SKIP
    }

    public void init(Context ctx, Class<?> c, List<String> templateArgNames) {
        this.name = c.getName();

        final List<? extends TypeVariable<? extends Class<?>>> cArgs = Arrays.asList(c.getTypeParameters());
        if (cArgs.size() != templateArgNames.size())
            throw new IllegalArgumentException("incorrect number of template arguments");

        final Map<String, String> templateArgRename = buildRenameMap(cArgs, templateArgNames);

        final Iterator<? extends TypeVariable<? extends Class<?>>> cArgsIter = cArgs.iterator();
        final Iterator<String> templateArgNamesIter = templateArgNames.iterator();
        while (templateArgNamesIter.hasNext()) {
            final TypeVariable<? extends Class<?>> cArgsValue = cArgsIter.next();
            final String argName = templateArgNamesIter.next();
            final Type[] bounds = cArgsValue.getBounds();

            final TemplateArgument arg = new TemplateArgument();
            arg.setName(argName);
            arg.setExtendTypes(Arrays.stream(bounds)
                    .map(x -> {
                        return ReflectUtil.visitType(x, new ReflectUtil.Visitor<BoundTemplate>() {
                            @Override
                            public BoundTemplate apply(Class<?> var) {
                                final JavaClass varImpl = ctx.resolveClass(var);
                                // XXX bind template variables in varImpl to their defaults.
                                // XXX return new BoundTemplate() of varImpl with its defaults.
                                throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
                            }

                            @Override
                            public BoundTemplate apply(TypeVariable<?> var) {
                                return new BoundTemplate.VarBinding(templateArgRename.get(var.getName()));
                            }

                            @Override
                            public BoundTemplate apply(GenericArrayType var) {
                                final BoundTemplate arrayElementType = ReflectUtil.visitType(var.getGenericComponentType(), this);
                                final BoundTemplate arrayType;
                                if (arrayElementType instanceof BoundTemplate.ArrayBinding) {
                                    arrayType = new BoundTemplate.ArrayBinding(((BoundTemplate.ArrayBinding) arrayElementType).getType(), ((BoundTemplate.ArrayBinding) arrayElementType).getExtents());
                                } else {
                                    arrayType = new BoundTemplate.ArrayBinding(arrayElementType, 1);
                                }
                                return arrayType;
                            }

                            @Override
                            public BoundTemplate apply(WildcardType var) {
                                // XXX implement BoundTemplate that encapsulates an Object.
                                throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
                            }

                            @Override
                            public BoundTemplate apply(ParameterizedType var) {
                                final JavaClass varImpl = ctx.resolveClass((Class<?>) var.getRawType());
                                final List<BoundTemplate> args = Arrays.stream(var.getActualTypeArguments())
                                        .map(arg -> ReflectUtil.visitType(arg, this))
                                        .collect(Collectors.toList());
                                return new BoundTemplate.ClassBinding(varImpl, args);
                            }
                        });
                    })
                    .collect(Collectors.toList()));
            this.templateArguments.add(arg);
        }
    }

    public String getName() {
        return name;
    }

    public List<TemplateArgument> getTemplateArguments() {
        return templateArguments;
    }

    /**
     * Build a map containing the mapping from Java Class template arguments to
     * C++ class template arguments.
     *
     * @param cArgs List of java class arguments.
     * @param templateArgNames Destination list of template argument names.
     * @return Mapping from names in {@code cArgs} names to
     * {@code templateArgNames} names.
     * @throws IllegalArgumentException If the lists are not of equal size.
     */
    private static Map<String, String> buildRenameMap(List<? extends TypeVariable<? extends Class<?>>> cArgs, List<String> templateArgNames) {
        final Map<String, String> result = new HashMap<>();

        final Iterator<? extends TypeVariable<? extends Class<?>>> cArgsIter = cArgs.iterator();
        final Iterator<String> templateArgNamesIter = templateArgNames.iterator();
        while (cArgsIter.hasNext() && templateArgNamesIter.hasNext()) {
            final TypeVariable<? extends Class<?>> cArgsValue = cArgsIter.next();
            final String argName = templateArgNamesIter.next();
            result.put(cArgsValue.getName(), argName);
        }

        if (cArgsIter.hasNext() || templateArgNamesIter.hasNext())
            throw new IllegalArgumentException("incorrect number of template arguments");
        return result;
    }

    /**
     * List of template arguments to the class.
     */
    private List<TemplateArgument> templateArguments = new ArrayList<>();

    /**
     * Name of the java class.
     */
    private String name;
}
