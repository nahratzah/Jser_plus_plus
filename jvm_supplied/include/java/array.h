#ifndef JAVA_ARRAY_H
#define JAVA_ARRAY_H

#include <java/array_fwd.h>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <java/primitives.h>
#include <java/object_intf.h>
#include <java/type_traits.h>
#include <java/_accessor.h>
#include <java/inline.h>
#include <java/ref.h>
#include <java/reflect_fwd.h>
#include <cycle_ptr/allocator.h>
#include <cycle_ptr/cycle_ptr.h>
#include <java/io/Serializable.h>
#include <java/lang/Cloneable.h>
#include <java/lang/Object.h>
#include <java/fwd/java/lang/Class.h>

namespace java::_erased::java {

template<typename, std::size_t> struct cast_check_generic_;

/**
 * \brief Erased base type of all the arrays.
 * \details Handles abstracting the array logic.
 */
class array_intf
: public ::java::_erased::java::lang::Object,
  public virtual ::java::_erased::java::io::Serializable,
  public virtual ::java::_erased::java::lang::Cloneable
{
  template<typename T, std::size_t Dim> friend struct ::java::_erased::java::cast_check_generic_;

 public:
  virtual ~array_intf() noexcept override = 0;

  JSER_INLINE auto empty() const noexcept -> bool {
    return empty_();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return size_();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dimensions_();
  }

  auto begin() const
  -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>>;
  auto end() const
  -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>>;
  auto begin()
  -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>>;
  auto end()
  -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>>;

  virtual auto reserve(::std::size_t sz) -> void = 0;

  auto push_back(::java::lang::Object obj) -> void;

  auto clone() -> ::cycle_ptr::cycle_gptr<array_intf>;

 protected:
  auto check_(::java::lang::Object elem) const -> ::java::lang::Object;

 private:
  virtual auto empty_() const noexcept -> bool = 0;
  virtual auto size_() const noexcept -> std::size_t = 0;
  virtual auto dimensions_() const noexcept -> std::size_t = 0;

  virtual auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> = 0;

  virtual auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> = 0;

  virtual auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> = 0;
  virtual auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> = 0;
  virtual auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> = 0;
  virtual auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> = 0;
  virtual auto push_back_(::java::lang::Object obj) -> void = 0;

  auto _virtual_clone([[maybe_unused]] ::java::_tags::java::lang::Cloneable _tag_)
  -> ::java::return_t<::java::lang::Cloneable> override final;

  virtual auto clone_()
  -> ::cycle_ptr::cycle_gptr<array_intf> = 0;
};

/**
 * \brief Type-erased array type.
 * \details
 * Erased array type is only defined for the primitive types (excluding void),
 * and objects.
 */
template<typename T>
class array;
/**
 * \brief Type-erased array for multi-dimensional arrays.
 * \details
 * Erased array type is only defined for the primitive types (excluding void),
 * and objects.
 */
template<typename T>
class array_of_array;

template<>
class array<::java::boolean_t> final
: public array_intf
{
  friend array_of_array<::java::boolean_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::boolean_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::boolean_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::boolean_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::byte_t> final
: public array_intf
{
  friend array_of_array<::java::byte_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::byte_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::byte_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::byte_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::short_t> final
: public array_intf
{
  friend array_of_array<::java::short_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::short_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::short_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::short_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::int_t> final
: public array_intf
{
  friend array_of_array<::java::int_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::int_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::int_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::int_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::long_t> final
: public array_intf
{
  friend array_of_array<::java::long_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::long_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::long_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::long_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::float_t> final
: public array_intf
{
  friend array_of_array<::java::float_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::float_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::float_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::float_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::double_t> final
: public array_intf
{
  friend array_of_array<::java::double_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::double_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::double_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::double_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::char_t> final
: public array_intf
{
  friend array_of_array<::java::char_t>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = ::std::vector<::java::char_t>;

 public:
  array() = default;

  explicit array(::std::initializer_list<::java::char_t> init)
  : data_(std::move(init))
  {}

  template<typename Iter, typename = std::enable_if_t<std::is_constructible_v<vector_type, Iter, Iter>>>
  explicit array(Iter b, Iter e)
  : data_(std::move(b), std::move(e))
  {}

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  JSER_INLINE auto begin() noexcept -> typename vector_type::iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() noexcept -> typename vector_type::iterator {
    return data_.end();
  }
  JSER_INLINE auto begin() const noexcept -> typename vector_type::const_iterator {
    return data_.begin();
  }
  JSER_INLINE auto end() const noexcept -> typename vector_type::const_iterator {
    return data_.end();
  }

  auto reserve(::std::size_t sz) -> void override;

  using array_intf::push_back;

  JSER_INLINE auto push_back(::java::char_t b) -> void {
    data_.push_back(b);
  }

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  vector_type data_;
};

template<>
class array<::java::lang::Object> final
: public array_intf
{
  friend array_of_array<::java::lang::Object>;
  friend ::java::_reflect_ops;
  friend ::java::_erased::java::lang::Class;

 private:
  using vector_type = std::vector<
      ::java::field_t<::java::lang::Object>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::java::field_t<::java::lang::Object>>>>;

 public:
  array() = delete;
  explicit array(::java::lang::Class<::java::G::pack<>> element_type);

  template<typename T>
  explicit array(::java::lang::Class<::java::G::pack<>> element_type, ::std::initializer_list<T> init)
  : element_type_(std::move(element_type)),
    data_(::std::make_move_iterator(init.begin()), ::std::make_move_iterator(init.end()), vector_type::allocator_type(*this))
  {
    if (!element_type_)
      throw ::std::invalid_argument("null class for array");
  }

  ~array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

  auto reserve(::std::size_t sz) -> void override;

  auto clone() -> ::cycle_ptr::cycle_gptr<array>;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  static auto __class__(::java::lang::Class<::java::G::pack<>> element_type, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class>;

  static auto __serializable_array_class__(::java::serialization::cycle_handler& handler, ::java::lang::Class<::java::G::pack<>> element_type, std::size_t dimensions)
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc>;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array& x, const array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::java::field_t<::java::lang::Class<::java::G::pack<>>> element_type_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::boolean_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::byte_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::short_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::int_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::long_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::float_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::double_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::char_t> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  explicit array_of_array(std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<>
class array_of_array<::java::lang::Object> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::cycle_ptr::cycle_member_ptr<array_intf>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::cycle_ptr::cycle_member_ptr<array_intf>>>>;

 public:
  array_of_array() = delete;
  array_of_array(::java::lang::Class<::java::G::pack<>> element_type, std::size_t dim);
  ~array_of_array() noexcept override;

  JSER_INLINE auto empty() const noexcept -> bool {
    return data_.empty();
  }

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dim_;
  }

  auto reserve(::std::size_t sz) -> void override;

 private:
  auto empty_() const noexcept -> bool override;
  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  virtual auto __get_class__() const
  -> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> override;

  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto leaf_element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto __encoder__(::java::serialization::cycle_handler& handler_) const
  -> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> override;

  auto __hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
  -> ::std::size_t override;

  static auto __equal_impl__(bool specialized, ::java::_equal_helper& eq,
                             const array_of_array& x, const array_of_array& y)
  -> void;

  auto __equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
  -> void override;

  auto begin_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto end_() const -> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> override;
  auto begin_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto end_() -> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> override;
  auto push_back_(::java::lang::Object obj) -> void override;
  auto clone_() -> ::cycle_ptr::cycle_gptr<array_intf> override;

  const ::java::field_t<::java::lang::Class<::java::G::pack<>>> element_type_;
  const ::std::size_t dim_;
  vector_type data_{ vector_type::allocator_type(*this) };
};

template<typename Type>
struct select_array_elemtype_
: ::std::enable_if<
    ::java::type_traits::is_java_primitive_v<Type>,
    Type>
{};

template<typename Tag, typename... Types>
struct select_array_elemtype_<::java::G::is_t<Tag, Types...>> {
  using type = ::java::lang::Object;
};

template<typename Type>
struct select_array_elemtype_<Type*>
: select_array_elemtype_<Type>
{};

template<typename Type>
struct select_array_dim_
: ::std::integral_constant<::std::size_t, 0u>
{};

template<typename Type>
struct select_array_dim_<Type*>
: ::std::integral_constant<::std::size_t, 1u + select_array_dim_<Type>::value>
{};

template<typename Type>
struct select_array_new_impl_
: ::std::conditional<
    (select_array_dim_<Type>::value >= 2u),
    array_of_array<typename select_array_elemtype_<Type>::type>,
    array<typename select_array_elemtype_<Type>::type>>
{};

template<typename Type>
struct array_impl_is_primitive_
: ::std::integral_constant<bool, ::java::type_traits::is_java_primitive_v<Type>>
{};

template<typename Type>
struct array_impl_is_primitive_<Type*>
: array_impl_is_primitive_<Type>
{};

template<typename Type>
struct get_array_elem_class_ {
  static JSER_INLINE auto get_class() -> decltype(auto) {
    return ::java::get_class<Type>();
  }
};

template<typename Type>
struct get_array_elem_class_<Type*>
: get_array_elem_class_<Type>
{};

template<typename Type, typename ArrayType>
JSER_INLINE auto cast_check_(const ArrayType& a)
-> std::enable_if_t<::std::is_base_of_v<array_intf, ArrayType>, bool>;

template<
    typename Type,
    bool IsPrimitive = array_impl_is_primitive_<Type>::value,
    std::size_t Dim = select_array_dim_<Type>::value>
struct array_factory_ {
  using array_impl = typename select_array_new_impl_<Type>::type;
  using initializer_list = ::std::initializer_list<::java::type<::std::remove_pointer_t<Type>>>;

  auto operator()(initializer_list list) const
  -> ::cycle_ptr::cycle_gptr<array_impl> {
    if constexpr(IsPrimitive) {
      static_assert(Dim > 1u);
      return ::cycle_ptr::make_cycle<array_impl>(Dim, list);
    } else if constexpr(Dim == 1u) {
      static_assert(!IsPrimitive);
      return ::cycle_ptr::make_cycle<array_impl>(
          ::java::get_class<typename select_array_elemtype_<Type>::type>(),
          list);
    } else {
      static_assert(!IsPrimitive);
      return ::cycle_ptr::make_cycle<array_impl>(
          ::java::get_class<typename select_array_elemtype_<Type>::type>(),
          Dim,
          list);
    }
  }
};

template<
    typename Type>
struct array_factory_<Type, true, 1> {
  using array_impl = typename select_array_new_impl_<Type>::type;
  using initializer_list = ::std::initializer_list<::std::remove_pointer_t<Type>>;

  auto operator()(initializer_list list) const
  -> ::cycle_ptr::cycle_gptr<array_impl> {
    return ::cycle_ptr::make_cycle<array_impl>(list);
  }
};

} /* namespace java::_erased::java */

namespace java {

template<template<class> class PtrImpl, typename Type>
class basic_ref<PtrImpl, Type*> final {
  // Be friends with all our specializations.
  template<template<class> class, typename> friend class java::basic_ref;

  // Be friend with raw_objintf function.
  template<template<class> class FnPtrImpl, typename FnType>
  friend auto raw_objintf(const basic_ref<FnPtrImpl, FnType>& r) noexcept
  -> std::conditional_t<
      std::is_const_v<FnType>,
      const object_intf*,
      object_intf*>;
  template<template<class> class FnPtrImpl, typename FnType>
  friend auto raw_objintf_ptr(const basic_ref<FnPtrImpl, FnType>& r) noexcept
  -> std::conditional_t<
      std::is_const_v<FnType>,
      ::cycle_ptr::cycle_gptr<const object_intf>,
      ::cycle_ptr::cycle_gptr<object_intf>>;

 protected:
  using ptr_type = PtrImpl<std::conditional_t<
      ::java::type_traits::is_java_primitive_v<Type>,
      ::java::_erased::java::array<Type>,
      ::java::_erased::java::array_intf>>;

 public:
  using size_type = std::size_t;

  JSER_INLINE basic_ref() = default;
  JSER_INLINE basic_ref(const basic_ref&) = default;
  JSER_INLINE basic_ref(basic_ref&&) = default;
  JSER_INLINE basic_ref& operator=(const basic_ref&) = default;
  JSER_INLINE basic_ref& operator=(basic_ref&&) = default;
  JSER_INLINE ~basic_ref() noexcept = default;

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner)
  : p_(owner)
  {}

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, const basic_ref& other)
  : p_(owner, other.p_)
  {}

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, basic_ref&& other)
  : p_(owner, ::std::move(other.p_))
  {}

  template<
      typename NewArraySelector = typename ::java::_erased::java::select_array_new_impl_<Type*>,
      typename NewArrayType = typename NewArraySelector::type>
  explicit JSER_INLINE basic_ref([[maybe_unused]] allocate_t a) {
    if constexpr(::java::_erased::java::array_impl_is_primitive_<Type*>::value)
      p_ = ::cycle_ptr::make_cycle<NewArrayType>();
    else
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::java::_erased::java::get_array_elem_class_<Type*>::get_class());
  }

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      typename NewArraySelector = typename ::java::_erased::java::select_array_new_impl_<Type*>,
      typename NewArrayType = typename NewArraySelector::type>
  explicit JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, [[maybe_unused]] allocate_t a)
  : p_(owner)
  {
    if constexpr(::java::_erased::java::array_impl_is_primitive_<Type*>::value)
      p_ = ::cycle_ptr::make_cycle<NewArrayType>();
    else
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::java::_erased::java::get_array_elem_class_<Type*>::get_class());
  }

  template<
      typename... Args,
      typename NewArraySelector = typename ::java::_erased::java::select_array_new_impl_<Type*>,
      typename NewArrayType = typename NewArraySelector::type,
      typename = std::enable_if_t<
          ( ::java::_erased::java::array_impl_is_primitive_<Type*>::value
          ? ::std::is_constructible_v<NewArrayType, Args...>
          : ::std::is_constructible_v<NewArrayType, ::java::lang::Class<::java::G::pack<>>, Args...>
          )>>
  explicit JSER_INLINE basic_ref([[maybe_unused]] allocate_t a, Args&&... args) {
    if constexpr(::java::_erased::java::array_impl_is_primitive_<Type*>::value)
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::std::forward<Args>(args)...);
    else
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::java::_erased::java::get_array_elem_class_<Type*>::get_class(), ::std::forward<Args>(args)...);
  }

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      typename... Args,
      typename NewArraySelector = typename ::java::_erased::java::select_array_new_impl_<Type*>,
      typename NewArrayType = typename NewArraySelector::type,
      typename = std::enable_if_t<
          ( ::java::_erased::java::array_impl_is_primitive_<Type*>::value
          ? ::std::is_constructible_v<NewArrayType, Args...>
          : ::std::is_constructible_v<NewArrayType, ::java::lang::Class<::java::G::pack<>>, Args...>
          )>>
  explicit JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, [[maybe_unused]] allocate_t a, Args&&... args)
  : p_(owner)
  {
    if constexpr(::java::_erased::java::array_impl_is_primitive_<Type*>::value)
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::std::forward<Args>(args)...);
    else
      p_ = ::cycle_ptr::make_cycle<NewArrayType>(::java::_erased::java::get_array_elem_class_<Type*>::get_class(), ::std::forward<Args>(args)...);
  }

  explicit JSER_INLINE basic_ref([[maybe_unused]] allocate_t a, typename ::java::_erased::java::array_factory_<Type*>::initializer_list init)
  : p_(::java::_erased::java::array_factory_<Type*>()(init))
  {}

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  explicit JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, [[maybe_unused]] allocate_t a, typename ::java::_erased::java::array_factory_<Type*>::initializer_list init)
  : p_(owner, ::java::_erased::java::array_factory_<Type*>()(init))
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>
          && !std::is_const_v<OType>>>
  JSER_INLINE basic_ref(const basic_ref<OPtrImpl, OType>& other)
  : p_(other.p_)
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>
          && !std::is_const_v<OType>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, const basic_ref<OPtrImpl, OType>& other)
  : p_(owner, other.p_)
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>
          && !std::is_const_v<OType>>>
  JSER_INLINE basic_ref(basic_ref<OPtrImpl, OType>&& other)
  : p_(std::move(other.p_))
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>
          && !std::is_const_v<OType>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, basic_ref<OPtrImpl, OType>&& other)
  : p_(owner, std::move(other.p_))
  {}

  template<template<typename> class XImpl, typename XType>
  JSER_INLINE basic_ref([[maybe_unused]] _cast c, const basic_ref<XImpl, XType>& x) {
    p_ = ::std::dynamic_pointer_cast<typename ptr_type::element_type>(x.p_);
    if (p_ == nullptr) {
      if (x.p_ != nullptr) throw std::bad_cast();
      return;
    }

    if (!::java::_erased::java::cast_check_<Type*>(*p_))
      throw std::bad_cast();
  }

  template<template<typename> class XImpl, typename XType>
  JSER_INLINE basic_ref(_cast c, basic_ref<XImpl, XType>&& x)
  : basic_ref(c, x)
  {
    x.p_ = nullptr;
  }

  auto operator=([[maybe_unused]] std::nullptr_t np)
  -> basic_ref& {
    p_ = nullptr;
    return *this;
  }

  JSER_INLINE explicit operator bool() const noexcept {
    return bool(p_);
  }

  JSER_INLINE auto operator!() const noexcept -> bool {
    return !p_;
  }

  JSER_INLINE auto operator==([[maybe_unused]] ::std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  JSER_INLINE auto operator!=([[maybe_unused]] ::std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

  JSER_INLINE auto empty() const -> bool {
    if (!*this) throw ::java::null_error();
    return p_->empty();
  }

  JSER_INLINE auto size() const -> size_type {
    if (!*this) throw ::java::null_error();
    return p_->size();
  }

  template<bool Enable = ::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto begin() const
  -> ::std::enable_if_t<Enable, typename std::vector<Type>::iterator> {
    if (!*this) throw ::java::null_error();
    return p_->begin();
  }

  template<bool Enable = ::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto end() const
  -> ::std::enable_if_t<Enable, typename std::vector<Type>::iterator> {
    if (!*this) throw ::java::null_error();
    return p_->end();
  }

  template<bool Enable = !::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto begin() const
  -> ::std::enable_if_t<Enable, ::java::bidirectional_iterator<Type>> {
    if (!*this) throw ::java::null_error();
    return ::java::cast<::java::bidirectional_iterator<Type>>(p_->begin());
  }

  template<bool Enable = !::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto end() const
  -> ::std::enable_if_t<Enable, ::java::bidirectional_iterator<Type>> {
    if (!*this) throw ::java::null_error();
    return ::java::cast<::java::bidirectional_iterator<Type>>(p_->end());
  }

  JSER_INLINE auto reserve(size_type sz) -> void {
    if (!*this) throw ::java::null_error();
    p_->reserve(sz);
  }

  JSER_INLINE auto push_back(::java::var_ref<Type> obj)
  -> void {
    if (!*this) throw ::java::null_error();
    p_->push_back(std::move(obj));
  }

 private:
  ptr_type p_ = nullptr;
};

template<template<class> class PtrImpl, typename Type>
class basic_ref<PtrImpl, Type*const> final {
  // Be friends with all our specializations.
  template<template<class> class, typename> friend class java::basic_ref;

  // Be friend with raw_objintf function.
  template<template<class> class FnPtrImpl, typename FnType>
  friend auto raw_objintf(const basic_ref<FnPtrImpl, FnType>& r) noexcept
  -> std::conditional_t<
      std::is_const_v<FnType>,
      const object_intf*,
      object_intf*>;
  template<template<class> class FnPtrImpl, typename FnType>
  friend auto raw_objintf_ptr(const basic_ref<FnPtrImpl, FnType>& r) noexcept
  -> std::conditional_t<
      std::is_const_v<FnType>,
      ::cycle_ptr::cycle_gptr<const object_intf>,
      ::cycle_ptr::cycle_gptr<object_intf>>;

 protected:
  using ptr_type = PtrImpl<std::add_const_t<std::conditional_t<
      ::java::type_traits::is_java_primitive_v<Type>,
      ::java::_erased::java::array<Type>,
      ::java::_erased::java::array_intf>>>;

 public:
  using size_type = std::size_t;

  JSER_INLINE basic_ref() = default;
  JSER_INLINE basic_ref(const basic_ref&) = default;
  JSER_INLINE basic_ref(basic_ref&&) = default;
  JSER_INLINE basic_ref& operator=(const basic_ref&) = default;
  JSER_INLINE basic_ref& operator=(basic_ref&&) = default;
  JSER_INLINE ~basic_ref() noexcept = default;

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner)
  : p_(owner)
  {}

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, const basic_ref& other)
  : p_(owner, other.p_)
  {}

  template<bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, basic_ref&& other)
  : p_(owner, ::std::move(other.p_))
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(const basic_ref<OPtrImpl, OType>& other)
  : p_(other.p_)
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, const basic_ref<OPtrImpl, OType>& other)
  : p_(owner, other.p_)
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(basic_ref<OPtrImpl, OType>&& other)
  : p_(std::move(other.p_))
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, basic_ref<OPtrImpl, OType>&& other)
  : p_(owner, std::move(other.p_))
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(const basic_ref<OPtrImpl, const OType>& other)
  : p_(other.p_)
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, const basic_ref<OPtrImpl, const OType>& other)
  : p_(owner, other.p_)
  {}

  template<template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(basic_ref<OPtrImpl, const OType>&& other)
  : p_(std::move(other.p_))
  {}

  template<
      bool Enable = ::std::is_constructible_v<ptr_type, ::cycle_ptr::cycle_base&>,
      template<typename> class OPtrImpl, typename OType,
      typename = std::enable_if_t<
          java::type_traits::is_assignable_v<Type*, std::remove_const_t<OType>>>>
  JSER_INLINE basic_ref(::std::enable_if_t<Enable, ::cycle_ptr::cycle_base&> owner, basic_ref<OPtrImpl, const OType>&& other)
  : p_(owner, std::move(other.p_))
  {}

  template<template<typename> class XImpl, typename XType>
  JSER_INLINE basic_ref([[maybe_unused]] _cast c, const basic_ref<XImpl, XType>& x) {
    p_ = ::std::dynamic_pointer_cast<typename ptr_type::element_type>(x.p_);
    if (p_ == nullptr) {
      if (x.p_ != nullptr) throw std::bad_cast();
      return;
    }

    if (!::java::_erased::java::cast_check_<Type*>(*p_))
      throw std::bad_cast();
  }

  template<template<typename> class XImpl, typename XType>
  JSER_INLINE basic_ref(_cast c, basic_ref<XImpl, XType>&& x)
  : basic_ref(c, x)
  {
    x.p_ = nullptr;
  }

  auto operator=([[maybe_unused]] std::nullptr_t np)
  -> basic_ref& {
    p_ = nullptr;
    return *this;
  }

  JSER_INLINE explicit operator bool() const noexcept {
    return bool(p_);
  }

  JSER_INLINE auto operator!() const noexcept -> bool {
    return !p_;
  }

  JSER_INLINE auto operator==([[maybe_unused]] ::std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  JSER_INLINE auto operator!=([[maybe_unused]] ::std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

  JSER_INLINE auto empty() const -> bool {
    if (!*this) throw ::java::null_error();
    return p_->empty();
  }

  JSER_INLINE auto size() const -> size_type {
    if (!*this) throw ::java::null_error();
    return p_->size();
  }

  template<bool Enable = ::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto begin() const
  -> ::std::enable_if_t<Enable, typename std::vector<Type>::const_iterator> {
    if (!*this) throw ::java::null_error();
    return p_->begin();
  }

  template<bool Enable = ::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto end() const
  -> ::std::enable_if_t<Enable, typename std::vector<Type>::const_iterator> {
    if (!*this) throw ::java::null_error();
    return p_->end();
  }

  template<bool Enable = !::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto begin() const
  -> ::std::enable_if_t<Enable, ::java::bidirectional_iterator<const Type>> {
    if (!*this) throw ::java::null_error();
    return ::java::cast<::java::bidirectional_iterator<const Type>>(p_->begin());
  }

  template<bool Enable = !::java::type_traits::is_java_primitive_v<Type>>
  JSER_INLINE auto end() const
  -> ::std::enable_if_t<Enable, ::java::bidirectional_iterator<const Type>> {
    if (!*this) throw ::java::null_error();
    return ::java::cast<::java::bidirectional_iterator<const Type>>(p_->end());
  }

 private:
  ptr_type p_ = nullptr;
};

} /* namespace java */

