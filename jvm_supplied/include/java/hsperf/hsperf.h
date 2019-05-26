#ifndef JAVA_HSPERF_HSPERF_H
#define JAVA_HSPERF_HSPERF_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#include <boost/endian/conversion.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace java::hsperf {


struct hsperf_prologue {
 public:
  static constexpr std::uint32_t magic = 0xcafec0c0u;
  static constexpr std::uint8_t big_endian = 0;
  static constexpr std::uint8_t little_endian = 1;
  static constexpr std::tuple<std::uint8_t, std::uint8_t> major_minor{ 2u, 0u };

  auto valid_magic() const
  noexcept
  -> bool {
    // Magic is always in big endian notation.
    return boost::endian::big_to_native(magic_) == magic;
  }

  auto valid_version() const
  noexcept
  -> bool {
    return std::tie(major_, minor_) == major_minor;
  }

  auto byte_order() const
  -> std::uint8_t {
    if (byte_order_ != big_endian && byte_order_ != little_endian)
      throw std::runtime_error("Invalid endian flag");
    return byte_order_;
  }

  auto accessible() const
  noexcept
  -> std::uint8_t {
    return accessible_;
  }

  auto validate_or_throw() const
  -> void {
    if (!valid_magic() || !valid_version())
      throw std::runtime_error("invalid prologue");
  }

  auto used() const
  -> std::uint32_t {
    switch (byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case big_endian:
      return boost::endian::big_to_native(used_);
    case little_endian:
      return boost::endian::little_to_native(used_);
    }
  }

  auto overflow() const
  -> std::uint32_t {
    switch (byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case big_endian:
      return boost::endian::big_to_native(overflow_);
    case little_endian:
      return boost::endian::little_to_native(overflow_);
    }
  }

  auto timestamp() const
  -> std::uint64_t {
    switch (byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case big_endian:
      return boost::endian::big_to_native(timestamp_);
    case little_endian:
      return boost::endian::little_to_native(timestamp_);
    }
  }

  auto entry_offset() const
  -> std::uint64_t {
    switch (byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case big_endian:
      return boost::endian::big_to_native(entry_offset_);
    case little_endian:
      return boost::endian::little_to_native(entry_offset_);
    }
  }

  auto num_entries() const
  -> std::uint64_t {
    switch (byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case big_endian:
      return boost::endian::big_to_native(num_entries_);
    case little_endian:
      return boost::endian::little_to_native(num_entries_);
    }
  }

 private:
  std::uint32_t magic_; // Always in Big Endian.
  std::uint8_t byte_order_;
  std::uint8_t major_;
  std::uint8_t minor_;
  std::uint8_t accessible_;
  std::uint32_t used_;
  std::uint32_t overflow_;
  std::uint64_t timestamp_;
  std::uint32_t entry_offset_;
  std::uint32_t num_entries_;
};

class perfdata {
 public:
  auto entry_length(const hsperf_prologue& p) const
  -> std::uint32_t {
    switch (p.byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case hsperf_prologue::big_endian:
      return boost::endian::big_to_native(entry_length_);
    case hsperf_prologue::little_endian:
      return boost::endian::little_to_native(entry_length_);
    }
  }

  auto name_offset(const hsperf_prologue& p) const
  -> std::uint32_t {
    switch (p.byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case hsperf_prologue::big_endian:
      return boost::endian::big_to_native(name_offset_);
    case hsperf_prologue::little_endian:
      return boost::endian::little_to_native(name_offset_);
    }
  }

  auto vector_len(const hsperf_prologue& p) const
  -> std::uint32_t {
    switch (p.byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case hsperf_prologue::big_endian:
      return boost::endian::big_to_native(vector_len_);
    case hsperf_prologue::little_endian:
      return boost::endian::little_to_native(vector_len_);
    }
  }

  auto data_type() const
  -> std::uint8_t {
    return data_type_;
  }

  auto flags() const
  -> std::uint8_t {
    return flags_;
  }

  auto data_unit() const
  -> std::uint8_t {
    return data_unit_;
  }

  auto data_var() const
  -> std::uint8_t {
    return data_var_;
  }

  auto data_off(const hsperf_prologue& p) const
  -> std::uint32_t {
    switch (p.byte_order()) {
    default:
      throw std::runtime_error("Invalid endian flag");
    case hsperf_prologue::big_endian:
      return boost::endian::big_to_native(data_off_);
    case hsperf_prologue::little_endian:
      return boost::endian::little_to_native(data_off_);
    }
  }

 private:
  std::uint32_t entry_length_;
  std::uint32_t name_offset_;
  std::uint32_t vector_len_;
  std::uint8_t data_type_;
  std::uint8_t flags_;
  std::uint8_t data_unit_;
  std::uint8_t data_var_;
  std::uint32_t data_off_;
};

class hsperf {
 public:
  using entry_type = std::variant<
      std::monostate, // Used for cases where we can't decode.
      bool,
      char,
      float,
      double,
      std::uint8_t,
      std::int16_t,
      std::int32_t,
      std::int64_t,
      std::vector<bool>,
      std::string, // UTF-8
      std::vector<float>,
      std::vector<double>,
      std::vector<std::uint8_t>,
      std::vector<std::int16_t>,
      std::vector<std::int32_t>,
      std::vector<std::int64_t>>;

  explicit hsperf(std::string filename);

  ~hsperf() noexcept;

  auto get() const
  -> std::unordered_map<std::string, entry_type>;

 private:
  template<typename T>
  auto read_at_offset(std::uint32_t off) const
  -> const T* {
    if (off + sizeof(T) > data_.size())
      throw std::invalid_argument("hsperf: offset out of range");

    return reinterpret_cast<const T*>(data_.data() + off);
  }

  boost::iostreams::mapped_file_source data_;
};

static_assert(sizeof(hsperf_prologue) == 32,
    "hsperf_prologue may not have gaps in its binary representation");

static_assert(sizeof(perfdata) == 20,
    "perfdata may not have gaps in its binary representation");


} /* namespace java::hsperf */

#endif /* JAVA_HSPERF_HSPERF_H */
