#include <java/array.h>
#include <java/serialization/encdec.h>
#include <java/serialization/type_def.h>
#include <algorithm>
#include <codecvt>
#include <iterator>
#include <locale>
#include <type_traits>
#include <java/reflect.h>
#include <java/hash.h>
#include <java/lang/Class.h>
#include <java/lang/Object.h>
#include <java/lang/Boolean.h>
#include <java/lang/Byte.h>
#include <java/lang/Short.h>
#include <java/lang/Integer.h>
#include <java/lang/Long.h>
#include <java/lang/Float.h>
#include <java/lang/Double.h>
#include <java/lang/Character.h>
#include <java/io/Serializable.h>

namespace java::_erased::java {
namespace {

template<typename Iter, typename Obj>
class wrapped_primitive_iter
: public Iter
{
 public:
  using value_type = Obj;
  using reference = std::conditional_t<
      ::std::is_const_v<::std::remove_reference_t<typename ::std::iterator_traits<Iter>::reference>>,
      ::java::const_ref<Obj>,
      Obj>;

 public:
  using Iter::Iter;
  using Iter::operator=;

  template<typename IterArg>
  explicit wrapped_primitive_iter(IterArg iter)
  : Iter(std::forward<IterArg>(iter))
  {}

  auto operator*() const -> reference {
    return value_type(::java::allocate, this->Iter::operator*());
  }
};

template<typename Iter>
class wrapped_arrayintf_iter
: public Iter
{
 public:
  using value_type = ::java::lang::Object;
  using reference = std::conditional_t<
      ::std::is_const_v<::std::remove_reference_t<typename ::std::iterator_traits<Iter>::reference>>,
      ::java::const_ref<value_type>,
      value_type>;

 public:
  using Iter::Iter;
  using Iter::operator=;

  template<typename IterArg>
  explicit wrapped_arrayintf_iter(IterArg iter)
  : Iter(std::forward<IterArg>(iter))
  {}

  auto operator*() const -> reference {
    using erased_ref = std::conditional_t<
        ::std::is_const_v<::std::remove_reference_t<typename ::std::iterator_traits<Iter>::reference>>,
        const ::java::_erased::java::lang::Object,
        ::java::_erased::java::lang::Object>;

    return reference(
        ::java::_direct(),
        ::cycle_ptr::cycle_gptr<erased_ref>(this->Iter::operator*()));
  }
};

} /* namespace java::_erased::java::<unnamed> */


array_intf::~array_intf() noexcept = default;

auto array_intf::begin() const
-> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return begin_();
}

auto array_intf::end() const
-> bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return end_();
}

auto array_intf::begin()
-> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return begin_();
}

auto array_intf::end()
-> bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return end_();
}


array<::java::boolean_t>::~array() noexcept = default;

auto array<::java::boolean_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::boolean_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::boolean_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::boolean_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::boolean_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::boolean_t>();
}

auto array<::java::boolean_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::boolean_t>();
}

auto array<::java::boolean_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::boolean_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::boolean_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::boolean_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::boolean_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::boolean_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::bool_array>(data_);
      });
}

auto array<::java::boolean_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (bool i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::boolean_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::boolean_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::boolean_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Boolean>(begin());
}

auto array<::java::boolean_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Boolean>(end());
}

auto array<::java::boolean_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Boolean>(begin());
}

auto array<::java::boolean_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Boolean>(end());
}


array<::java::byte_t>::~array() noexcept = default;

auto array<::java::byte_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::byte_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::byte_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::byte_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::byte_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::byte_t>();
}

auto array<::java::byte_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::byte_t>();
}

auto array<::java::byte_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::byte_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::byte_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::byte_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::byte_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::byte_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::byte_array>(data_);
      });
}

auto array<::java::byte_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::byte_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::byte_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::byte_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Byte>(begin());
}

auto array<::java::byte_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Byte>(end());
}

auto array<::java::byte_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Byte>(begin());
}

auto array<::java::byte_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Byte>(end());
}


