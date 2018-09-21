#ifndef JAVA_ITERATOR_H
#define JAVA_ITERATOR_H

#include <memory>
#include <type_traits>
#include <java/lang/Object.h>
#include <java/ref.h>

namespace java {


template<typename Type>
class forward_iterator;

template<typename Type>
class bidirectional_iterator;


template<>
class forward_iterator<type_of_t<const_ref<java::lang::Object>>> {
 public:
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

  template<typename Iterator>
  explicit forward_iterator(Iterator&& iter);

  forward_iterator(const forward_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_iterator(forward_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
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

  auto operator=(forward_iterator& other) noexcept {
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

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class forward_iterator<type_of_t<java::lang::Object>> {
 public:
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
    virtual auto deref() const -> java::lang::Object = 0;

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

  template<typename Iterator>
  explicit forward_iterator(Iterator&& iter);

  forward_iterator(const forward_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  forward_iterator(forward_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
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

  auto operator=(forward_iterator& other) noexcept {
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

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>> {
 public:
  class interface
  : public forward_iterator<type_of_t<const_ref<java::lang::Object>>>::interface
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

  template<typename Iterator>
  explicit bidirectional_iterator(Iterator&& iter);

  bidirectional_iterator(const bidirectional_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_iterator(bidirectional_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
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

  auto operator=(bidirectional_iterator& other) noexcept {
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

 private:
  std::unique_ptr<interface> iter_ptr_;
};


template<>
class bidirectional_iterator<type_of_t<java::lang::Object>> {
 public:
  class interface
  : public forward_iterator<type_of_t<java::lang::Object>>::interface
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

  template<typename Iterator>
  explicit bidirectional_iterator(Iterator&& iter);

  bidirectional_iterator(const bidirectional_iterator& other)
  : iter_ptr_(other.iter_ptr_ ? other.iter_ptr_->copy() : nullptr)
  {}

  bidirectional_iterator(bidirectional_iterator&& other) noexcept
  : iter_ptr_(std::move(other.iter_ptr_))
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

  auto operator=(bidirectional_iterator& other) noexcept {
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
  constexpr forward_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              type<Type>
          >>>
  explicit forward_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Type>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit forward_iterator(forward_iterator<type_of_t<obj_type>>&& iter)
  : iter_(std::move(iter))
  {}

 public:
  auto operator++() -> forward_iterator& {
    ++iter_;
    return *this;
  }

  auto operator++(int) -> forward_iterator {
    return forward_iterator(iter_++);
  }

  auto operator*() const -> type<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

 private:
  forward_iterator<type_of_t<obj_type>> iter_;
};

template<typename Type>
class bidirectional_iterator {
 public:
  constexpr bidirectional_iterator() = default;

  template<typename Iterator,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype(*std::declval<Iterator>()),
              type<Type>
          >>>
  explicit bidirectional_iterator(Iterator&& iter)
  : iter_(std::forward<Iterator>(iter))
  {}

 private:
  using obj_type = std::conditional_t<
      std::is_const_v<Type>,
      const_ref<java::lang::Object>,
      java::lang::Object>;

  explicit bidirectional_iterator(bidirectional_iterator<type_of_t<obj_type>>&& iter)
  : iter_(std::move(iter))
  {}

 public:
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

  auto operator*() const -> type<Type> {
    return cast<var_ref<Type>>(*iter_);
  }

 private:
  bidirectional_iterator<type_of_t<obj_type>> iter_;
};


template<typename Iterator>
forward_iterator<type_of_t<java::lang::Object>>::forward_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


template<typename Iterator>
forward_iterator<type_of_t<const_ref<java::lang::Object>>>::forward_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


template<typename Iterator>
bidirectional_iterator<type_of_t<java::lang::Object>>::bidirectional_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


template<typename Iterator>
bidirectional_iterator<type_of_t<const_ref<java::lang::Object>>>::bidirectional_iterator(Iterator&& iter)
: iter_ptr_(std::make_unique<impl<std::remove_cv_t<std::remove_reference_t<Iterator>>>>(std::forward<Iterator>(iter)))
{}


} /* namespace java */

#endif /* JAVA_ITERATOR_H */
