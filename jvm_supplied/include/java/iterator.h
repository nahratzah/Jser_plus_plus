#ifndef JAVA_ITERATOR_H
#define JAVA_ITERATOR_H

#include <memory>
#include <type_traits>
#include <utility>
#include <java/fwd/java/lang/Object.h>
#include <java/ref.h>

namespace java::type_traits {
namespace {

template<typename>
struct is_iterator_
: std::false_type
{};

template<typename>
struct is_pair_iterator_
: std::false_type
{};

} /* namespace java::type_traits::<unnamed> */


/**
 * \brief Type trait to figure out if a type is a java::iterator.
 * \details Evaluates to `std::true_type` if this type is a java::iterator.
 * Evaluates to `std::false_type` otherwise.
 * \tparam T the type to test if it is an iterator.
 */
template<typename T>
using is_iterator = typename is_iterator_<T>::type;

/**
 * \brief Type trait to figure out if a type is a java::iterator.
 * \details Evaluates to `true` if this type is a java::iterator.
 * Evaluates to `false` otherwise.
 * \tparam T the type to test if it is an iterator.
 */
template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;


/**
 * \brief Type trait to figure out if a type is a java::pair_iterator.
 * \details Evaluates to `std::true_type` if this type is a java::pair_iterator.
 * Evaluates to `std::false_type` otherwise.
 * \tparam T the type to test if it is an iterator.
 */
template<typename T>
using is_pair_iterator = typename is_pair_iterator_<T>::type;

/**
 * \brief Type trait to figure out if a type is a java::pair_iterator.
 * \details Evaluates to `true` if this type is a java::pair_iterator.
 * Evaluates to `false` otherwise.
 * \tparam T the type to test if it is an iterator.
 */
template<typename T>
constexpr bool is_pair_iterator_v = is_pair_iterator<T>::value;


} /* namespace java::type_traits */

namespace java {


template<typename Type>
class forward_iterator;

template<typename Type>
class bidirectional_iterator;


template<typename Key, typename Mapped>
class forward_pair_iterator;

template<typename Key, typename Mapped>
class bidirectional_pair_iterator;


template<typename T, typename Iterator, typename = void>
struct retype_iterator_;

template<typename T, template<typename> class Iterator, typename IteratorType>
struct retype_iterator_<T, Iterator<IteratorType>,
    std::enable_if_t<
        ::java::type_traits::is_iterator_v<Iterator<IteratorType>>
        && ::java::type_traits::is_generic_v<std::remove_const_t<T>>
    >>
{
  using type = Iterator<T>;
};

template<typename Key, typename Mapped, template<typename, typename> class Iterator, typename IteratorKey, typename IteratorMapped>
struct retype_iterator_<::std::pair<Key, Mapped>, Iterator<IteratorKey, IteratorMapped>,
    std::enable_if_t<
        ::java::type_traits::is_iterator_v<Iterator<IteratorKey, IteratorMapped>>
        && ::java::type_traits::is_generic_v<std::remove_const_t<Key>>
        && ::java::type_traits::is_generic_v<std::remove_const_t<Mapped>>
    >>
{
  using type = Iterator<Key, Mapped>;
};

/**
 * \brief Cast iterator to different iterator.
 * \details Casts X to its counterpart holding type of R,
 * then performs implicit conversion from that to R.
 *
 * \attention This is unsafe: we cannot detect a bad cast until you dereference
 * the returned iterator.
 */
template<typename R, typename X>
auto cast(X&& x)
-> std::enable_if_t<
    ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
    && ::java::type_traits::is_iterator_v<R>
    && !::java::type_traits::is_pair_iterator_v<R>
    && (std::is_const_v<type_of_t<typename R::value_type>>
        || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
    && std::is_convertible_v<
        typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
        R>,
    R> {
  using retyped_x = typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type;

  return retyped_x(_cast(), std::forward<X>(x));
}

/**
 * \brief Cast iterator to different iterator.
 * \details Casts X to its counterpart holding type of R,
 * then performs implicit conversion from that to R.
 *
 * \attention This is unsafe: we cannot detect a bad cast until you dereference
 * the returned iterator.
 */
template<typename R, typename X>
auto cast(X&& x)
-> std::enable_if_t<
    ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
    && ::java::type_traits::is_pair_iterator_v<R>
    && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
            && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
        || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
            && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
    && std::is_convertible_v<
        typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
        R>,
    R> {
  using retyped_x = typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type;

  return retyped_x(_cast(), std::forward<X>(x));
}


} /* namespace java */

namespace java::type_traits {
namespace {

template<typename Type>
struct is_iterator_<forward_iterator<Type>>
: std::true_type
{};

template<typename Type>
struct is_iterator_<bidirectional_iterator<Type>>
: std::true_type
{};

template<typename Key, typename Mapped>
struct is_iterator_<forward_pair_iterator<Key, Mapped>>
: std::true_type
{};

template<typename Key, typename Mapped>
struct is_iterator_<bidirectional_pair_iterator<Key, Mapped>>
: std::true_type
{};

template<typename Key, typename Mapped>
struct is_pair_iterator_<forward_pair_iterator<Key, Mapped>>
: std::true_type
{};

template<typename Key, typename Mapped>
struct is_pair_iterator_<bidirectional_pair_iterator<Key, Mapped>>
: std::true_type
{};

} /* namespace java::type_traits::<unnamed> */
} /* namespace java::type_traits */

namespace java {


template<>
class forward_iterator<type_of_t<const_ref<java::lang::Object>>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = const_ref<java::lang::Object>;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  class interface {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Prefix operator++.
     * \details Increments this.
     */
    virtual auto operator++() -> void = 0;

