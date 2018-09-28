#ifndef JAVA_ARRAY_H
#define JAVA_ARRAY_H

#include <cstddef>
#include <vector>
#include <java/primitives.h>
#include <java/object_intf.h>
#include <java/type_traits.h>
#include <java/_accessor.h>
#include <java/inline.h>
#include <java/ref.h>
#include <cycle_ptr/allocator.h>
#include <cycle_ptr/cycle_ptr.h>
#include <java/io/Serializable.h>
#include <java/lang/Object.h>

namespace java::_erased::java {

/**
 * \brief Erased base type of all the arrays.
 * \details Handles abstracting the array logic.
 */
class array_intf
: public ::java::_erased::java::lang::Object,
  public virtual ::java::_erased::java::io::Serializable
{
 public:
  virtual ~array_intf() noexcept override = 0;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return size_();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return dimensions_();
  }

 private:
  auto _virtual___get_class__(::java::_tags::java::lang::Object _tag_) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override final;

  auto _virtual___get_class__(::java::_tags::java::io::Serializable _tag_) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override final;

  auto array_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>>;

  virtual auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> = 0;

  virtual auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>>;

  virtual auto size_() const noexcept -> std::size_t = 0;
  virtual auto dimensions_() const noexcept -> std::size_t = 0;
};

/**
 * \brief Type-erased array type.
 * \details
 * Erased array type is only defined for the primitive types (excluding void),
 * object_intf, and array types.
 */
template<typename T>
class array;

template<>
class array<::java::boolean_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::boolean_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::byte_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::byte_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::short_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::short_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::int_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::int_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::long_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::long_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::float_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::float_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::double_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::double_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::char_t> final
: public array_intf
{
 private:
  using vector_type = ::std::vector<::java::char_t>;

 public:
  array() = default;
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto array_class_(std::size_t dimensions) const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  vector_type data_;
};

template<>
class array<::java::lang::Object> final
: public array_intf
{
 private:
  using vector_type = std::vector<
      ::java::field_t<::java::lang::Object>,
      ::cycle_ptr::cycle_allocator<::std::allocator<::java::field_t<::java::lang::Object>>>>;

 public:
  array() = delete;
  explicit array(::java::lang::Class<::java::G::pack<>> element_type);
  ~array() noexcept override;

  JSER_INLINE auto size() const noexcept -> std::size_t {
    return data_.size();
  }

  JSER_INLINE auto dimensions() const noexcept -> std::size_t {
    return 1u;
  }

 private:
  auto element_class_() const
  -> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> override;

  auto size_() const noexcept -> std::size_t override;
  auto dimensions_() const noexcept -> std::size_t override;

  const ::java::lang::Class<::java::G::pack<>> element_type_;
  vector_type data_{
    ::cycle_ptr::cycle_allocator<::std::allocator<::java::field_t<::java::lang::Object>>>(*this)
  };
};

} /* namespace java::_erased::java */

namespace java::G {

/**
 * \brief Array tag.
 * \tparam T The element type of the array. Must be a java primitive type
 * or a generic.
 */
template<typename T>
struct array_t;

} /* namespace java::G */

namespace java {

template<template<class> class PtrImpl, typename Type>
class basic_ref<PtrImpl, Type*> final {
 protected:
  using ptr_type = PtrImpl<::java::_erased::java::array_intf>;

 public:
  using size_type = std::size_t;

  JSER_INLINE basic_ref() = default;
  JSER_INLINE basic_ref(const basic_ref&) = default;
  JSER_INLINE basic_ref(basic_ref&&) = default;
  JSER_INLINE basic_ref& operator=(const basic_ref&) = default;
  JSER_INLINE basic_ref& operator=(basic_ref&&) = default;
  JSER_INLINE ~basic_ref() noexcept = default;

  JSER_INLINE explicit operator bool() const noexcept {
    return bool(p_);
  }

  JSER_INLINE auto operator!() const noexcept -> bool {
    return !p_;
  }

