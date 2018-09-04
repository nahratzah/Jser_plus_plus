#ifndef JAVA_LANG_OBJECT_H
#define JAVA_LANG_OBJECT_H

#include <memory>
#include <string_view>
#include <cycle_ptr/cycle_ptr.h>
#include <java/object_intf.h>
#include <java/serialization/type_def.h>
#include <java/serialization/encdec.h>
#include <java/serialization/generics.h>
#include <java/serialization/_accessor.h>

namespace java::_tags::java::lang {
struct Object;
} /* namespace java::_tags::java::lang */

namespace java::_erased::java::lang {

class Object
: public cycle_ptr::cycle_base,
  public virtual object_intf
{
 public:
  ~Object() noexcept override;
};

} /* namespace java::_erased::java::lang */

namespace java::_tags::java::lang {

struct Object {
  using erased_type = java::_erased::java::lang::Object;

  static constexpr std::size_t generics_arity = 0;
};

} /* namespace java::_tags::java::lang */

namespace java {

template<typename Base>
class _accessor<Base, java::_tags::java::lang::Object>
: public virtual Base
{
 protected:
  _accessor() = default;
  _accessor(const _accessor&) = default;
  _accessor(_accessor&&) = default;
  _accessor& operator=(const _accessor&) = default;
  _accessor& operator=(_accessor&&) = default;
  ~_accessor() = default;
};

} /* namespace java */

namespace java::lang {
using Object = java::var_ref<java::G::is<java::_tags::java::lang::Object>>;
} /* namespace java::lang */

namespace java::serialization {


template<>
struct type_def<java::_erased::java::lang::Object> {
  // Not Serializable
  using type = java::_erased::java::lang::Object;

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
