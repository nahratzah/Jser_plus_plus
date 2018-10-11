#ifndef JAVA_SERIALIZATION_DECODER_H
#define JAVA_SERIALIZATION_DECODER_H

#include <java/serialization/decoder_fwd.h>
#include <java/serialization/module_fwd.h>
#include <java/serialization/encdec.h>
#include <java/inline.h>
#include <java/ref.h>
#include <java/primitives.h>
#include <java/lang/Object.h>
#include <cycle_ptr/cycle_ptr.h>
#include <cycle_ptr/allocator.h>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <variant>

namespace java::serialization {

class decoder_ctx {
 private:
  using decoder_map = std::unordered_map<::cycle_ptr::cycle_gptr<stream::stream_element>, ::cycle_ptr::cycle_gptr<::java::serialization::decoder>>;

 public:
  decoder_ctx(const module& m);
  ~decoder_ctx() noexcept;

  auto decoder(::std::nullptr_t np)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto decoder(::cycle_ptr::cycle_gptr<const stream::new_object> obj)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto decoder(::cycle_ptr::cycle_gptr<const stream::stream_string> obj)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto decoder(::cycle_ptr::cycle_gptr<const stream::new_enum> enm)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto decoder(::cycle_ptr::cycle_gptr<const stream::new_class> cls)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

#if 0
  auto decoder(::cycle_ptr::cycle_gptr<const stream::new_array> obj)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;
#endif

  auto decoder(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto get_module() const noexcept -> const module& {
    return module_;
  }

 private:
  decoder_map decoder_map_;
  const module& module_;
};

class decoder
: public ::cycle_ptr::cycle_base
{
 private:
  enum stage {
    Unstarted,
    Initial,
    Comparable,
    Complete
  };

 public:
  /**
   * \brief Set with dependent decoders.
   * \details Returned by functions that perform various initialization levels,
   * indicating that their initialization level depends on these fields also
   * completing.
   */
  using dependent_set = ::std::unordered_set<::cycle_ptr::cycle_gptr<decoder>>;

  explicit decoder(decoder_ctx& ctx);
  virtual ~decoder() noexcept = 0;

  auto get_initial() -> java::lang::Object;

  auto get_comparable() -> java::lang::Object;

  auto get_complete() -> java::lang::Object;

 private:
  static auto ensure_comparable(::cycle_ptr::cycle_gptr<decoder> initial) -> void;
  static auto ensure_complete(::cycle_ptr::cycle_gptr<decoder> initial) -> void;

  /**
   * \brief Construct initial object.
   * \details Initializes as little as possible,
   * restricting to only const-fields and primitives.
   * (Primitives because they're easy.)
   *
   * This method is called exactly once, to transition from
   * the Unstarted state to the Initial state.
   * It provides enough to initialize a field, but insufficient
   * use the field.
   * \returns Object that is being constructed by this decoder.
   * If a null-reference is returned, the stage will immediately progress
   * to Complete.
   */
  virtual auto init() -> ::java::lang::Object = 0;

  /**
   * \brief Further initialize the object to the Comparable state.
   * \details In the comparable state, enough fields are filled in to
   * enable hash code computation, and comparisons.
   *
   * This means all fields involved in these actions, shall also be initialized
   * to at least the Comparable state.
   *
   * This method is called exactly once, to transition from
   * the Initial state to the Comparable state.
   * Any call to this function will always be preceded by a call to init().
   *
   * \returns Set of decoders that also must meet the Comparable state,
   * for this object to be comparable.
   */
  virtual auto init_comparable() -> dependent_set = 0;

  /**
   * \brief Complete the construction of this object.
   *
   * \returns Set of decoders that must also meet the Complete state,
   * for this object to be completed.
   */
  virtual auto complete() -> dependent_set = 0;

  /**
   * \brief The object being constructed.
   */
  ::java::field_t<::java::lang::Object> object_;

  /**
   * \brief Indicator for state change being in progress.
   * \details
   * If we need to perform state transition while this is "locked"
   * (meaning, it's set to `true`), we have a recursion problem.
   * In that case, we throw a decoding_error.
   *
   * \note We don't bother "unlocking" if an error occurs, as the complete
   * decoding operation will be discarded anyway.
   */
  bool in_progress_ = false;

  /**
   * \brief Indicator of what stage we're in.
   * \details
   * The stage is tracked, to ensure we keep our promise of only invoking each
   * transition method once.
   */
  stage stage_ = Unstarted;

 protected:
  /**
   * \brief Decoder context.
   * \details
   * Holds on to all decoders and allows for lookups.
   */
  decoder_ctx& ctx;
};

class class_decoder_intf
: public decoder
{
 private:
  using field_decoder_set = ::std::unordered_set<
      ::cycle_ptr::cycle_member_ptr<decoder>,
      ::std::hash<::cycle_ptr::cycle_member_ptr<decoder>>,
      ::std::equal_to<::cycle_ptr::cycle_member_ptr<decoder>>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<decoder>>>>;

 public:
  class_decoder_intf(decoder_ctx& ctx, ::cycle_ptr::cycle_gptr<const stream::new_object> data);
  class_decoder_intf(decoder_ctx& ctx, ::cycle_ptr::cycle_gptr<const stream::stream_element> data);
  ~class_decoder_intf() noexcept override = 0;

  auto initial_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
  -> ::java::lang::Object;
  auto comparable_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
  -> ::java::lang::Object;
  auto complete_field(::cycle_ptr::cycle_gptr<const stream::stream_element> obj)
  -> ::java::lang::Object;

  auto init_comparable() -> dependent_set override;
  auto complete() -> dependent_set override;

 private:
  static auto cast_data_(::cycle_ptr::cycle_gptr<const stream::stream_element> data)
  -> ::cycle_ptr::cycle_gptr<const stream::new_object>;

  field_decoder_set field_decoders_;

 public:
  ///\brief Object description. Never null.
  const ::cycle_ptr::cycle_gptr<const stream::new_object> data;
};

class basic_class_decoder {
  template<typename> friend class class_decoder;

