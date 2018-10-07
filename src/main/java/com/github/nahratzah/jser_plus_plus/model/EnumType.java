package com.github.nahratzah.jser_plus_plus.model;

import com.github.nahratzah.jser_plus_plus.config.CfgArgument;
import com.github.nahratzah.jser_plus_plus.config.CfgType;
import com.github.nahratzah.jser_plus_plus.config.Config;
import com.github.nahratzah.jser_plus_plus.config.Includes;
import com.github.nahratzah.jser_plus_plus.config.class_members.Constructor;
import com.github.nahratzah.jser_plus_plus.config.class_members.Method;
import com.github.nahratzah.jser_plus_plus.config.cplusplus.Visibility;
import com.github.nahratzah.jser_plus_plus.input.Context;
import com.github.nahratzah.jser_plus_plus.output.builtins.StCtx;
import java.util.Arrays;
import java.util.Collection;
import static java.util.Collections.EMPTY_LIST;
import static java.util.Collections.EMPTY_MAP;
import static java.util.Collections.singletonList;
import static java.util.Collections.singletonMap;
import static java.util.Collections.unmodifiableList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import static java.util.Objects.requireNonNull;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stringtemplate.v4.ST;

/**
 * An enum type.
 *
 * @author ariane
 */
public class EnumType extends ClassType {
    private static final List<String> EXTRA_PUBLIC_INCLUDES = unmodifiableList(Arrays.asList("string_view", "java/primitives.h"));
    private String enumTypeTemplate;

    public EnumType(Class<?> c) {
        super(c);
        if (!c.isEnum())
            throw new IllegalArgumentException("EnumType requires an enum class");
    }

