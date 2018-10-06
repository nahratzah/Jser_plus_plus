#include <java/array.h>
#include <java/serialization/encdec.h>
#include <java/serialization/type_def.h>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <iterator>
#include <java/reflect.h>
#include <java/lang/Class.h>
#include <java/lang/Object.h>
#include <java/io/Serializable.h>

namespace java::_erased::java {


array_intf::~array_intf() noexcept = default;

auto array_intf::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return __get_class__(dimensions());
}


array<::java::boolean_t>::~array() noexcept = default;

auto array<::java::boolean_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::boolean_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::boolean_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::boolean_t>();
}

auto array<::java::boolean_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::boolean_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::bool_array>(data_);
      });
}


array<::java::byte_t>::~array() noexcept = default;

auto array<::java::byte_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::byte_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::byte_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::byte_t>();
}

auto array<::java::byte_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::byte_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::byte_array>(data_);
      });
}


array<::java::short_t>::~array() noexcept = default;

auto array<::java::short_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::short_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::short_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::short_t>();
}

auto array<::java::short_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::short_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::short_array>(data_);
      });
}


array<::java::int_t>::~array() noexcept = default;

auto array<::java::int_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::int_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::int_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::int_t>();
}

auto array<::java::int_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::int_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::int_array>(data_);
      });
}


array<::java::long_t>::~array() noexcept = default;

auto array<::java::long_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::long_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::long_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::long_t>();
}

auto array<::java::long_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::long_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::long_array>(data_);
      });
}


array<::java::float_t>::~array() noexcept = default;

auto array<::java::float_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::float_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::float_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::float_t>();
}

auto array<::java::float_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::float_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::float_array>(data_);
      });
}


array<::java::double_t>::~array() noexcept = default;

auto array<::java::double_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::double_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::double_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::double_t>();
}

auto array<::java::double_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::double_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::double_array>(data_);
      });
}


array<::java::char_t>::~array() noexcept = default;

auto array<::java::char_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::char_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::char_t>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return ::java::get_class<::java::char_t>();
}

auto array<::java::char_t>::__get_class__(std::size_t dimensions) const
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

auto array<::java::char_t>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this]([[maybe_unused]] ::java::serialization::cycle_handler& handler, new_array& obj) {
        obj.type = __serializable_array_class__(handler, dimensions());
        obj.data.emplace<new_array::char_array>(data_);
      });
}


array<::java::lang::Object>::array(::java::lang::Class<::java::G::pack<>> element_type)
: element_type_(std::move(element_type))
{
  if (!element_type_)
    throw std::invalid_argument("null class for array");
}

array<::java::lang::Object>::~array() noexcept = default;

auto array<::java::lang::Object>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::lang::Object>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}

auto array<::java::lang::Object>::element_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return element_type_;
}

auto array<::java::lang::Object>::__get_class__(std::size_t dimensions) const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  conversion_type conversion;
  const auto fd = field_descriptor(conversion.from_bytes(element_type_->name()), dimensions);
  return ::cycle_ptr::make_cycle<::java::_erased::java::lang::Class>(
      to_string(fd),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [fd](::java::serialization::cycle_handler& handler) {
        return __serializable_array_class__(handler, fd);
      });
}

auto array<::java::lang::Object>::__serializable_array_class__(::java::serialization::cycle_handler& handler, const ::java::serialization::stream::field_descriptor& fd)
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::new_class_desc__class_desc> {
  using namespace ::java::serialization::stream;

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

auto array<::java::lang::Object>::do_encode_(::java::serialization::cycle_handler& handler_) const
-> ::cycle_ptr::cycle_gptr<const ::java::serialization::stream::stream_element> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  return handler_.template encode_obj<new_array>(
      this->shared_from_this(this),
      [this](::java::serialization::cycle_handler& handler, new_array& obj) {
        conversion_type conversion;
        const auto fd = field_descriptor(conversion.from_bytes(element_type_->name()), dimensions());

        obj.type = __serializable_array_class__(handler, fd);
        std::transform(
            data_.begin(), data_.end(),
            ::std::back_inserter(obj.data.emplace<new_array::object_array>(new_array::object_array::allocator_type(obj))),
            [&handler](const auto& element) {
              return handler.encode_field(element);
            });
      });
}


} /* namespace java::_erased::java */
