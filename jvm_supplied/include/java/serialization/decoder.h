#ifndef JAVA_SERIALIZATION_DECODER_H
#define JAVA_SERIALIZATION_DECODER_H

#include <java/serialization/decoder_fwd.h>
#include <java/serialization/module_fwd.h>
#include <java/serialization/encdec.h>
#include <java/ref.h>
#include <java/lang/Object.h>
#include <cycle_ptr/cycle_ptr.h>
#include <unordered_set>
#include <unordered_map>

namespace java::serialization {

class decoder_ctx {
 private:
  using decoder_map = std::unordered_map<::cycle_ptr::cycle_gptr<stream::stream_element>, ::cycle_ptr::cycle_gptr<::java::serialization::decoder>>;

 public:
  decoder_ctx(const module& m);
  ~decoder_ctx() noexcept;

  auto decoder(::std::nullptr_t np)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

  auto decoder(::cycle_ptr::cycle_gptr<stream::new_object> obj)
  -> ::cycle_ptr::cycle_gptr<::java::serialization::decoder>;

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

  decoder(decoder_ctx& ctx);
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

///\brief Validate that a class description is for the expected class name.
///\param cd A class description.
///\param name The expected name of the class description.
///\throws decoding_error If the class description is null, not for an object,
///or for an object with a different name.
auto streamelem_is_non_null_named_class(::cycle_ptr::cycle_gptr<const stream::class_desc> raw_cd, std::u16string_view name)
-> ::cycle_ptr::cycle_gptr<const stream::new_class_desc__class_desc> {
  if (raw_cd == nullptr)
    throw decoding_error("null class description while decoding object");

  ::cycle_ptr::cycle_gptr<const stream::new_class_desc__class_desc> cd =
      ::std::dynamic_pointer_cast<const stream::new_class_desc__class_desc>(raw_cd);

  if (cd == nullptr)
    throw decoding_error("class description is not a normal class description");
  if (cd->class_name.is_array())
    throw decoding_error("class description for object is an array");
  if (cd->class_name.is_primitive())
    throw decoding_error("class description for object is a primitive type");
  if (::std::get<std::u16string_view>(cd->class_name.type()) != name)
    throw decoding_error("class description for object is of unexpected type");

  return cd;
}

} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_DECODER_H */