array<::java::short_t>::~array() noexcept = default;

auto array<::java::short_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::short_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::short_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::short_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::short_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::short_t>();
}

auto array<::java::short_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::short_t>();
}

auto array<::java::short_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::short_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::short_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::short_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::short_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::short_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::short_array>(data_);
      });
}

auto array<::java::short_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::short_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::short_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::short_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Short>(begin());
}

auto array<::java::short_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Short>(end());
}

auto array<::java::short_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Short>(begin());
}

auto array<::java::short_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Short>(end());
}


array<::java::int_t>::~array() noexcept = default;

auto array<::java::int_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::int_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::int_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::int_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::int_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::int_t>();
}

auto array<::java::int_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::int_t>();
}

auto array<::java::int_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::int_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::int_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::int_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::int_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::int_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::int_array>(data_);
      });
}

auto array<::java::int_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::int_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::int_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::int_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Integer>(begin());
}

auto array<::java::int_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Integer>(end());
}

auto array<::java::int_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Integer>(begin());
}

auto array<::java::int_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Integer>(end());
}


array<::java::long_t>::~array() noexcept = default;

auto array<::java::long_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::long_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::long_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::long_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::long_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::long_t>();
}

auto array<::java::long_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::long_t>();
}

auto array<::java::long_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::long_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::long_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::long_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::long_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::long_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::long_array>(data_);
      });
}

auto array<::java::long_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::long_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::long_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::long_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Long>(begin());
}

auto array<::java::long_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Long>(end());
}

auto array<::java::long_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Long>(begin());
}

auto array<::java::long_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Long>(end());
}


array<::java::float_t>::~array() noexcept = default;

auto array<::java::float_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::float_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::float_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::float_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::float_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::float_t>();
}

auto array<::java::float_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::float_t>();
}

auto array<::java::float_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::float_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::float_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::float_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::float_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::float_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::float_array>(data_);
      });
}

auto array<::java::float_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::float_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::float_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::float_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Float>(begin());
}

auto array<::java::float_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Float>(end());
}

auto array<::java::float_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Float>(begin());
}

auto array<::java::float_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Float>(end());
}


array<::java::double_t>::~array() noexcept = default;

auto array<::java::double_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::double_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::double_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::double_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::double_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::double_t>();
}

auto array<::java::double_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::double_t>();
}

auto array<::java::double_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::double_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::double_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::double_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::double_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::double_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::double_array>(data_);
      });
}

auto array<::java::double_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::double_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::double_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::double_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Double>(begin());
}

auto array<::java::double_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Double>(end());
}

auto array<::java::double_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Double>(begin());
}

auto array<::java::double_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Double>(end());
}


array<::java::char_t>::~array() noexcept = default;

auto array<::java::char_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::char_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::char_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::char_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(dimensions());
}

auto array<::java::char_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::char_t>();
}

auto array<::java::char_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::char_t>();
}

auto array<::java::char_t>::__class__(std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;

  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(field_descriptor(primitive_type::char_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, dimensions);
      });
}

auto array<::java::char_t>::__serializable_array_class__(::java::serialization::cycle_handler& handler, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

  return handler.encode_class_desc(
      to_string(field_descriptor(primitive_type::char_type, dimensions)),
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::char_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::char_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::char_array>(data_);
      });
}

auto array<::java::char_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : *this) hc << i;
  return std::move(hc);
}

auto array<::java::char_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  eq(x.data_, y.data_);
}

auto array<::java::char_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::char_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Character>(begin());
}

auto array<::java::char_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_primitive_iter<vector_type::const_iterator, ::java::lang::Character>(end());
}

auto array<::java::char_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Character>(begin());
}

auto array<::java::char_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_primitive_iter<vector_type::iterator, ::java::lang::Character>(end());
}


array<::java::lang::Object>::array(::java::lang::Class<::java::G::pack<>> element_type)
: element_type_(std::move(element_type))
{
  if (!element_type_)
    throw std::invalid_argument("null class for array");
}