  JSER_INLINE auto operator==(::std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  JSER_INLINE auto operator!=(::std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

  JSER_INLINE auto size() const noexcept -> size_type {
    if (!*this) return 0;
    return p_->size();
  }

 private:
  ptr_type p_ = nullptr;
};

template<template<class> class PtrImpl, typename Type>
class basic_ref<PtrImpl, Type*const> final {
 protected:
  using ptr_type = PtrImpl<::java::_erased::java::array_intf>;

 public:
  using size_type = std::size_t;

  JSER_INLINE basic_ref() = default;
  JSER_INLINE basic_ref(const basic_ref&) = default;
  JSER_INLINE basic_ref(basic_ref&&) = default;
  JSER_INLINE basic_ref& operator=(const basic_ref&) = default;
  JSER_INLINE basic_ref& operator=(basic_ref&&) = default;
  JSER_INLINE ~basic_ref() noexcept = default;

  JSER_INLINE explicit operator bool() const noexcept {
    return bool(p_);
  }

  JSER_INLINE auto operator!() const noexcept -> bool {
    return !p_;
  }

  JSER_INLINE auto operator==(::std::nullptr_t np) const noexcept -> bool {
    return p_ == nullptr;
  }

  JSER_INLINE auto operator!=(::std::nullptr_t np) const noexcept -> bool {
    return p_ != nullptr;
  }

  JSER_INLINE auto size() const noexcept -> size_type {
    if (!*this) return 0;
    return p_->size();
  }

 private:
  ptr_type p_ = nullptr;
};

namespace {

template<typename Type, std::size_t Dimensions>
struct add_dimensions_ {
  using type = typename add_dimensions_<Type, Dimensions - 1>::type*;
};

template<typename Type>
struct add_dimensions_<Type, 0> {
  using type = Type;
};

} /* namespace java::<unnamed> */

template<typename T, std::size_t Dimensions = 1>
using array_type = basic_ref<
    cycle_ptr::cycle_gptr,
    typename add_dimensions_<typename maybe_unpack_type_<T>::type, Dimensions>::type>;

} /* namespace java */


#if 0

namespace java {


template<typename T, unsigned int Dimensions = 1>
class array final
: public java::lang::Object,
  public virtual java::io::Serializable
{
  static_assert(Dimensions >= 1u, "Array requires at least one dimension.");

 private:
  static constexpr bool is_pointer = array<T, Dimensions - 1>::is_pointer;

  using vector_type = std::vector<
      array<T, Dimensions - 1>,
      cycle_ptr::cycle_allocator<std::allocator<array<T, Dimensions - 1>>>>;
  using initializer_list_type = std::initializer_list<
      typename array<T, Dimensions - 1>::initializer_list_type>;

 public:
  using value_type = typename vector_type::value_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reverse_iterator = typename vector_type::reverse_iterator;
  using const_reverse_iterator = typename vector_type::const_reverse_iterator;
  using size_type = typename vector_type::size_type;
  using difference_type = typename vector_type::difference_type;

  array()
  : data_(typename vector_type::allocator_type(*this))
  {}

  explicit array(cycle_ptr::unowned_cycle_t u)
  : java::lang::Object(u),
    data_(typename vector_type::allocator_type(*this))
  {}

  array(const array& other)
  : data_(other.data_, typename vector_type::allocator_type(*this))
  {}

  array(array&& other)
  : data_(std::move(other.data_), typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, const array& other)
  : java::lang::Object(u),
    data_(other.data_, typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, array&& other)
  : java::lang::Object(u),
    data_(std::move(other.data_), typename vector_type::allocator_type(*this))
  {}

  array(initializer_list_type il)
  : data_(il.begin(), il.end(), typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, initializer_list_type il)
  : java::lang::Object(u),
    data_(il.begin(), il.end(), typename vector_type::allocator_type(*this))
  {}

  template<typename Iter>
  array(Iter b, Iter e)
  : data_(b, e, typename vector_type::allocator_type(*this))
  {}

  template<typename Iter>
  array(cycle_ptr::unowned_cycle_t u, Iter b, Iter e)
  : java::lang::Object(u),
    data_(b, e, typename vector_type::allocator_type(*this))
  {}

  auto operator=(initializer_list_type il)
  -> array& {
    data_.assign(il.begin(), il.end());
    return *this;
  }

  auto operator=(const array& other)
  -> array& {
    data_ = other.data_;
    return *this;
  }

  auto operator=(array&& other)
  -> array& {
    data_ = std::move(other.data_);
    return *this;
  }

  template<typename Iter>
  auto assign(Iter b, Iter e)
  -> void {
    data_.assign(b, e);
  }

  auto begin() -> iterator { return data_.begin(); }
  auto end() -> iterator { return data_.end(); }
  auto rbegin() -> reverse_iterator { return data_.rbegin(); }
  auto rend() -> reverse_iterator { return data_.rend(); }

  auto begin() const -> const_iterator { return data_.begin(); }
  auto end() const -> const_iterator { return data_.end(); }
  auto rbegin() const -> const_reverse_iterator { return data_.rbegin(); }
  auto rend() const -> const_reverse_iterator { return data_.rend(); }

  auto cbegin() const -> const_iterator { return data_.cbegin(); }
  auto cend() const -> const_iterator { return data_.cend(); }
  auto crbegin() const -> const_reverse_iterator { return data_.crbegin(); }
  auto crend() const -> const_reverse_iterator { return data_.crend(); }

  auto empty() const -> bool { return data_.empty(); }
  auto size() const -> size_type { return data_.size(); }
  auto max_size() const -> size_type { return data_.max_size(); }
  auto capacity() const -> size_type { return data_.capacity(); }

  auto clear() -> void { data_.clear(); }
  auto shrink_to_fit() -> void { data_.shrink_to_fit(); }
  auto reserve(size_type n) -> void { data_.reserve(n); }
  auto resize(size_type n) -> void {
    data_.resize(n, typename vector_type::value_type(cycle_ptr::unowned_cycle));
  }

  auto operator[](size_type idx) -> reference { return data_[idx]; }
  auto operator[](size_type idx) const -> const_reference { return data_[idx]; }
  auto at(size_type idx) -> reference { return data_.at(idx); }
  auto at(size_type idx) const -> const_reference { return data_.at(idx); }
  auto front() -> reference { return data_.front(); }
  auto front() const -> const_reference { return data_.front(); }
  auto back() -> reference { return data_.back(); }
  auto back() const -> const_reference { return data_.back(); }

  auto operator==(const array& other) const
  -> bool {
    return data_ == other.data_;
  }

  auto operator!=(const array& other) const
  -> bool {
    return data_ == other.data_;
  }

  auto swap(array& other) -> void { data_.swap(other.data_); }

  auto pop_back() -> void {
    data_.pop_back();
  }

  auto push_back(const_reference ptr)
  -> void {
    data_.push_back(ptr);
  }

  auto push_back(value_type&& ptr)
  -> void {
    data_.push_back(std::move(ptr));
  }

  auto insert(const_iterator pos, const_reference ptr)
  -> iterator {
    return data_.insert(pos, ptr);
  }

  auto insert(const_iterator pos, value_type&& ptr)
  -> iterator {
    return data_.insert(pos, std::move(ptr));
  }

  template<typename Iter>
  auto insert(const_iterator pos, Iter b, Iter e)
  -> iterator {
    return data_.insert(pos, b, e);
  }

  auto erase(const_iterator i)
  -> iterator {
    return data_.erase(i);
  }

  auto erase(const_iterator b, const_iterator e)
  -> iterator {
    return data_.erase(b, e);
  }

 private:
  auto do_encode_(java::io::Serializable::tag t) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override {
    return java::serialization::type_def<array>::encode(*this);
  }

  auto do_encode_(java::io::Serializable::tag t, java::serialization::cycle_handler& h) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override {
    return h.encode_field<array>(this->template shared_from_this(this));
  }

  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override {
    return java::serialization::type_def<array>::get_class();
  }

  vector_type data_;
};

namespace {
template<typename T>
struct array_member_ptr_to_gptr_ {
  static constexpr bool is_pointer = false;
  using type = T;
};

template<typename T>
struct array_member_ptr_to_gptr_<cycle_ptr::cycle_member_ptr<T>> {
  static constexpr bool is_pointer = true;
  using type = cycle_ptr::cycle_gptr<T>;
};
}

template<typename T>
class array<T, 1> final
: public java::lang::Object,
  public virtual java::io::Serializable
{
  template<typename, unsigned int> friend class java::array;

 private:
  static constexpr bool is_pointer = array_member_ptr_to_gptr_<std::remove_cv_t<T>>::is_pointer;
  using global_t = typename array_member_ptr_to_gptr_<std::remove_cv_t<T>>::type;

  using vector_type = std::vector<T, cycle_ptr::cycle_allocator<std::allocator<T>>>;
  using initializer_list_type = std::initializer_list<global_t>;

 public:
  using value_type = typename vector_type::value_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reverse_iterator = typename vector_type::reverse_iterator;
  using const_reverse_iterator = typename vector_type::const_reverse_iterator;
  using size_type = typename vector_type::size_type;
  using difference_type = typename vector_type::difference_type;

  array()
  : data_(typename vector_type::allocator_type(*this))
  {}

  explicit array(cycle_ptr::unowned_cycle_t u)
  : java::lang::Object(u),
    data_(typename vector_type::allocator_type(*this))
  {}

  array(const array& other)
  : data_(other.data_, typename vector_type::allocator_type(*this))
  {}

  array(array&& other)
  : data_(std::move(other.data_), typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, const array& other)
  : java::lang::Object(u),
    data_(other.data_, typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, array&& other)
  : java::lang::Object(u),
    data_(std::move(other.data_), typename vector_type::allocator_type(*this))
  {}

  array(initializer_list_type il)
  : data_(il.begin(), il.end(), typename vector_type::allocator_type(*this))
  {}

  array(cycle_ptr::unowned_cycle_t u, initializer_list_type il)
  : java::lang::Object(u),
    data_(il.begin(), il.end(), typename vector_type::allocator_type(*this))
  {}

  template<typename Iter>
  array(Iter b, Iter e)
  : data_(b, e, typename vector_type::allocator_type(*this))
  {}

  template<typename Iter>
  array(cycle_ptr::unowned_cycle_t u, Iter b, Iter e)
  : java::lang::Object(u),
    data_(b, e, typename vector_type::allocator_type(*this))
  {}

  auto operator=(initializer_list_type il)
  -> array& {
    data_.assign(il.begin(), il.end());
    return *this;
  }

  auto operator=(const array& other)
  -> array& {
    data_ = other.data_;
    return *this;
  }

  auto operator=(array&& other)
  -> array& {
    data_ = std::move(other.data_);
    return *this;
  }

  template<typename Iter>
  auto assign(Iter b, Iter e)
  -> void {
    data_.assign(b, e);
  }

  auto begin() -> iterator { return data_.begin(); }
  auto end() -> iterator { return data_.end(); }
  auto rbegin() -> reverse_iterator { return data_.rbegin(); }
  auto rend() -> reverse_iterator { return data_.rend(); }

  auto begin() const -> const_iterator { return data_.begin(); }
  auto end() const -> const_iterator { return data_.end(); }
  auto rbegin() const -> const_reverse_iterator { return data_.rbegin(); }
  auto rend() const -> const_reverse_iterator { return data_.rend(); }

  auto cbegin() const -> const_iterator { return data_.cbegin(); }
  auto cend() const -> const_iterator { return data_.cend(); }
  auto crbegin() const -> const_reverse_iterator { return data_.crbegin(); }
  auto crend() const -> const_reverse_iterator { return data_.crend(); }

  auto empty() const -> bool { return data_.empty(); }
  auto size() const -> size_type { return data_.size(); }
  auto max_size() const -> size_type { return data_.max_size(); }
  auto capacity() const -> size_type { return data_.capacity(); }

  auto clear() -> void { data_.clear(); }
  auto shrink_to_fit() -> void { data_.shrink_to_fit(); }
  auto reserve(size_type n) -> void { data_.reserve(n); }
  auto resize(size_type n) -> void {
    if constexpr(is_pointer)
      data_.resize(n, typename vector_type::value_type(cycle_ptr::unowned_cycle));
    else
      data_.resize(n);
  }

  auto operator[](size_type idx) -> reference { return data_[idx]; }
  auto operator[](size_type idx) const -> const_reference { return data_[idx]; }
  auto at(size_type idx) -> reference { return data_.at(idx); }
  auto at(size_type idx) const -> const_reference { return data_.at(idx); }
  auto front() -> reference { return data_.front(); }
  auto front() const -> const_reference { return data_.front(); }
  auto back() -> reference { return data_.back(); }
  auto back() const -> const_reference { return data_.back(); }

  auto operator==(const array& other) const
  -> bool {
    return data_ == other.data_;
  }

  auto operator!=(const array& other) const
  -> bool {
    return data_ == other.data_;
  }

  auto swap(array& other) -> void { data_.swap(other.data_); }

  auto pop_back() -> void {
    data_.pop_back();
  }

  auto push_back(const_reference ptr)
  -> void {
    data_.push_back(ptr);
  }

  auto push_back(value_type&& ptr)
  -> void {
    data_.push_back(std::move(ptr));
  }

  template<bool Enable = is_pointer>
  auto push_back(const global_t& ptr)
  -> std::enable_if_t<Enable> {
    data_.emplace_back(ptr);
  }

  template<bool Enable = is_pointer>
  auto push_back(global_t&& ptr)
  -> std::enable_if_t<Enable> {
    data_.emplace_back(std::move(ptr));
  }

  auto insert(const_iterator pos, const_reference ptr)
  -> iterator {
    return data_.insert(pos, ptr);
  }

  auto insert(const_iterator pos, value_type&& ptr)
  -> iterator {
    return data_.insert(pos, std::move(ptr));
  }

  template<bool Enable = is_pointer>
  auto insert(const_iterator pos, const global_t& ptr)
  -> std::enable_if_t<Enable, iterator> {
    return insert(pos, value_type(cycle_ptr::unowned_cycle, ptr));
  }

  template<bool Enable = is_pointer>
  auto insert(const_iterator pos, global_t&& ptr)
  -> std::enable_if_t<Enable, iterator> {
    return insert(pos, value_type(cycle_ptr::unowned_cycle, std::move(ptr)));
  }

  template<typename Iter>
  auto insert(const_iterator pos, Iter b, Iter e)
  -> iterator {
    return data_.insert(pos, b, e);
  }

  auto erase(const_iterator i)
  -> iterator {
    return data_.erase(i);
  }

  auto erase(const_iterator b, const_iterator e)
  -> iterator {
    return data_.erase(b, e);
  }

 private:
  auto do_encode_(java::io::Serializable::tag t) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override {
    return java::serialization::type_def<array>::encode(*this);
  }

  auto do_encode_(java::io::Serializable::tag t, java::serialization::cycle_handler& h) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override {
    return h.encode_field<array>(this->template shared_from_this(this));
  }

  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override {
    return java::serialization::type_def<array>::get_class();
  }

  vector_type data_;
};


extern template class array<cycle_ptr::cycle_member_ptr<java::lang::Object>, 1>;
extern template class array<cycle_ptr::cycle_member_ptr<java::lang::Object>, 2>;
extern template class array<int, 1>;
extern template class array<int, 2>;


} /* namespace java */

namespace java::serialization {


template<typename T, unsigned int Dimensions>
struct type_def<java::array<T, Dimensions>> {
  static_assert(Dimensions >= 1u,
      "Arrays must have at least a dimension of 1.");

