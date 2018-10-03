#include <java/array.h>
#include <java/serialization/encdec.h>
#include <codecvt>
#include <locale>
#include <java/reflect.h>
#include <java/lang/Class.h>
#include <java/lang/Object.h>
#include <java/io/Serializable.h>

namespace java::_erased::java {


array_intf::~array_intf() noexcept = default;

auto array_intf::__get_class__() const
-> ::cycle_ptr::cycle_gptr<::java::_erased::java::lang::Class> {
  return ::java::raw_ptr<::java::_tags::java::lang::Class>(array_class_());
}

auto array_intf::array_class_() const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  return array_class_(dimensions());
}

auto array_intf::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;
  using conversion_type = ::std::wstring_convert<
    std::codecvt_utf8_utf16<char16_t>,
    char16_t>;

  conversion_type conversion;
  const auto& element_class_name = element_class_()->name();
  const auto& utf16_element_class_name = conversion.from_bytes(
      element_class_name.data(),
      element_class_name.data() + element_class_name.size());

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(utf16_element_class_name, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [utf16_element_class_name, dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(utf16_element_class_name, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
      });
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

auto array<::java::boolean_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::boolean_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::boolean_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::byte_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::byte_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::byte_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::short_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::short_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::short_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::int_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::int_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::int_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::long_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::long_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::long_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::float_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::float_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::float_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::double_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::double_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::double_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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

auto array<::java::char_t>::array_class_(std::size_t dimensions) const
-> ::java::return_t<::java::lang::Class<::java::type<::java::G::pack<>>>> {
  using namespace ::java::serialization::stream;

  return ::java::lang::Class<::java::type<::java::G::pack<>>>(
      ::java::allocate,
      to_string(field_descriptor(primitive_type::char_type, dimensions)),
      ::java::get_class<::java::lang::Object>(),
      ::std::initializer_list<::java::lang::Class<::java::G::pack<>>>{
        ::java::get_class<::java::io::Serializable>()
      },
      [dimensions]([[maybe_unused]] ::java::serialization::cycle_handler& handler) {
        return cycle_ptr::make_cycle<new_class_desc__class_desc>(
            field_descriptor(primitive_type::char_type, dimensions),
            0, // serialVersionUID
            nullptr, // No serializable super class.
            SC_SERIALIZABLE);
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


} /* namespace java::_erased::java */
