#include <java/serialization/decoder.h>
#include <java/serialization/exception.h>
#include <java/serialization/module.h>
#include <cassert>
#include <utility>
#include <stdexcept>

namespace java::serialization {

decoder_ctx::decoder_ctx(const module& m)
: module_(m)
{}

decoder_ctx::~decoder_ctx() noexcept = default;

auto decoder_ctx::decoder([[maybe_unused]] ::std::nullptr_t np)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  class null_decoder final
  : public ::java::serialization::decoder
  {
   public:
    using ::java::serialization::decoder::decoder;
    ~null_decoder() noexcept override = default;

   private:
    auto init() -> ::java::lang::Object override {
      return nullptr;
    }

    auto init_comparable() -> dependent_set override {
      return dependent_set();
    }

    auto complete() -> dependent_set override {
      return dependent_set();
    }
  };

  return cycle_ptr::make_cycle<null_decoder>(*this);
};

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::new_object> obj)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (obj == nullptr) return decoder(nullptr);

  if (!obj->cls) throw decoding_error("encoded object has no class");

  // New object is an object class.
  auto cd = dynamic_cast<const stream::new_class_desc__class_desc*>(obj->cls.get());
  if (cd != nullptr) {
    const stream::field_descriptor& class_name = cd->class_name;

    if (class_name.is_primitive())
      throw decoding_error("encoded object declares itself as primitive (primitives are not objects)");
    if (class_name.is_array())
      throw decoding_error("encoded object declares itself an array (arrays have dedicated encoding)");

    return module_.decoder(::std::get<::std::u16string_view>(class_name.type()), *this, std::move(obj));
  }

  // New object is a proxy.
  auto pd = dynamic_cast<const stream::new_class_desc__proxy_class_desc*>(obj->cls.get());
  if (pd != nullptr) {
    // XXX: at some point we ought to support this...
    throw decoding_error("unsupported: encoded object is a proxy type");
  }

  // Neither cast worked.
  throw decoding_error("encoded object class not recognized");
}

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::stream_element> elem)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (elem == nullptr) return decoder(nullptr);

  {
    auto obj = ::std::dynamic_pointer_cast<const stream::new_object>(elem);
    if (obj != nullptr) return decoder(std::move(obj));
  }

#if 0 // XXX
  {
    auto str = ::std::dynamic_pointer_cast<const stream::string_stream>(elem);
    if (str != nullptr) return decoder(std::move(str));
  }
#endif

  throw decoding_error("unrecognized stream element type");
}


decoder::decoder(decoder_ctx& ctx)
: ctx(ctx)
{}

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


class_decoder_intf::class_decoder_intf(decoder_ctx& ctx, ::cycle_ptr::cycle_gptr<const stream::new_object> data)
: decoder(ctx),
  field_decoders_(field_decoder_set::allocator_type(*this)),
  data(std::move(data))
{
  if (data == nullptr) throw ::std::invalid_argument("null data");
}

class_decoder_intf::class_decoder_intf(decoder_ctx& ctx, ::cycle_ptr::cycle_gptr<const stream::stream_element> data)
: class_decoder_intf(ctx, cast_data_(std::move(data)))
{}

class_decoder_intf::~class_decoder_intf() noexcept = default;

auto class_decoder_intf::initial_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
-> ::java::lang::Object {
  ::cycle_ptr::cycle_gptr<decoder> d = ctx.decoder(obj);
  ::java::lang::Object d_obj = d->get_initial();
  field_decoders_.emplace(std::move(d));
  return d_obj;
}

auto class_decoder_intf::comparable_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
-> ::java::lang::Object {
  ::cycle_ptr::cycle_gptr<decoder> d = ctx.decoder(obj);
  ::java::lang::Object d_obj = d->get_comparable();
  field_decoders_.emplace(std::move(d));
  return d_obj;
}

auto class_decoder_intf::complete_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
-> ::java::lang::Object {
  ::cycle_ptr::cycle_gptr<decoder> d = ctx.decoder(obj);
  ::java::lang::Object d_obj = d->get_complete();
  field_decoders_.emplace(std::move(d));
  return d_obj;
}

auto class_decoder_intf::init_comparable()
-> dependent_set {
  return dependent_set(field_decoders_.begin(), field_decoders_.end());
}

auto class_decoder_intf::complete()
-> dependent_set {
  return dependent_set(field_decoders_.begin(), field_decoders_.end());
}

auto class_decoder_intf::cast_data_(::cycle_ptr::cycle_gptr<const stream::stream_element> data)
-> ::cycle_ptr::cycle_gptr<const stream::new_object> {
  ::cycle_ptr::cycle_gptr<const stream::new_object> casted_data =
      ::std::dynamic_pointer_cast<const stream::new_object>(data);
  if (casted_data == nullptr && data != nullptr)
    throw decoding_error("expected object description for object class");
  return casted_data;
}

} /* namespace java::serialization */
