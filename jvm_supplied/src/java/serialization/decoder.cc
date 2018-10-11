#include <java/serialization/decoder.h>
#include <java/serialization/exception.h>
#include <java/serialization/module.h>
#include <cassert>
#include <utility>
#include <stdexcept>
#include <java/fwd/java/lang/String.h>
#include <java/fwd/java/lang/Class.h>

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

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::stream_string> str)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (str == nullptr) return decoder(nullptr);
  return module_.decoder(::java::_tags::java::lang::String::u_name(), *this, std::move(str));
}

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::new_enum> enm)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (enm == nullptr) return decoder(nullptr);

  if (!enm->type) throw decoding_error("encoded object has no class");

  // New object is an object class.
  auto cd = dynamic_cast<const stream::new_class_desc__class_desc*>(enm->type.get());
  if (cd == nullptr) throw decoding_error("encoded enum class is not a plain class");
  const stream::field_descriptor& class_name = cd->class_name;

  if (class_name.is_primitive())
    throw decoding_error("encoded enum declares itself as primitive (primitives are not objects)");
  if (class_name.is_array())
    throw decoding_error("encoded enum declares itself an array (arrays have dedicated encoding)");

  return module_.decoder(::std::get<::std::u16string_view>(class_name.type()), *this, std::move(enm));
}

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::new_class> cls)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (cls == nullptr) return decoder(nullptr);
  return module_.decoder(::java::_tags::java::lang::Class::u_name(), *this, std::move(cls));
}

auto decoder_ctx::decoder(::cycle_ptr::cycle_gptr<const stream::stream_element> elem)
-> ::cycle_ptr::cycle_gptr<::java::serialization::decoder> {
  if (elem == nullptr) return decoder(nullptr);

  {
    auto obj = ::std::dynamic_pointer_cast<const stream::new_object>(elem);
    if (obj != nullptr) return decoder(std::move(obj));
  }

  {
    auto str = ::std::dynamic_pointer_cast<const stream::stream_string>(elem);
    if (str != nullptr) return decoder(std::move(str));
  }

  {
    auto enm = ::std::dynamic_pointer_cast<const stream::new_enum>(elem);
    if (enm != nullptr) return decoder(std::move(enm));
  }

  {
    auto cls = ::std::dynamic_pointer_cast<const stream::new_class>(elem);
    if (cls != nullptr) return decoder(std::move(cls));
  }

#if 0 // XXX
  {
    auto arr = ::std::dynamic_pointer_cast<const stream::new_array>(elem);
    if (arr != nullptr) return decoder(std::move(arr));
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


basic_class_decoder::basic_class_decoder(class_decoder_intf& intf, cycle_ptr::cycle_gptr<const stream::class_desc> cls, std::u16string_view name)
: intf_(intf),
  cls_(std::move(cls)),
  cd_(find_cd_(this->intf_, this->cls_))
{
  if (cls_ == nullptr)
    throw decoding_error("null class description while decoding object");

  const stream::new_class_desc__class_desc*const cd =
      dynamic_cast<const stream::new_class_desc__class_desc*>(cls_.get());

  if (cd == nullptr)
    throw decoding_error("class description is not a normal class description");
  if (cd->class_name.is_array())
    throw decoding_error("class description for object is an array");
  if (cd->class_name.is_primitive())
    throw decoding_error("class description for object is a primitive type");
  if (::std::get<std::u16string_view>(cd->class_name.type()) != name)
    throw decoding_error("class description for object is of unexpected type");
}

basic_class_decoder::~basic_class_decoder() noexcept = default;

auto basic_class_decoder::find_cd_(const class_decoder_intf& intf, const cycle_ptr::cycle_gptr<const stream::class_desc>& cls)
-> const stream::new_object::class_data& {
  const auto cdata_iter = intf.data->data.find(cls);
  if (cdata_iter == intf.data->data.end()) throw decoding_error("missing class data");
  return cdata_iter->second;
}

auto basic_class_decoder::get_field_(::std::u16string_view name) const
-> const stream::new_object::class_data::field_value* {
  const auto field_iter = cd_.fields.find(name);
  if (field_iter == cd_.fields.end()) return nullptr;
  return &field_iter->second;
}

auto basic_class_decoder::get_initial_field_(::std::u16string_view name) const
-> ::java::lang::Object {
  using obj_type = cycle_ptr::cycle_member_ptr<const stream::stream_element>;

  const stream::new_object::class_data::field_value*const field = get_field_(name);
  if (field != nullptr && !::std::holds_alternative<obj_type>(*field))
    throw decoding_error("field has incorrect type");

  return intf_.initial_field(field ? ::std::get<obj_type>(*field) : nullptr);
}

auto basic_class_decoder::get_comparable_field_(::std::u16string_view name) const
-> ::java::lang::Object {
  using obj_type = cycle_ptr::cycle_member_ptr<const stream::stream_element>;

  const stream::new_object::class_data::field_value*const field = get_field_(name);
  if (field != nullptr && !::std::holds_alternative<obj_type>(*field))
    throw decoding_error("field has incorrect type");

  return intf_.comparable_field(field ? ::std::get<obj_type>(*field) : nullptr);
}

auto basic_class_decoder::get_complete_field_(::std::u16string_view name) const
-> ::java::lang::Object {
  using obj_type = cycle_ptr::cycle_member_ptr<const stream::stream_element>;

  const stream::new_object::class_data::field_value*const field = get_field_(name);
  if (field != nullptr && !::std::holds_alternative<obj_type>(*field))
    throw decoding_error("field has incorrect type");

  return intf_.complete_field(field ? ::std::get<obj_type>(*field) : nullptr);
}

template auto basic_class_decoder::get_primitive_field<::java::boolean_t>(::std::u16string_view name, std::optional<::java::boolean_t> fb) const -> ::java::boolean_t;
template auto basic_class_decoder::get_primitive_field<::java::byte_t>(::std::u16string_view name, std::optional<::java::byte_t> fb) const -> ::java::byte_t;
template auto basic_class_decoder::get_primitive_field<::java::short_t>(::std::u16string_view name, std::optional<::java::short_t> fb) const -> ::java::short_t;
template auto basic_class_decoder::get_primitive_field<::java::int_t>(::std::u16string_view name, std::optional<::java::int_t> fb) const -> ::java::int_t;
template auto basic_class_decoder::get_primitive_field<::java::long_t>(::std::u16string_view name, std::optional<::java::long_t> fb) const -> ::java::long_t;
template auto basic_class_decoder::get_primitive_field<::java::float_t>(::std::u16string_view name, std::optional<::java::float_t> fb) const -> ::java::float_t;
template auto basic_class_decoder::get_primitive_field<::java::double_t>(::std::u16string_view name, std::optional<::java::double_t> fb) const -> ::java::double_t;
template auto basic_class_decoder::get_primitive_field<::java::char_t>(::std::u16string_view name, std::optional<::java::char_t> fb) const -> ::java::char_t;


} /* namespace java::serialization */