    /**
     * \brief Dereference operation.
     * \returns Object.
     */
    virtual auto cderef() const -> const_ref<java::lang::Object> = 0;

    /**
     * \brief Compare for equality with other interface.
     * \param y The other interface to test equality with. May be null.
     */
    virtual auto operator==(const interface* y) const noexcept -> bool = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    virtual auto copy() const
    -> interface* = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator([[maybe_unused]] _cast c, const forward_iterator<T>& i)
  : forward_iterator(i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator([[maybe_unused]] _cast c, forward_iterator<T>&& i) noexcept
  : forward_iterator(std::move(i))
  {}

 public:
  constexpr forward_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  forward_iterator(Iterator&& iter);

  forward_iterator(const forward_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_iterator(forward_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  forward_iterator(const forward_iterator<type_of_t<java::lang::Object>>& other);
  forward_iterator(forward_iterator<type_of_t<java::lang::Object>>&& other) noexcept;

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      >>
  forward_iterator(const forward_iterator<OtherType>& other)
  : forward_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      >>
  forward_iterator(forward_iterator<OtherType>&& other) noexcept
  : forward_iterator(std::move(other.iter_))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(const bidirectional_iterator<OtherType>& other)
  : forward_iterator(forward_iterator<OtherType>(other))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : forward_iterator(forward_iterator<OtherType>(std::move(other)))
  {}

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename X>
  auto operator==(const forward_iterator<X>& other) const noexcept -> bool;
  template<typename X>
  auto operator==(const bidirectional_iterator<X>& other) const noexcept -> bool;

  auto operator=(const forward_iterator& other) -> forward_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_iterator&& other) noexcept -> forward_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> forward_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> forward_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator*() const -> const_ref<java::lang::Object> {
    return iter_ptr_->cderef();
  }

  auto operator->() const -> const_ref<java::lang::Object> {
    return iter_ptr_->cderef();
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class forward_iterator<type_of_t<java::lang::Object>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = java::lang::Object;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  class interface
  : public virtual forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Dereference operation.
     * \returns Object.
     */
    virtual auto deref() const -> java::lang::Object = 0;

    auto cderef() const -> const_ref<java::lang::Object> override final;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    virtual auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator([[maybe_unused]] _cast c, const forward_iterator<T>& i)
  : forward_iterator(i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator([[maybe_unused]] _cast c, forward_iterator<T>&& i) noexcept
  : forward_iterator(std::move(i))
  {}

 public:
  constexpr forward_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  forward_iterator(Iterator&& iter);

  forward_iterator(const forward_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_iterator(forward_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      && !std::is_const_v<OtherType>>>
  forward_iterator(const forward_iterator<OtherType>& other)
  : forward_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      && !std::is_const_v<OtherType>>>
  forward_iterator(forward_iterator<OtherType>&& other) noexcept
  : forward_iterator(std::move(other.iter_))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(const bidirectional_iterator<OtherType>& other)
  : forward_iterator(forward_iterator<OtherType>(other))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : forward_iterator(forward_iterator<OtherType>(std::move(other)))
  {}

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename X>
  auto operator==(const forward_iterator<X>& other) const noexcept -> bool;
  template<typename X>
  auto operator==(const bidirectional_iterator<X>& other) const noexcept -> bool;

  auto operator=(const forward_iterator& other) -> forward_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_iterator&& other) noexcept -> forward_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> forward_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> forward_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator*() const -> java::lang::Object {
    return iter_ptr_->deref();
  }

  auto operator->() const -> java::lang::Object {
    return iter_ptr_->deref();
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = const_ref<java::lang::Object>;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  class interface
  : public virtual forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Prefix operator--
     * \details Decrements this.
     */
    virtual auto operator--() -> void = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator([[maybe_unused]] _cast c, const bidirectional_iterator<T>& i)
  : bidirectional_iterator(i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator([[maybe_unused]] _cast c, bidirectional_iterator<T>&& i) noexcept
  : bidirectional_iterator(std::move(i))
  {}

 public:
  constexpr bidirectional_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_iterator(Iterator&& iter);

  bidirectional_iterator(const bidirectional_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_iterator(bidirectional_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  bidirectional_iterator(const bidirectional_iterator<type_of_t<java::lang::Object>>& other);
  bidirectional_iterator(bidirectional_iterator<type_of_t<java::lang::Object>>&& other) noexcept;

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      >>
  bidirectional_iterator(const bidirectional_iterator<OtherType>& other)
  : bidirectional_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      >>
  bidirectional_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : bidirectional_iterator(std::move(other.iter_))
  {}

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename X>
  auto operator==(const forward_iterator<X>& other) const noexcept -> bool;
  template<typename X>
  auto operator==(const bidirectional_iterator<X>& other) const noexcept -> bool;

  auto operator=(const bidirectional_iterator& other) -> bidirectional_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_iterator&& other) noexcept -> bidirectional_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> bidirectional_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> bidirectional_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator--() -> bidirectional_iterator& {
    --*iter_ptr_;
    return *this;
  }

  auto operator--(int) -> bidirectional_iterator {
    auto copy = *this;
    --*this;
    return copy;
  }

  auto operator*() const -> const_ref<java::lang::Object> {
    return iter_ptr_->cderef();
  }

  auto operator->() const -> const_ref<java::lang::Object> {
    return iter_ptr_->cderef();
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_iterator<type_of_t<java::lang::Object>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = java::lang::Object;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  class interface
  : public virtual forward_iterator<type_of_t<java::lang::Object>>::interface,
    public virtual bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator([[maybe_unused]] _cast c, const bidirectional_iterator<T>& i)
  : bidirectional_iterator(i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator([[maybe_unused]] _cast c, bidirectional_iterator<T>&& i) noexcept
  : bidirectional_iterator(std::move(i))
  {}

 public:
  constexpr bidirectional_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_iterator(Iterator&& iter);

  bidirectional_iterator(const bidirectional_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_iterator(bidirectional_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      && !std::is_const_v<OtherType>>>
  bidirectional_iterator(const bidirectional_iterator<OtherType>& other)
  : bidirectional_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<java::type_traits::is_assignable_v<
          type_of_t<java::lang::Object>,
          std::remove_const_t<OtherType>>
      && !std::is_const_v<OtherType>>>
  bidirectional_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : bidirectional_iterator(std::move(other.iter_))
  {}

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename X>
  auto operator==(const forward_iterator<X>& other) const noexcept -> bool;
  template<typename X>
  auto operator==(const bidirectional_iterator<X>& other) const noexcept -> bool;

  auto operator=(const bidirectional_iterator& other) -> bidirectional_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_iterator&& other) noexcept -> bidirectional_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> bidirectional_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> bidirectional_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator--() -> bidirectional_iterator& {
    --*iter_ptr_;
    return *this;
  }

  auto operator--(int) -> bidirectional_iterator {
    auto copy = *this;
    --*this;
    return copy;
  }

  auto operator*() const -> java::lang::Object {
    return iter_ptr_->deref();
  }

  auto operator->() const -> java::lang::Object {
    return iter_ptr_->deref();
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<typename Iterator>
class forward_iterator<type_of_t<java::lang::Object>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto deref() const -> java::lang::Object override {
    return *iter_;
  }

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class forward_iterator<type_of_t<const_ref<java::lang::Object>>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto cderef() const -> const_ref<java::lang::Object> override {
    return *iter_;
  }

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class bidirectional_iterator<type_of_t<java::lang::Object>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto operator--() -> void override {
    --iter_;
  }

  auto deref() const -> java::lang::Object override {
    return *iter_;
  }

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto operator--() -> void override {
    --iter_;
  }

  auto cderef() const -> const_ref<java::lang::Object> override {
    return *iter_;
  }

  auto operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Type>
class forward_iterator {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = var_ref<Type>;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  constexpr forward_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              var_ref<Type>>
          && !java::type_traits::is_iterator_v<Iterator>>>
  forward_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Type>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit forward_iterator(const forward_iterator<type_of_t<obj_type>>& iter)
  : iter_(iter)
  {}

  explicit forward_iterator(forward_iterator<type_of_t<obj_type>>&& iter) noexcept
  : iter_(std::move(iter))
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator(_cast c, const forward_iterator<T>& i)
  : iter_(c, i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  forward_iterator(_cast c, forward_iterator<T>&& i) noexcept
  : iter_(c, std::move(i))
  {}

 public:
  template<typename OtherType, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Type>,
              std::remove_const_t<OtherType>>
          && (std::is_const_v<Type> || !std::is_const_v<OtherType>)
      >>
  forward_iterator(const forward_iterator<OtherType>& other)
  : forward_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Type>,
              std::remove_const_t<OtherType>>
          && (std::is_const_v<Type> || !std::is_const_v<OtherType>)
      >>
  forward_iterator(forward_iterator<OtherType>&& other) noexcept
  : forward_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(const bidirectional_iterator<OtherType>& other)
  : forward_iterator(forward_iterator<OtherType>(other))
  {}

  template<typename OtherType, typename =
      std::enable_if_t<std::is_convertible_v<forward_iterator<OtherType>, forward_iterator>>>
  forward_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : forward_iterator(forward_iterator<OtherType>(std::move(other)))
  {}

  template<typename Iterator>
  auto operator==(const Iterator& other) const noexcept
  -> std::enable_if_t<::java::type_traits::is_iterator_v<Iterator>, bool> {
    return iter_ == other;
  }

  auto operator++() -> forward_iterator& {
    ++iter_;
    return *this;
  }

  auto operator++(int) -> forward_iterator {
    return forward_iterator(iter_++);
  }

  auto operator*() const -> var_ref<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

  auto operator->() const -> var_ref<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

 private:
  forward_iterator<type_of_t<obj_type>> iter_;
};


template<typename Type>
class bidirectional_iterator {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_iterator_v<R>
      && !::java::type_traits::is_pair_iterator_v<R>
      && (std::is_const_v<type_of_t<typename R::value_type>>
          || !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type>>)
      && std::is_convertible_v<
          typename retype_iterator_<type_of_t<typename R::value_type>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename> friend class ::java::forward_iterator;
  template<typename> friend class ::java::bidirectional_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = var_ref<Type>;
  using pointer = value_type;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  constexpr bidirectional_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              var_ref<Type>>
          && !java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Type>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit bidirectional_iterator(const bidirectional_iterator<type_of_t<obj_type>>& iter)
  : iter_(iter)
  {}

  explicit bidirectional_iterator(bidirectional_iterator<type_of_t<obj_type>>&& iter) noexcept
  : iter_(std::move(iter))
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator(_cast c, const bidirectional_iterator<T>& i)
  : iter_(c, i)
  {}

  template<typename T, typename = std::enable_if_t<
      std::is_const_v<type_of_t<value_type>>
      || !std::is_const_v<T>
      >>
  bidirectional_iterator(_cast c, bidirectional_iterator<T>&& i) noexcept
  : iter_(c, std::move(i))
  {}

 public:
  template<typename OtherType, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Type>,
              std::remove_const_t<OtherType>>
          && (std::is_const_v<Type> || !std::is_const_v<OtherType>)
      >>
  bidirectional_iterator(const bidirectional_iterator<OtherType>& other)
  : bidirectional_iterator(other.iter_)
  {}

  template<typename OtherType, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Type>,
              std::remove_const_t<OtherType>>
          && (std::is_const_v<Type> || !std::is_const_v<OtherType>)
      >>
  bidirectional_iterator(bidirectional_iterator<OtherType>&& other) noexcept
  : bidirectional_iterator(other.iter_)
  {}

  template<typename Iterator>
  auto operator==(const Iterator& other) const noexcept
  -> std::enable_if_t<::java::type_traits::is_iterator_v<Iterator>, bool> {
    return iter_ == other;
  }

  auto operator++() -> bidirectional_iterator& {
    ++iter_;
    return *this;
  }

  auto operator++(int) -> bidirectional_iterator {
    return bidirectional_iterator(iter_++);
  }

  auto operator--() -> bidirectional_iterator& {
    --iter_;
    return *this;
  }

  auto operator--(int) -> bidirectional_iterator {
    return bidirectional_iterator(iter_--);
  }

  auto operator*() const -> var_ref<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

  auto operator->() const -> var_ref<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

 private:
  bidirectional_iterator<type_of_t<obj_type>> iter_;
};


template<typename Iterator, typename>
forward_iterator<type_of_t<const_ref<java::lang::Object>>>::forward_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline forward_iterator<type_of_t<const_ref<java::lang::Object>>>::forward_iterator(const forward_iterator<type_of_t<java::lang::Object>>& other)
: iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
{}

inline forward_iterator<type_of_t<const_ref<java::lang::Object>>>::forward_iterator(forward_iterator<type_of_t<java::lang::Object>>&& other) noexcept
: iter_ptr_(std::move(other.iter_ptr_))
{}

inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename X>
inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename X>
inline auto forward_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
forward_iterator<type_of_t<java::lang::Object>>::forward_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename X>
inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename X>
inline auto forward_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::bidirectional_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::bidirectional_iterator(const bidirectional_iterator<type_of_t<java::lang::Object>>& other)
: iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
{}

inline bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::bidirectional_iterator(bidirectional_iterator<type_of_t<java::lang::Object>>&& other) noexcept
: iter_ptr_(std::move(other.iter_ptr_))
{}

inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename X>
inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename X>
inline auto bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
bidirectional_iterator<type_of_t<java::lang::Object>>::bidirectional_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename X>
inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const forward_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename X>
inline auto bidirectional_iterator<type_of_t<java::lang::Object>>::operator==(const bidirectional_iterator<X>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename X, typename Y>
inline auto operator!=(const forward_iterator<X>& x, const forward_iterator<Y>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X, typename Y>
inline auto operator!=(const forward_iterator<X>& x, const bidirectional_iterator<Y>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X, typename Y>
inline auto operator!=(const bidirectional_iterator<X>& x, const forward_iterator<Y>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X, typename Y>
inline auto operator!=(const bidirectional_iterator<X>& x, const bidirectional_iterator<Y>& y) noexcept -> bool {
  return !(x == y);
}


template<typename First, typename Second>
class pair_proxy {
 public:
  explicit pair_proxy(First first, Second second)
  : value_(std::forward<First>(first), std::forward<Second>(second))
  {}

  auto operator->() -> ::std::pair<First, Second>* {
    return &value_;
  }

 private:
  ::std::pair<First, Second> value_;
};


template<>
class forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>>;
  using pointer = pair_proxy<const_ref<java::lang::Object>, const_ref<java::lang::Object>>;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  class interface {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Prefix operator++.
     * \details Increments this.
     */
    virtual auto operator++() -> void = 0;

    /**
     * \brief Dereference operation.
     * \returns Object.
     */
    virtual auto cderef() const
    -> ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>> = 0;

    /**
     * \brief Compare for equality with other interface.
     * \param y The other interface to test equality with. May be null.
     */
    virtual auto operator==(const interface* y) const noexcept -> bool = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    virtual auto copy() const
    -> interface* = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  forward_pair_iterator([[maybe_unused]] _cast c, const forward_pair_iterator<Key, Mapped>& i)
  : forward_pair_iterator(i)
  {}

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  forward_pair_iterator([[maybe_unused]] _cast c, forward_pair_iterator<Key, Mapped>&& i) noexcept
  : forward_pair_iterator(std::move(i))
  {}

 public:
  constexpr forward_pair_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  forward_pair_iterator(Iterator&& iter);

  forward_pair_iterator(const forward_pair_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_pair_iterator(forward_pair_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  forward_pair_iterator(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other);
  forward_pair_iterator(forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>&& other) noexcept;

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
      >>
  forward_pair_iterator(const forward_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
      >>
  forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(std::move(other.iter_))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(other))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(std::move(other)))
  {}

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename XKey, typename XMapped>
  auto operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;
  template<typename XKey, typename XMapped>
  auto operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;

  auto operator=(const forward_pair_iterator& other) -> forward_pair_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_pair_iterator&& other) noexcept -> forward_pair_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> forward_pair_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> forward_pair_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator*() const -> reference {
    return iter_ptr_->cderef();
  }

  auto operator->() const -> pointer {
    auto dereferenced = iter_ptr_->cderef();
    return pointer(::std::move(dereferenced.first), ::std::move(dereferenced.second));
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<java::lang::Object, java::lang::Object>;
  using pointer = pair_proxy<java::lang::Object, java::lang::Object>;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  class interface
  : public virtual forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Dereference operation.
     * \returns Object.
     */
    virtual auto deref() const
    -> ::std::pair<java::lang::Object, java::lang::Object> = 0;

    auto cderef() const
    -> ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>> override final;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    virtual auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  forward_pair_iterator([[maybe_unused]] _cast c, const forward_pair_iterator<Key, Mapped>& i)
  : forward_pair_iterator(i)
  {}

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  forward_pair_iterator([[maybe_unused]] _cast c, forward_pair_iterator<Key, Mapped>&& i) noexcept
  : forward_pair_iterator(std::move(i))
  {}

 public:
  constexpr forward_pair_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  forward_pair_iterator(Iterator&& iter);

  forward_pair_iterator(const forward_pair_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_pair_iterator(forward_pair_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
          && !std::is_const_v<OtherKey>
          && !std::is_const_v<OtherMapped>>>
  forward_pair_iterator(const forward_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
          && !std::is_const_v<OtherKey>
          && !std::is_const_v<OtherMapped>>>
  forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(std::move(other.iter_))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(other))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(std::move(other)))
  {}

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename XKey, typename XMapped>
  auto operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;
  template<typename XKey, typename XMapped>
  auto operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;

  auto operator=(const forward_pair_iterator& other) -> forward_pair_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_pair_iterator&& other) noexcept -> forward_pair_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> forward_pair_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> forward_pair_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator*() const -> reference {
    return iter_ptr_->deref();
  }

  auto operator->() const -> pointer {
    auto dereferenced = iter_ptr_->deref();
    return pointer(::std::move(dereferenced.first), ::std::move(dereferenced.second));
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>>;
  using pointer = pair_proxy<const_ref<java::lang::Object>, const_ref<java::lang::Object>>;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  class interface
  : public virtual forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Prefix operator--
     * \details Decrements this.
     */
    virtual auto operator--() -> void = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<value_type::first_type>> && std::is_const_v<type_of_t<value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  bidirectional_pair_iterator([[maybe_unused]] _cast c, const bidirectional_pair_iterator<Key, Mapped>& i)
  : bidirectional_pair_iterator(i)
  {}

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<value_type::first_type>> && std::is_const_v<type_of_t<value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  bidirectional_pair_iterator([[maybe_unused]] _cast c, bidirectional_pair_iterator<Key, Mapped>&& i) noexcept
  : bidirectional_pair_iterator(std::move(i))
  {}

 public:
  constexpr bidirectional_pair_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_pair_iterator(Iterator&& iter);

  bidirectional_pair_iterator(const bidirectional_pair_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_pair_iterator(bidirectional_pair_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  bidirectional_pair_iterator(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other);
  bidirectional_pair_iterator(bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>&& other) noexcept;

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
      >>
  bidirectional_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : bidirectional_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
      >>
  bidirectional_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : bidirectional_pair_iterator(std::move(other.iter_))
  {}

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename XKey, typename XMapped>
  auto operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;
  template<typename XKey, typename XMapped>
  auto operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;

  auto operator=(const bidirectional_pair_iterator& other) -> bidirectional_pair_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_pair_iterator&& other) noexcept -> bidirectional_pair_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> bidirectional_pair_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> bidirectional_pair_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator--() -> bidirectional_pair_iterator& {
    --*iter_ptr_;
    return *this;
  }

  auto operator--(int) -> bidirectional_pair_iterator {
    auto copy = *this;
    --*this;
    return copy;
  }

  auto operator*() const -> reference {
    return iter_ptr_->cderef();
  }

  auto operator->() const -> pointer {
    auto dereferenced = iter_ptr_->cderef();
    return pointer(::std::move(dereferenced.first), ::std::move(dereferenced.second));
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>> {
  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<java::lang::Object, java::lang::Object>;
  using pointer = pair_proxy<java::lang::Object, java::lang::Object>;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  class interface
  : public virtual forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::interface,
    public virtual bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface
  {
   public:
    ///\brief Destructor.
    virtual ~interface() noexcept = 0;

    /**
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    auto copy() const
    -> interface* override = 0;
  };

 private:
  template<typename Iterator>
  class impl;

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<value_type::first_type>> && std::is_const_v<type_of_t<value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  bidirectional_pair_iterator([[maybe_unused]] _cast c, const bidirectional_pair_iterator<Key, Mapped>& i)
  : bidirectional_pair_iterator(i)
  {}

  template<typename Key, typename Mapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<value_type::first_type>> && std::is_const_v<type_of_t<value_type::second_type>>)
      || (!std::is_const_v<Key> && !std::is_const_v<Mapped>)
      >>
  bidirectional_pair_iterator([[maybe_unused]] _cast c, bidirectional_pair_iterator<Key, Mapped>&& i) noexcept
  : bidirectional_pair_iterator(std::move(i))
  {}

 public:
  constexpr bidirectional_pair_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_pair_iterator(Iterator&& iter);

  bidirectional_pair_iterator(const bidirectional_pair_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_pair_iterator(bidirectional_pair_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
          && !std::is_const_v<OtherKey>
          && !std::is_const_v<OtherMapped>>>
  bidirectional_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : bidirectional_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              type_of_t<java::lang::Object>,
              std::remove_const_t<OtherMapped>>
          && !std::is_const_v<OtherKey>
          && !std::is_const_v<OtherMapped>>>
  bidirectional_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : bidirectional_pair_iterator(std::move(other.iter_))
  {}

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool;
  auto operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool;

  template<typename XKey, typename XMapped>
  auto operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;
  template<typename XKey, typename XMapped>
  auto operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool;

  auto operator=(const bidirectional_pair_iterator& other) -> bidirectional_pair_iterator& {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_pair_iterator&& other) noexcept -> bidirectional_pair_iterator& {
    iter_ptr_ = std::move(other.iter_ptr_);
    return *this;
  }

  auto operator++() -> bidirectional_pair_iterator& {
    ++*iter_ptr_;
    return *this;
  }

  auto operator++(int) -> bidirectional_pair_iterator {
    auto copy = *this;
    ++*this;
    return copy;
  }

  auto operator--() -> bidirectional_pair_iterator& {
    --*iter_ptr_;
    return *this;
  }

  auto operator--(int) -> bidirectional_pair_iterator {
    auto copy = *this;
    --*this;
    return copy;
  }

  auto operator*() const -> reference {
    return iter_ptr_->deref();
  }

  auto operator->() const -> pointer {
    auto dereferenced = iter_ptr_->deref();
    return pointer(::std::move(dereferenced.first), ::std::move(dereferenced.second));
  }

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<typename Key, typename Mapped>
class forward_pair_iterator {
  static_assert(::std::is_const_v<Key> == ::std::is_const_v<Mapped>,
      "Constness of Key and Mapped type must match.");

  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<var_ref<Key>, var_ref<Mapped>>;
  using pointer = pair_proxy<var_ref<Key>, var_ref<Mapped>>;
  using reference = value_type;
  using iterator_category = std::forward_iterator_tag;

  constexpr forward_pair_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              value_type>
          && !java::type_traits::is_iterator_v<Iterator>>>
  forward_pair_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Key>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit forward_pair_iterator(const forward_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>>& iter)
  : iter_(iter)
  {}

  explicit forward_pair_iterator(forward_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>>&& iter) noexcept
  : iter_(std::move(iter))
  {}

  template<typename OtherKey, typename OtherMapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>)
      >>
  forward_pair_iterator(_cast c, const forward_pair_iterator<OtherKey, OtherMapped>& i)
  : iter_(c, i)
  {}

  template<typename OtherKey, typename OtherMapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>)
      >>
  forward_pair_iterator(_cast c, forward_pair_iterator<OtherKey, OtherMapped>&& i) noexcept
  : iter_(c, std::move(i))
  {}

 public:
  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Key>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              std::remove_const_t<Mapped>,
              std::remove_const_t<OtherMapped>>
          && ((std::is_const_v<Key> && std::is_const_v<Mapped>)
              || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>))
      >>
  forward_pair_iterator(const forward_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Key>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              std::remove_const_t<Mapped>,
              std::remove_const_t<OtherMapped>>
          && ((std::is_const_v<Key> && std::is_const_v<Mapped>)
              || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>))
      >>
  forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(other))
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<std::is_convertible_v<forward_pair_iterator<OtherKey, OtherMapped>, forward_pair_iterator>>>
  forward_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : forward_pair_iterator(forward_pair_iterator<OtherKey, OtherMapped>(std::move(other)))
  {}

  template<typename Iterator>
  auto operator==(const Iterator& other) const noexcept
  -> std::enable_if_t<::java::type_traits::is_iterator_v<Iterator>, bool> {
    return iter_ == other;
  }

  auto operator++() -> forward_pair_iterator& {
    ++iter_;
    return *this;
  }

  auto operator++(int) -> forward_pair_iterator {
    return forward_pair_iterator(iter_++);
  }

  auto operator*() const -> reference {
    return ::std::make_pair(cast<var_ref<Key>>(::std::move(iter_->first)), cast<var_ref<Mapped>>(::std::move(iter_->second)));
  }

  auto operator->() const -> pointer {
    return pointer(cast<var_ref<Key>>(::std::move(iter_->first)), cast<var_ref<Mapped>>(::std::move(iter_->second)));
  }

 private:
  forward_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>> iter_;
};


template<typename Key, typename Mapped>
class bidirectional_pair_iterator {
  static_assert(::std::is_const_v<Key> == ::std::is_const_v<Mapped>,
      "Constness of Key and Mapped type must match.");

  template<typename R, typename X>
  friend auto cast(X&& x)
  -> std::enable_if_t<
      ::java::type_traits::is_iterator_v<std::remove_cv_t<std::remove_reference_t<X>>>
      && ::java::type_traits::is_pair_iterator_v<R>
      && ((std::is_const_v<type_of_t<typename R::value_type::first_type>>
              && std::is_const_v<type_of_t<typename R::value_type::second_type>>)
          || (!std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::first_type>>
              && !std::is_const_v<type_of_t<typename std::remove_cv_t<std::remove_reference_t<X>>::value_type::second_type>>))
      && std::is_convertible_v<
          typename retype_iterator_<::std::pair<type_of_t<typename R::value_type::first_type>, type_of_t<typename R::value_type::second_type>>, std::remove_cv_t<std::remove_reference_t<X>>>::type,
          R>,
      R>;

  template<typename, typename> friend class ::java::forward_pair_iterator;
  template<typename, typename> friend class ::java::bidirectional_pair_iterator;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = ::std::pair<var_ref<Key>, var_ref<Mapped>>;
  using pointer = pair_proxy<var_ref<Key>, var_ref<Mapped>>;
  using reference = value_type;
  using iterator_category = std::bidirectional_iterator_tag;

  constexpr bidirectional_pair_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              value_type>
          && !java::type_traits::is_iterator_v<Iterator>>>
  bidirectional_pair_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Key>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit bidirectional_pair_iterator(const bidirectional_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>>& iter)
  : iter_(iter)
  {}

  explicit bidirectional_pair_iterator(bidirectional_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>>&& iter) noexcept
  : iter_(std::move(iter))
  {}

  template<typename OtherKey, typename OtherMapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>)
      >>
  bidirectional_pair_iterator(_cast c, const bidirectional_pair_iterator<OtherKey, OtherMapped>& i)
  : iter_(c, i)
  {}

  template<typename OtherKey, typename OtherMapped, typename = std::enable_if_t<
      (std::is_const_v<type_of_t<typename value_type::first_type>> && std::is_const_v<type_of_t<typename value_type::second_type>>)
      || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>)
      >>
  bidirectional_pair_iterator(_cast c, bidirectional_pair_iterator<OtherKey, OtherMapped>&& i) noexcept
  : iter_(c, std::move(i))
  {}

 public:
  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Key>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              std::remove_const_t<Mapped>,
              std::remove_const_t<OtherMapped>>
          && ((std::is_const_v<Key> && std::is_const_v<Mapped>)
              || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>))
      >>
  bidirectional_pair_iterator(const bidirectional_pair_iterator<OtherKey, OtherMapped>& other)
  : bidirectional_pair_iterator(other.iter_)
  {}

  template<typename OtherKey, typename OtherMapped, typename =
      std::enable_if_t<
          java::type_traits::is_assignable_v<
              std::remove_const_t<Key>,
              std::remove_const_t<OtherKey>>
          && java::type_traits::is_assignable_v<
              std::remove_const_t<Mapped>,
              std::remove_const_t<OtherMapped>>
          && ((std::is_const_v<Key> && std::is_const_v<Mapped>)
              || (!std::is_const_v<OtherKey> && !std::is_const_v<OtherMapped>))
      >>
  bidirectional_pair_iterator(bidirectional_pair_iterator<OtherKey, OtherMapped>&& other) noexcept
  : bidirectional_pair_iterator(other.iter_)
  {}

  template<typename Iterator>
  auto operator==(const Iterator& other) const noexcept
  -> std::enable_if_t<::java::type_traits::is_iterator_v<Iterator>, bool> {
    return iter_ == other;
  }

  auto operator++() -> bidirectional_pair_iterator& {
    ++iter_;
    return *this;
  }

  auto operator++(int) -> bidirectional_pair_iterator {
    return bidirectional_pair_iterator(iter_++);
  }

  auto operator--() -> bidirectional_pair_iterator& {
    --iter_;
    return *this;
  }

  auto operator--(int) -> bidirectional_pair_iterator {
    return bidirectional_pair_iterator(iter_--);
  }

  auto operator*() const -> reference {
    return ::std::make_pair(cast<var_ref<Key>>(::std::move(iter_->first)), cast<var_ref<Mapped>>(::std::move(iter_->second)));
  }

  auto operator->() const -> pointer {
    return pointer(cast<var_ref<Key>>(::std::move(iter_->first)), cast<var_ref<Mapped>>(::std::move(iter_->second)));
  }

 private:
  bidirectional_pair_iterator<type_of_t<obj_type>, type_of_t<obj_type>> iter_;
};


template<typename Iterator>
class forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto deref() const -> ::std::pair<java::lang::Object, java::lang::Object> override {
    return *iter_;
  }

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto cderef() const -> ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>> override {
    return *iter_;
  }

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto operator--() -> void override {
    --iter_;
  }

  auto deref() const -> ::std::pair<java::lang::Object, java::lang::Object> override {
    return *iter_;
  }

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator>
class bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::impl final
: public interface
{
 public:
  explicit impl(Iterator iter)
  : iter_(iter)
  {}

  auto operator++() -> void override {
    ++iter_;
  }

  auto operator--() -> void override {
    --iter_;
  }

  auto cderef() const -> ::std::pair<const_ref<java::lang::Object>, const_ref<java::lang::Object>> override {
    return *iter_;
  }

  auto operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::interface* y) const noexcept
  -> bool override {
    const impl* yy = dynamic_cast<const impl*>(y);
    return yy != nullptr && iter_ == yy->iter_;
  }

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Iterator, typename>
forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::forward_pair_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::forward_pair_iterator(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other)
: iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
{}

inline forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::forward_pair_iterator(forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>&& other) noexcept
: iter_ptr_(std::move(other.iter_ptr_))
{}

inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename XKey, typename XMapped>
inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename XKey, typename XMapped>
inline auto forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::forward_pair_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename XKey, typename XMapped>
inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename XKey, typename XMapped>
inline auto forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::bidirectional_pair_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::bidirectional_pair_iterator(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other)
: iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
{}

inline bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::bidirectional_pair_iterator(bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>&& other) noexcept
: iter_ptr_(std::move(other.iter_ptr_))
{}

inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename XKey, typename XMapped>
inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename XKey, typename XMapped>
inline auto bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>::operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename Iterator, typename>
bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::bidirectional_pair_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}

inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<type_of_t<const_ref<java::lang::Object>>, type_of_t<const_ref<java::lang::Object>>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>& other) const noexcept -> bool {
  if (iter_ptr_ == nullptr) return other.iter_ptr_ == nullptr;
  return *iter_ptr_ == other.iter_ptr_.get();
}

template<typename XKey, typename XMapped>
inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const forward_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}

template<typename XKey, typename XMapped>
inline auto bidirectional_pair_iterator<type_of_t<java::lang::Object>, type_of_t<java::lang::Object>>::operator==(const bidirectional_pair_iterator<XKey, XMapped>& other) const noexcept -> bool {
  return *this == other.iter_;
}


template<typename X1, typename X2, typename Y1, typename Y2>
inline auto operator!=(const forward_pair_iterator<X1, X2>& x, const forward_pair_iterator<Y1, Y2>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X1, typename X2, typename Y1, typename Y2>
inline auto operator!=(const forward_pair_iterator<X1, X2>& x, const bidirectional_pair_iterator<Y1, Y2>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X1, typename X2, typename Y1, typename Y2>
inline auto operator!=(const bidirectional_pair_iterator<X1, X2>& x, const forward_pair_iterator<Y1, Y2>& y) noexcept -> bool {
  return !(x == y);
}

template<typename X1, typename X2, typename Y1, typename Y2>
inline auto operator!=(const bidirectional_pair_iterator<X1, X2>& x, const bidirectional_pair_iterator<Y1, Y2>& y) noexcept -> bool {
  return !(x == y);
}


} /* namespace java */

#include <java/lang/Object.h>

#endif /* JAVA_ITERATOR_H */