  /**
   * \brief Select class name for the array type.
   * \note Returned name is a field_descriptor, not a string_view.
   */
  static auto java_class_name()
  -> java::serialization::stream::field_descriptor {
    using namespace java::serialization::stream;

    if constexpr(std::is_same_v<java::boolean_t, T>)
      return field_descriptor(primitive_type::bool_type, Dimensions);
    else if constexpr(std::is_same_v<java::char_t, T>)
      return field_descriptor(primitive_type::char_type, Dimensions);
    else if constexpr(std::is_same_v<java::byte_t, T>)
      return field_descriptor(primitive_type::byte_type, Dimensions);
    else if constexpr(std::is_same_v<java::short_t, T>)
      return field_descriptor(primitive_type::short_type, Dimensions);
    else if constexpr(std::is_same_v<java::int_t, T>)
      return field_descriptor(primitive_type::int_type, Dimensions);
    else if constexpr(std::is_same_v<java::long_t, T>)
      return field_descriptor(primitive_type::long_type, Dimensions);
    else if constexpr(std::is_same_v<java::float_t, T>)
      return field_descriptor(primitive_type::float_type, Dimensions);
    else if constexpr(std::is_same_v<java::double_t, T>)
      return field_descriptor(primitive_type::double_type, Dimensions);
    else if constexpr(std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>)
      return field_descriptor(type_def<typename T::element_type>::java_class_name(), Dimensions);
    else
      return field_descriptor(type_def<T>::java_class_name(), Dimensions);
  }