 protected:
  basic_class_decoder(class_decoder_intf& intf, cycle_ptr::cycle_gptr<const stream::class_desc> cls, std::u16string_view name);
  ~basic_class_decoder() noexcept;

 public:
  template<typename T>
  auto get_primitive_field(::std::u16string_view name, std::optional<T> fb = ::std::optional<T>()) const
  -> T {
    const stream::new_object::class_data::field_value*const field = get_field_(name);
    if (!field) {
      if (fb) return *fb;
      throw decoding_error("missing field data");
    }

    if (!::std::holds_alternative<T>(*field)) throw decoding_error("field has incorrect type");
    return ::std::get<T>(*field);
  }

  template<typename T>
  JSER_INLINE auto get_initial_field(::std::u16string_view name) const
  -> T {
    return ::java::cast<T>(get_initial_field_(name));
  }

  template<typename T>
  JSER_INLINE auto get_comparable_field(::std::u16string_view name) const
  -> T {
    return ::java::cast<T>(get_comparable_field_(name));
  }

  template<typename T>
  JSER_INLINE auto get_complete_field(::std::u16string_view name) const
  -> T {
    return ::java::cast<T>(get_complete_field_(name));
  }

 private:
  static auto find_cd_(const class_decoder_intf& intf, const cycle_ptr::cycle_gptr<const stream::class_desc>& cls)
  -> const stream::new_object::class_data&;

  auto get_field_(::std::u16string_view name) const
  -> const stream::new_object::class_data::field_value*;

  auto get_initial_field_(::std::u16string_view name) const
  -> ::java::lang::Object;

  auto get_comparable_field_(::std::u16string_view name) const
  -> ::java::lang::Object;

  auto get_complete_field_(::std::u16string_view name) const
  -> ::java::lang::Object;

  ///\brief Class decoder interface.
  class_decoder_intf& intf_;
  ///\brief Class description. Never null.
  const cycle_ptr::cycle_gptr<const stream::class_desc> cls_;
  ///\brief Class data.
  const stream::new_object::class_data& cd_;
};


template<typename Tag>
class class_decoder
: public basic_class_decoder
{
 public:
  class_decoder(class_decoder_intf& intf)
  : basic_class_decoder(intf, intf.data->cls, Tag::u_name())
  {}

  template<typename OtherTag>
  class_decoder(const class_decoder<OtherTag>& other)
  : basic_class_decoder(other.intf_, other.cls_->get_super(), Tag::u_name())
  {}
};


extern template auto basic_class_decoder::get_primitive_field<::java::boolean_t>(::std::u16string_view name, std::optional<::java::boolean_t> fb) const -> ::java::boolean_t;
extern template auto basic_class_decoder::get_primitive_field<::java::byte_t>(::std::u16string_view name, std::optional<::java::byte_t> fb) const -> ::java::byte_t;
extern template auto basic_class_decoder::get_primitive_field<::java::short_t>(::std::u16string_view name, std::optional<::java::short_t> fb) const -> ::java::short_t;
extern template auto basic_class_decoder::get_primitive_field<::java::int_t>(::std::u16string_view name, std::optional<::java::int_t> fb) const -> ::java::int_t;
extern template auto basic_class_decoder::get_primitive_field<::java::long_t>(::std::u16string_view name, std::optional<::java::long_t> fb) const -> ::java::long_t;
extern template auto basic_class_decoder::get_primitive_field<::java::float_t>(::std::u16string_view name, std::optional<::java::float_t> fb) const -> ::java::float_t;
extern template auto basic_class_decoder::get_primitive_field<::java::double_t>(::std::u16string_view name, std::optional<::java::double_t> fb) const -> ::java::double_t;
extern template auto basic_class_decoder::get_primitive_field<::java::char_t>(::std::u16string_view name, std::optional<::java::char_t> fb) const -> ::java::char_t;

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_DECODER_H */