#include <java/reflect.h>
#include <java/lang/Class.h>

namespace java::_erased::java {


template<typename Type, typename ArrayType>
JSER_INLINE auto cast_check_(const ArrayType& a)
-> std::enable_if_t<::std::is_base_of_v<array_intf, ArrayType>, bool> {
  if constexpr(array_impl_is_primitive_<Type>::value) {
    return ::java::get_class<Type>()->isAssignableFrom(::java::_reflect_ops::get_class(a));
  } else {
    cast_check_generic_<Type, select_array_dim_<Type>::value> impl;
    return impl(a, a.dimensions());
  }
}


template<typename Tag, typename... Args, std::size_t Dim>
struct cast_check_generic_<::java::G::is_t<Tag, Args...>, Dim> {
  JSER_INLINE auto operator()(const array_intf& a, std::size_t dimensions) const
  -> bool {
    return dimensions == Dim
        && ::java::get_class<::java::G::is_t<Tag, Args...>>() == a.leaf_element_class_();
  }
};

template<typename Tag, typename... Args, std::size_t Dim>
struct cast_check_generic_<::java::G::extends_t<Tag, Args...>, Dim> {
  JSER_INLINE auto operator()(const array_intf& a, std::size_t dimensions) const
  -> bool {
    return dimensions == Dim
        && ::java::get_class<::java::G::is_t<Tag, Args...>>().isAssignableFrom(a.leaf_element_class_());
  }
};

template<typename Tag, typename... Args, std::size_t Dim>
struct cast_check_generic_<::java::G::super_t<Tag, Args...>, Dim>
: cast_check_generic_<::java::G::extends<::java::_tags::java::lang::Object>, Dim>
{};

template<typename... T, std::size_t Dim>
struct cast_check_generic_<::java::G::pack_t<T...>, Dim> {
  JSER_INLINE auto operator()(const array_intf& a, std::size_t dimensions) const
  -> bool {
    for (bool i : { cast_check_generic_<T, Dim>()(a, dimensions)... })
      if (!i) return false;
    return true;
  }
};

template<std::size_t Dim>
struct cast_check_generic_<::java::G::pack_t<>, Dim>
: cast_check_generic_<::java::G::extends<::java::_tags::java::lang::Object>, Dim>
{};

template<typename T, std::size_t Dim>
struct cast_check_generic_<T*, Dim>
: cast_check_generic_<T, Dim>
{};


} /* namespace java::_erased::java */

#endif /* JAVA_ARRAY_H */
