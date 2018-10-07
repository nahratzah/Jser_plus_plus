#include <java/serialization/decoder.h>
#include <java/serialization/exception.h>
#include <cassert>
#include <utility>

namespace java::serialization {

decoder::decoder() = default;
decoder::~decoder() noexcept = default;

auto decoder::get_initial() -> java::lang::Object {
  if (stage_ <= Unstarted) {
    // Acquire lock.
    if (::std::exchange(in_progress_, true))
      throw decoding_error("recursion mandated during decoding");

    assert(stage_ == Unstarted);
    assert(object_ == nullptr);
    object_ = init();
    stage_ = (object_ == nullptr ? Complete : Initial);
    in_progress_ = false;
  }

  assert(stage_ >= Initial);
  return object_;
}

auto decoder::get_comparable() -> java::lang::Object {
  if (stage_ < Comparable)
    ensure_comparable(this->shared_from_this(this));
  assert(stage_ >= Comparable);
  return object_;
}

auto decoder::get_complete() -> java::lang::Object {
  if (stage_ < Complete)
    ensure_complete(this->shared_from_this(this));
  assert(stage_ >= Complete);
  return object_;
}

auto decoder::ensure_comparable(::cycle_ptr::cycle_gptr<decoder> initial)
-> void {
  dependent_set done;
  dependent_set pending;
  pending.insert(std::move(initial));

  while (!pending.empty()) {
#if __has_cpp_attribute(__cpp_lib_node_extract) >= 201606
    auto current_node = pending.extract(pending.begin());
    const auto& current = current_node.value();
    if (!done.insert(std::move(current_node)).inserted) continue; // Already processed.
#else
    const auto current = *pending.begin();
    pending.erase(pending.begin());
    if (!done.insert(current).second) continue; // Already processed.
#endif

    // If the current stage is Unstarted, this would be indicative of a bug.
    if (current->stage_ == Unstarted)
      throw decoding_error("unstarted decoder dependency looks very suspect");

    // Change current to Comparable stage, and add its dependents to the set.
    if (current->stage_ == Initial) {
      // Acquire lock.
      if (std::exchange(current->in_progress_, true))
        throw decoding_error("recursion mandated during decoding");

      dependent_set cascade = current->init_comparable();
#if __has_cpp_attribute(__cpp_lib_node_extract) >= 201606
      pending.merge(std::move(cascade));
#else
      pending.insert(cascade.begin(), cascade.end());
#endif
    }
  }

  // We've successfully processed the `initial` member
  // and all its recursive dependencies.
  // We now mark all decoders as Comparable and release the lock.
  std::for_each(
      done.begin(), done.end(),
      [](const auto& decoder_ptr) {
        if (decoder_ptr->stage_ == Initial) {
          assert(decoder_ptr->in_progress_);

          decoder_ptr->stage_ = Comparable;
          decoder_ptr->in_progress_ = false;
        } else {
          assert(!decoder_ptr->in_progress_);
        }
      });
}

auto decoder::ensure_complete(::cycle_ptr::cycle_gptr<decoder> initial)
-> void {
  dependent_set done;
  dependent_set pending;
  pending.insert(std::move(initial));

  while (!pending.empty()) {
#if __has_cpp_attribute(__cpp_lib_node_extract) >= 201606
    auto current_node = pending.extract(pending->begin());
    const auto& current = current_node.value();
    if (!done.insert(current_node).inserted) continue; // Already processed.
#else
    const auto current = *pending.begin();
    pending.erase(pending.begin());
    if (!done.insert(current).second) continue; // Already processed.
#endif

    // If the current stage is not Comparable, first elevate to that level.
    if (current->stage_ < Comparable) ensure_comparable(current);

    // Change current to Complete stage, and add its dependents to the set.
    if (current->stage_ == Comparable) {
      // Acquire lock.
      if (std::exchange(current->in_progress_, true))
        throw decoding_error("recursion mandated during decoding");

      dependent_set cascade = current->complete();
#if __has_cpp_attribute(__cpp_lib_node_extract) >= 201606
      pending.merge(std::move(cascade));
#else
      pending.insert(cascade.begin(), cascade.end());
#endif
    }
  }

  // We've successfully processed the `initial` member
  // and all its recursive dependencies.
  // We now mark all decoders as Complete and release the lock.
  std::for_each(
      done.begin(), done.end(),
      [](const auto& decoder_ptr) {
        if (decoder_ptr->stage_ == Comparable) {
          assert(decoder_ptr->in_progress_);

          decoder_ptr->stage_ = Complete;
          decoder_ptr->in_progress_ = false;
        } else {
          assert(!decoder_ptr->in_progress_);
        }
      });
}

} /* namespace java::serialization */
