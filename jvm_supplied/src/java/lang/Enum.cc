#include <java/lang/Enum.h>

namespace java::serialization {


auto template_type_def<java::lang::Enum>::get_class()
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  using namespace java::serialization::stream;

  const auto result = cycle_ptr::make_cycle<new_class_desc__class_desc>(
      field_descriptor(java_class_name()),
      0); // Enum always has serialVersionUID of zero.
  result->info.flags = SC_SERIALIZABLE | SC_ENUM;
  result->info.annotation = {};
  result->info.super = nullptr;
  return result;
}

auto template_type_def<java::lang::Enum>::get_specialized_class(std::u16string_view name)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc> {
  using namespace java::serialization::stream;

  const auto result = cycle_ptr::make_cycle<new_class_desc__class_desc>(
      field_descriptor(name),
      0); // Enum always has serialVersionUID of zero.
  result->info.flags = SC_SERIALIZABLE | SC_ENUM;
  result->info.annotation = {};
  result->info.super = get_class();
  return result;
}


} /* namespace java::serialization */
