#ifndef JAVA_OBJECT_INTF_H
#define JAVA_OBJECT_INTF_H

#include <java/serialization/encdec.h>

namespace java {


/**
 * \brief Tag a type as a java type.
 * \details
 * Planned to enable general java operations, such as equals and hash support.
 * For now, it just provides tagging.
 */
class object_intf {
 public:
  virtual ~object_intf() noexcept = 0;

#if 0 // Disable until we have proper config handling logic.
 private:
  virtual auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> = 0;
#endif
};


} /* namespace java */

#endif /* JAVA_OBJECT_INTF_H */