array<::java::lang::Object>::~array() noexcept = default;

auto array<::java::lang::Object>::empty_() const noexcept -> bool {
  return empty();
}

auto array<::java::lang::Object>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::lang::Object>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::lang::Object>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __class__(element_type_, dimensions());
}

auto array<::java::lang::Object>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return element_type_;
}

auto array<::java::lang::Object>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return element_type_;
}

auto array<::java::lang::Object>::__class__(::java::lang::Class<::java::G::pack<>> element_type, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  conversion_type conversion;
  const auto fd = field_descriptor(conversion.from_bytes(element_type->name()), dimensions);
  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(fd),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [element_type, dimensions](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, element_type, dimensions);
      });
}

auto array<::java::lang::Object>::__serializable_array_class__(::java::serialization::cycle_handler& handler, ::java::lang::Class<::java::G::pack<>> element_type, std::size_t dimensions)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  conversion_type conversion;
  const auto fd = field_descriptor(conversion.from_bytes(element_type->name()), dimensions);
  return handler.encode_class_desc(
      to_string(fd),
      [&fd]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            fd,
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
}

auto array<::java::lang::Object>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, element_type_, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(element);
            });
      });
}

auto array<::java::lang::Object>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array<::java::lang::Object>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array& x, const array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array<::java::lang::Object>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array* casted_other = dynamic_cast<const array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array<::java::lang::Object>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return data_.begin();
}

auto array<::java::lang::Object>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return data_.end();
}

auto array<::java::lang::Object>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return data_.begin();
}

auto array<::java::lang::Object>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return data_.end();
}


array_of_array<::java::boolean_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::boolean_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::boolean_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::boolean_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::boolean_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::boolean_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::boolean_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::boolean_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::boolean_t>();
}

auto array_of_array<::java::boolean_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::boolean_t>::__class__(dim_);
}

auto array_of_array<::java::boolean_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::boolean_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::boolean_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::boolean_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::boolean_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::boolean_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::boolean_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::boolean_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::boolean_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::byte_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::byte_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::byte_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::byte_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::byte_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::byte_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::byte_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::byte_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::byte_t>();
}

auto array_of_array<::java::byte_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::byte_t>::__class__(dim_);
}

auto array_of_array<::java::byte_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::byte_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::byte_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::byte_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::byte_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::byte_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::byte_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::byte_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::byte_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::short_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::short_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::short_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::short_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::short_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::short_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::short_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::short_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::short_t>();
}

auto array_of_array<::java::short_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::short_t>::__class__(dim_);
}

auto array_of_array<::java::short_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::short_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::short_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::short_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::short_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::short_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::short_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::short_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::short_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::int_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::int_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::int_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::int_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::int_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::int_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::int_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::int_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::int_t>();
}

auto array_of_array<::java::int_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::int_t>::__class__(dim_);
}

auto array_of_array<::java::int_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::int_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::int_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::int_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::int_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::int_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::int_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::int_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::int_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::long_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::long_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::long_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::long_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::long_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::long_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::long_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::long_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::long_t>();
}

auto array_of_array<::java::long_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::long_t>::__class__(dim_);
}

auto array_of_array<::java::long_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::long_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::long_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::long_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::long_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::long_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::long_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::long_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::long_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::float_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::float_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::float_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::float_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::float_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::float_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::float_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::float_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::float_t>();
}

auto array_of_array<::java::float_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::float_t>::__class__(dim_);
}

auto array_of_array<::java::float_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::float_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::float_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::float_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::float_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::float_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::float_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::float_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::float_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::double_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::double_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::double_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::double_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::double_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::double_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::double_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::double_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::double_t>();
}

auto array_of_array<::java::double_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::double_t>::__class__(dim_);
}

auto array_of_array<::java::double_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::double_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::double_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::double_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::double_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::double_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::double_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::double_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::double_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::char_t>::array_of_array(std::size_t dim)
: dim_(dim)
{
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::char_t>::~array_of_array() noexcept = default;

auto array_of_array<::java::char_t>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::char_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::char_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::char_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::char_t>::__class__(dim_ - 1u));
}