  /**
   * \brief Retrieve class for array type.
   */
  static auto get_class()
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> {
    using namespace java::serialization::stream;

    const auto result = cycle_ptr::make_cycle<new_class_desc__class_desc>(java_class_name(), 0);
    result->info.flags = SC_SERIALIZABLE;
    return result;
  }

  template<typename Collection>
  static auto encode(const Collection& v)
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_array> {
    using std::begin;
    using std::end;
    using namespace java::serialization::stream;

    if constexpr(Dimensions == 1u
        && (std::is_same_v<java::boolean_t, T>
            || std::is_same_v<java::char_t, T>
            || std::is_same_v<java::byte_t, T>
            || std::is_same_v<java::short_t, T>
            || std::is_same_v<java::int_t, T>
            || std::is_same_v<java::long_t, T>
            || std::is_same_v<java::float_t, T>
            || std::is_same_v<java::double_t, T>)) {
      return cycle_ptr::make_cycle<new_array>(
          std::in_place_type<T>,
          get_class(),
          begin(v), end(v));
    } else if constexpr(Dimensions != 1u) {
      cycle_handler handler;
      return cycle_ptr::make_cycle<new_array>(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<array<T, Dimensions - 1u>>(begin(v), end(v)));
    } else if constexpr(std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>) {
      cycle_handler handler;
      return cycle_ptr::make_cycle<new_array>(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<typename T::element_type>(begin(v), end(v)));
    } else {
      cycle_handler handler;
      return cycle_ptr::make_cycle<new_array>(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<T>(begin(v), end(v)));
    }
  }

