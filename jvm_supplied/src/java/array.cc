#include <java/array.h>

namespace java::_erased::java {


array_intf::~array_intf() noexcept = default;


array<::java::boolean_t>::~array() noexcept = default;

auto array<::java::boolean_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::boolean_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::byte_t>::~array() noexcept = default;

auto array<::java::byte_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::byte_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::short_t>::~array() noexcept = default;

auto array<::java::short_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::short_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::int_t>::~array() noexcept = default;

auto array<::java::int_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::int_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::long_t>::~array() noexcept = default;

auto array<::java::long_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::long_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::float_t>::~array() noexcept = default;

auto array<::java::float_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::float_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::double_t>::~array() noexcept = default;

auto array<::java::double_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::double_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::char_t>::~array() noexcept = default;

auto array<::java::char_t>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::char_t>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


array<::java::lang::Object>::~array() noexcept = default;

auto array<::java::lang::Object>::size_() const noexcept -> std::size_t {
  return size();
}

auto array<::java::lang::Object>::dimensions_() const noexcept -> std::size_t {
  return dimensions();
}


} /* namespace java::_erased::java */