auto array_of_array<::java::char_t>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::char_t>();
}

auto array_of_array<::java::char_t>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::char_t>::__class__(dim_);
}

auto array_of_array<::java::char_t>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::char_t>::__serializable_array_class__(handler, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::char_t>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::char_t>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::char_t>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::char_t>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::char_t>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::char_t>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::char_t>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


array_of_array<::java::lang::Object>::array_of_array(::java::lang::Class<::java::G::pack<>> element_type, std::size_t dim)
: element_type_(std::move(element_type)),
  dim_(dim)
{
  if (!element_type_)
    throw std::invalid_argument("null class for array");
  if (dim_ < 2)
    throw std::invalid_argument("insufficient dimensions (min 2) for array of array");
}

array_of_array<::java::lang::Object>::~array_of_array() noexcept = default;

auto array_of_array<::java::lang::Object>::empty_() const noexcept -> bool {
  return empty();
}

auto array_of_array<::java::lang::Object>::size_() const noexcept -> std::size_t {
  return size();
}

auto array_of_array<::java::lang::Object>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array_of_array<::java::lang::Object>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::_direct(),
      array<::java::lang::Object>::__class__(element_type_, dim_ - 1u));
}

auto array_of_array<::java::lang::Object>::leaf_element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return element_type_;
}

auto array_of_array<::java::lang::Object>::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return array<::java::lang::Object>::__class__(element_type_, dim_);
}

auto array_of_array<::java::lang::Object>::__encoder__(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = array<::java::lang::Object>::__serializable_array_class__(handler, element_type_, dimensions());
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(
                  ::java::io::Serializable(
                      ::java::_direct(),
                      cycle_ptr::cycle_gptr<::java::_erased::java::io::Serializable>(element)));
            });
      });
}

auto array_of_array<::java::lang::Object>::__hash_code__(bool specialized, ::std::size_t max_cascade) const noexcept
-> std::size_t {
  static const ::std::size_t nonce = ::java::__hash_nonce();

  specialized = true;
  auto hc = ::java::hash_combiner(this->::java::_erased::java::lang::Object::__hash_code__(specialized, max_cascade), max_cascade)
      << nonce;
  for (const auto& i : data_) hc << i;
  return std::move(hc);
}

auto array_of_array<::java::lang::Object>::__equal_impl__(bool specialized, ::java::_equal_helper& eq, const array_of_array& x, const array_of_array& y)
-> void {
  specialized = true;
  ::java::_erased::java::lang::Object::__equal_impl__(specialized, eq, x, y);

  if (!std::equal(
          x.data_.begin(), x.data_.end(),
          y.data_.begin(), y.data_.end(),
          [&eq](const auto& x, const auto& y) {
            return eq(x, y).ok(); // By returning the `ok()` state, we can bail out early.
          }))
    eq.fail();
}

auto array_of_array<::java::lang::Object>::__equal__(bool specialized, ::java::_equal_helper& eq, const ::java::object_intf& other) const
-> void {
  const array_of_array* casted_other = dynamic_cast<const array_of_array*>(&other);
  if (casted_other == nullptr)
    eq.fail();
  else
    __equal_impl__(specialized, eq, *this, *casted_other);
}

auto array_of_array<::java::lang::Object>::begin_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.begin());
}

auto array_of_array<::java::lang::Object>::end_() const
-> ::java::bidirectional_iterator<::java::type_of_t<::java::const_ref<::java::lang::Object>>> {
  return wrapped_arrayintf_iter<vector_type::const_iterator>(data_.end());
}

auto array_of_array<::java::lang::Object>::begin_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.begin());
}

auto array_of_array<::java::lang::Object>::end_()
-> ::java::bidirectional_iterator<::java::type_of_t<::java::lang::Object>> {
  return wrapped_arrayintf_iter<vector_type::iterator>(data_.end());
}


} /* namespace java::_erased::java */