  template<typename Collection>
  static auto encode(const Collection& v,
      cycle_ptr::cycle_gptr<java::serialization::stream::new_array> result,
      cycle_handler& handler)
  -> void {
    using std::begin;
    using std::end;
    using namespace java::serialization::stream;

    if constexpr(Dimensions == 1u
        && (std::is_same_v<java::boolean_t, T>
            || std::is_same_v<java::char_t, T>
            || std::is_same_v<java::byte_t, T>
            || std::is_same_v<java::short_t, T>
            || std::is_same_v<java::int_t, T>
            || std::is_same_v<java::long_t, T>
            || std::is_same_v<java::float_t, T>
            || std::is_same_v<java::double_t, T>)) {
      result->assign(
          std::in_place_type<T>,
          get_class(),
          begin(v), end(v));
    } else if constexpr(Dimensions != 1u) {
      result->assign(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<array<T, Dimensions - 1u>>(begin(v), end(v)));
    } else if constexpr(std::is_convertible_v<T, cycle_ptr::cycle_gptr<const void>>) {
      result->assign(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<typename T::element_type>(begin(v), end(v)));
    } else {
      result->assign(
          std::in_place_type<referent>,
          get_class(),
          handler.template encode_obj_vector<T>(begin(v), end(v)));
    }
  }
};


} /* namespace java::serialization */

#endif

#endif /* JAVA_ARRAY_H */
