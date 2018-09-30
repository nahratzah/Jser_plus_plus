#include <java/reflect.h>
#include <java/hash.h>

namespace java {

auto _equal_helper::hasher::operator()(const pair& x) const noexcept -> std::size_t {
  return ::java::hash_combiner(0u)
      << std::get<0>(x)
      << std::get<1>(x);
}

auto _equal_helper::eq_(cycle_ptr::cycle_gptr<const object_intf> x,
                        cycle_ptr::cycle_gptr<const object_intf> y)
-> _equal_helper& {
  using std::swap;

  if (x == y) return *this;
  if (x == nullptr || y == nullptr) {
    fail();
    return *this;
  }

  if (x.get() > y.get()) swap(x, y);
  const object_intf*const x_raw = x.get();
  const object_intf*const y_raw = y.get();

  const pair item = pair(std::move(x), std::move(y));
  if (!visited_.insert(item).second) return *this;

  try {
    x_raw->__equal__(false, *this, *y_raw);
  } catch (...) {
    visited_.erase(item);
    throw;
  }

  if (!success_) visited_.erase(item);
  return *this;
}

auto _equal_helper::eq_(cycle_ptr::cycle_gptr<const object_intf> x,
                        const object_intf& y)
-> _equal_helper& {
  auto y_obj = dynamic_cast<const ::java::_erased::java::lang::Object*>(&y);
  if (y_obj == nullptr) {
    fail();
    return *this;
  }

  return eq_(std::move(x), y_obj->shared_from_this(&y));
}

auto _equal_helper::test_(cycle_ptr::cycle_gptr<const object_intf> x,
                          cycle_ptr::cycle_gptr<const object_intf> y)
-> bool {
  using std::swap;

  if (x == y) return true;
  if (x == nullptr || y == nullptr) return false;

  if (x.get() > y.get()) swap(x, y);
  const object_intf*const x_raw = x.get();
  const object_intf*const y_raw = y.get();

  const bool saved_success = std::exchange(success_, true);
  const pair item = pair(std::move(x), std::move(y));
  if (!visited_.insert(item).second) return true;

  try {
    x_raw->__equal__(false, *this, *y_raw);
  } catch (...) {
    visited_.erase(item);
    throw;
  }

  if (!success_) visited_.erase(item);
  return std::exchange(success_, saved_success);
}

auto _equal_helper::test_(cycle_ptr::cycle_gptr<const object_intf> x,
                          const object_intf& y)
-> bool {
  auto y_obj = dynamic_cast<const ::java::_erased::java::lang::Object*>(&y);
  if (y_obj == nullptr) return false;

  return test_(std::move(x), y_obj->shared_from_this(&y));
}

} /* namespace java */
