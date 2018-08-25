#ifndef JAVA_LANG_OBJECT_H
#define JAVA_LANG_OBJECT_H

#include <memory>
#include <string_view>
#include <cycle_ptr/cycle_ptr.h>
#include <java/object_intf.h>
#include <java/serialization/type_def.h>
#include <java/serialization/encdec.h>

namespace java::lang {


class Object
: public cycle_ptr::cycle_base,
  public virtual object_intf
{
 public:
  Object() = default;
  explicit Object(cycle_ptr::unowned_cycle_t u) : cycle_ptr::cycle_base(u) {}
  virtual ~Object() noexcept;
};


} /* namespace java::lang */

namespace java::serialization {


template<>
struct type_def<java::lang::Object> {
  // Not Serializable
  using type = java::lang::Object;

  static constexpr std::u16string_view java_class_name() {
    using namespace std::string_view_literals;
    return u"java.lang.Object"sv;
  }

  // Even non-serializable classes have a get_class method,
  // since while the type is not serializable, type.getClass() is!
  static auto get_class()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;
};


} /* namespace java::serialization */

#endif /* JAVA_LANG_OBJECT_H */
