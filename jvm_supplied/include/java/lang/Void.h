#ifndef JAVA_LANG_VOID_H
#define JAVA_LANG_VOID_H

#include <memory>
#include <string_view>
#include <cycle_ptr/cycle_ptr.h>
#include <java/serialization/type_def.h>
#include <java/serialization/encdec.h>

namespace java::lang {


class Void; // Not instantiable.


} /* namespace java::lang */

namespace java::serialization {


template<>
struct type_def<java::lang::Void> {
  // Not Serializable
  using type = java::lang::Void;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"java.lang.Void"sv;
  }

  // Even non-serializable classes have a get_class method,
  // since while the type is not serializable, type.getClass() is!
  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;
};


} /* namespace java::serialization */

#endif /* JAVA_LANG_VOID_H */
