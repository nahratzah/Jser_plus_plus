#ifndef JAVA_ITERATOR_H
#define JAVA_ITERATOR_H

#include <memory>
#include <type_traits>
#include <java/lang/Object.h>
#include <java/ref.h>

namespace java::type_traits {
namespace {

template<typename>
struct is_iterator_
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


} /* namespace java::type_traits */

namespace java {


template<typename Type>
class forward_iterator;

template<typename Type>
class bidirectional_iterator;


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

} /* namespace java::type_traits::<unnamed> */
} /* namespace java::type_traits */

namespace java {


template<>
class forward_iterator<type_of_t<const_ref<java::lang::Object>>> {
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
     * \brief Create a copy of this iterator.
     * \returns Copy of this.
     */
    [[nodiscard]]
    virtual auto copy() const
    -> interface* = 0;
  };

  template<typename Iterator>
  class impl;

  constexpr forward_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  explicit forward_iterator(Iterator&& iter);

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

  auto operator=(const forward_iterator& other) {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_iterator&& other) noexcept {
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
  friend forward_iterator<type_of_t<const_ref<java::lang::Object>>>;

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

  template<typename Iterator>
  class impl;

  constexpr forward_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  explicit forward_iterator(Iterator&& iter);

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

  auto operator=(const forward_iterator& other) {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(forward_iterator&& other) noexcept {
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

  template<typename Iterator>
  class impl;

  constexpr bidirectional_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  explicit bidirectional_iterator(Iterator&& iter);

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

  auto operator=(const bidirectional_iterator& other) {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_iterator&& other) noexcept {
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
  friend bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>;

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

  template<typename Iterator>
  class impl;

  constexpr bidirectional_iterator() noexcept = default;

  template<typename Iterator, typename = std::enable_if_t<!::java::type_traits::is_iterator_v<Iterator>>>
  explicit bidirectional_iterator(Iterator&& iter);

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

  auto operator=(const bidirectional_iterator& other) {
    if (!other.iter_ptr_) {
      iter_ptr_ = nullptr;
    } else {
      auto new_iter_ptr = std::unique_ptr<interface>(other.iter_ptr_->copy());
      iter_ptr_ = std::move(new_iter_ptr);
    }
    return *this;
  }

  auto operator=(bidirectional_iterator&& other) noexcept {
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

  [[nodiscard]]
  auto copy() const -> impl* override {
    return new impl(*this);
  }

 private:
  Iterator iter_;
};


template<typename Type>
class forward_iterator {
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
  explicit forward_iterator(Iterator&& iter)
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
  explicit bidirectional_iterator(Iterator&& iter)
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


template<typename Iterator, typename>
forward_iterator<type_of_t<java::lang::Object>>::forward_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


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


template<typename Iterator, typename>
bidirectional_iterator<type_of_t<java::lang::Object>>::bidirectional_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


} /* namespace java */

#endif /* JAVA_ITERATOR_H */