    @Override
    public void init(Context ctx, Config cfg) {
        super.init(ctx, cfg);

        enumTypeTemplate = "::java::_static_accessor<$boundTemplateType(java.({" + getName().replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")) + "}), \"style=tag\")$>::enum_t";
        initEnumFieldAndConstructor(ctx, cfg);
        createCompareTo(ctx, cfg);
        initEnumValueStatics(ctx, cfg);
    }

    private void initEnumFieldAndConstructor(Context ctx, Config cfg) {
        // Declare the enum field.
        final FieldType enumField = new FieldType(
                ctx,
                this,
                "value",
                new CxxType(enumTypeTemplate, new Includes())
                        .prerender(ctx, EMPTY_MAP, getTemplateArgumentNames()));
        enumField.setConst(true);
        this.fields.add(enumField);

        // Declare the single-argument constructor.
        final CfgType constructorArgumentType = new CfgType();
        constructorArgumentType.setCxxType(enumTypeTemplate);
        final CfgArgument constructorArgument = new CfgArgument();
        constructorArgument.setName("v");
        constructorArgument.setType(constructorArgumentType);
        final Constructor constructorCfg = new Constructor();
        constructorCfg.setArguments(singletonList(constructorArgument));
        constructorCfg.setInitializers(singletonMap(enumField.getName(), constructorArgument.getName()));
        constructorCfg.setVisibility(Visibility.PUBLIC);
        final ClassMemberModel constructor = new ClassMemberModel.ClassConstructor(ctx, this, constructorCfg);
        this.classMembers.add(constructor);
    }

    private void createCompareTo(Context ctx, Config cfg) {
        final CfgArgument methodArgument = new CfgArgument();
        methodArgument.setName("o");
        methodArgument.setType(new CfgType(null, "const " + getName()));
        final CfgType returnType = new CfgType("::java::int_t", null);
        returnType.setIncludes(new Includes(singletonList("java/primitives.h"), EMPTY_LIST));
        final Method methodCfg = new Method();
        methodCfg.setName("compareTo");
        methodCfg.setOverride(true);
        methodCfg.setConst(true);
        methodCfg.setArguments(singletonList(methodArgument));
        methodCfg.setReturnType(returnType);
        methodCfg.setVisibility(Visibility.PUBLIC);
        methodCfg.setIncludes(new Includes(EMPTY_LIST, singletonList("java/primitives.h")));
        methodCfg.setBody("const ::java::int_t x = static_cast<::java::int_t>(value);\n"
                + "const ::java::int_t y = static_cast<::java::int_t>(o->value());\n"
                + "\n"
                + "return (x < y ? -1 : (x > y ? 1 : 0));");
        final ClassMemberModel method = new ClassMemberModel.ClassMethod(ctx, this, methodCfg);
        this.classMembers.add(method);
    }

    private void initEnumValueStatics(Context ctx, Config cfg) {
        final ST bodyTemplate = new ST(StCtx.BUILTINS, "static const $boundTemplateType(model, \"style=type, class\")$ impl =\n"
                + "    $boundTemplateType(model, \"style=type, class\")$(\n"
                + "        ::java::_direct(),\n"
                + "        cycle_ptr::make_cycle<$erasedType(model.type)$>(enum_t::$value$));\n"
                + "return impl;")
                .add("model", new BoundTemplate.ClassBinding<>(this, getTemplateArgumentNames().stream().map(BoundTemplate.VarBinding::new).collect(Collectors.toList())));

        getEnumValues().forEach(enumConstant -> {
            bodyTemplate.add("value", enumConstant.getRemappedName());
            try {
                final CfgType methodResult = new CfgType();
                methodResult.setJavaType(getName() + (getNumTemplateArguments() == 0 ? "" : "<" + String.join(", ", getTemplateArgumentNames()) + ">"));
                final Method methodCfg = new Method();
                methodCfg.setBody(bodyTemplate.render(Locale.ROOT).replaceAll(Pattern.quote("$"), Matcher.quoteReplacement("\\$")));
                methodCfg.setName(enumConstant.getRemappedName());
                methodCfg.setReturnType(methodResult);
                methodCfg.setStatic(true);
                methodCfg.setVisibility(Visibility.PUBLIC);
                methodCfg.setDocString("\\brief Enum constant " + enumConstant.getName() + ".\n\\returns An instance of " + getName());
                final ClassMemberModel.ClassMethod enumConstantMethod = new ClassMemberModel.ClassMethod(ctx, this, methodCfg);

                classMembers.add(enumConstantMethod);
            } finally {
                bodyTemplate.remove("value");
            }
        });
    }

    @Override
    public Stream<String> getImplementationIncludes(boolean publicOnly, Set<JavaType> recursionGuard) {
        return Stream.concat(
                super.getImplementationIncludes(publicOnly, recursionGuard),
                EXTRA_PUBLIC_INCLUDES.stream());
    }

    @Override
    public boolean isAbstract() {
        // Enums are never abstract, but the way enums are compiled,
        // they'll appear to be from a java.lang.Class perspective.
        // Override explicitly to remedy this trait.
        return false;
    }

    @Override
    public boolean isEnum() {
        return true;
    }

    /**
     * Retrieve all the enum values that this enum contains.
     *
     * @return Map containing as key the original name of the enum constant,
     * mapped to the remapped name.
     */
    public Collection<EnumConstant> getEnumValues() {
        return Arrays.stream(c.getEnumConstants())
                .map(Enum.class::cast)
                .map(e -> e.name())
                .map(EnumConstant::new)
                .collect(Collectors.toList());
    }

    /**
     * Mangle enum constant names, such that they won't clash with defines.
     *
     * @param name The name of the enum constant.
     * @return Mangled name of the enum constant.
     */
    private static String remapEnumConstant(String name) {
        return name.substring(0, 1).toUpperCase(Locale.ROOT) + name.substring(1).toLowerCase(Locale.ROOT);
    }

    /**
     * Track enum names and their remapped name.
     */
    public static final class EnumConstant {
        public EnumConstant(String name) {
            this.name = requireNonNull(name);
        }

        public String getName() {
            return name;
        }

        public String getRemappedName() {
            return remapEnumConstant(name);
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 37 * hash + Objects.hashCode(this.name);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null) return false;
            if (getClass() != obj.getClass()) return false;
            final EnumConstant other = (EnumConstant) obj;
            if (!Objects.equals(this.name, other.name)) return false;
            return true;
        }

        private final String name;
    }
}
