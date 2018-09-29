#ifndef JAVA_OBJECT_INTF_H
#define JAVA_OBJECT_INTF_H

#include <java/serialization/encdec.h>
#include <cstddef>
#include <functional>

namespace java {
class _reflect_ops;
class object_intf;
} /* namespace java */

namespace java::_erased::java::lang {
class Class;
} /* namespace java::_erased::java::lang */

namespace std {

template<>
struct hash<::java::object_intf*> {
  auto operator()(const ::java::object_intf* ptr) const noexcept -> std::size_t;
};

template<>
struct hash<const ::java::object_intf*>
: hash<::java::object_intf*>
{};

} /* namespace std */

namespace java {


/**
 * \brief Tag a type as a java type.
 * \details
 * Planned to enable general java operations, such as equals and hash support.
 * For now, it just provides tagging.
 */
class object_intf {
  friend ::java::_reflect_ops;
  friend ::std::hash<object_intf*>;

 public:
  virtual ~object_intf() noexcept = 0;

 private:
  ///\brief Get the class of this object.
  virtual auto __get_class__() const
  -> cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> = 0;

  /**
   * \brief Retrieve the hash code for this object.
   * \param specialized If set, the object_intf base case does not use the
   * object address in the returned hash code.
   * \param max_cascade Maximum cascade level. If this becomes 0, the hash code
   * will not descend into member fields. This parameter exists to guard
   * against data with infinite recursion.
   */
  virtual auto __hash_code__(bool specialized, std::size_t max_cascade) const noexcept
  -> ::std::size_t;
};


} /* namespace java */

#endif /* JAVA_OBJECT_INTF_H */
