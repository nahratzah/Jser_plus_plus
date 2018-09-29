#ifndef JAVA_OBJECT_INTF_H
#define JAVA_OBJECT_INTF_H

#include <java/serialization/encdec.h>

namespace java::_erased::java::lang {
class Class;
} /* namespace java::_erased::java::lang */

namespace java {

class _reflect_ops;

/**
 * \brief Tag a type as a java type.
 * \details
 * Planned to enable general java operations, such as equals and hash support.
 * For now, it just provides tagging.
 */
class object_intf {
  friend class ::java::_reflect_ops;

 public:
  virtual ~object_intf() noexcept = 0;

 private:
  virtual auto __get_class__() const
  -> cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> = 0;
};


} /* namespace java */

#endif /* JAVA_OBJECT_INTF_H */
