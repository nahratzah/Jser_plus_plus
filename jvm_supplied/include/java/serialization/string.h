#ifndef JAVA_SERIALIZATION_STRING_H
#define JAVA_SERIALIZATION_STRING_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <boost/asio/buffer.hpp>

namespace java::serialization {


auto compute_utf_data_size(std::u16string_view s) noexcept -> std::size_t;
auto compute_utf_size(std::u16string_view s) noexcept -> std::size_t;
auto write_utf_data(std::u16string_view s, std::vector<std::uint8_t>& buffer) -> std::vector<std::uint8_t>::size_type;
auto write_utf(std::u16string_view s) -> std::vector<std::uint8_t>;
auto write_long_utf(std::u16string_view s) -> std::vector<std::uint8_t>;

auto read_utf_data(boost::asio::const_buffer buf) -> std::u16string;
auto read_utf_len(boost::asio::const_buffer& buf) -> std::uint16_t;
auto read_utf(boost::asio::const_buffer& buf) -> std::u16string;
auto read_long_utf_len(boost::asio::const_buffer& buf) -> std::uint64_t;
auto read_long_utf(boost::asio::const_buffer& buf) -> std::u16string;

template<typename OutIter>
auto read_utf_data(boost::asio::const_buffer buf, OutIter out)
-> OutIter {
  while (buf.size() > 0) {
    const auto len = buf.size();
    const std::uint8_t b0 = static_cast<const std::uint8_t*>(buf.data())[0];
    if (b0 == 0u) { // Null character is encoded in two bytes.
      if (len < 2)
        throw std::invalid_argument("java::serialization: buffer too short");
      if (static_cast<const std::uint8_t*>(buf.data())[1] != 0u)
        throw std::invalid_argument("java::serialization: incorrect null character encoding");

      *out = char16_t(0);
      buf = buf + 2;
    } else if ((b0 & ~0x7fu) == 0x00u) { // Single byte.
      *out = static_cast<char16_t>(b0);
      buf = buf + 1;
    } else if ((b0 & ~0x1fu) == 0xc0u) { // Two bytes.
      if (len < 2)
        throw std::invalid_argument("java::serialization: buffer too short");

      // Second encoded byte.
      const std::uint8_t b1 = static_cast<const std::uint8_t*>(buf.data())[1];
      if ((b1 & ~0x3fu) != 0x80u)
        throw std::invalid_argument("java::serialization: incorrect encoding");

      std::uint16_t val = (static_cast<std::uint16_t>(b0) & 0x1fu) << 6
          | (b1 & 0x3fu);
      *out = static_cast<char16_t>(val);
      buf = buf + 2;
    } else if ((b0 & ~0x1fu) == 0xe0u) { // Three bytes.
      if (len < 3)
        throw std::invalid_argument("java::serialization: buffer too short");

      // Second encoded byte.
      const std::uint8_t b1 = static_cast<const std::uint8_t*>(buf.data())[1];
      if ((b1 & ~0x3fu) != 0x80u)
        throw std::invalid_argument("java::serialization: incorrect encoding");

      // Third encoded byte.
      const std::uint8_t b2 = static_cast<const std::uint8_t*>(buf.data())[2];
      if ((b2 & ~0x3fu) != 0x80u)
        throw std::invalid_argument("java::serialization: incorrect encoding");

      std::uint16_t val =
          (static_cast<std::uint16_t>(b0) & 0x1fu) << 12
          | (static_cast<std::uint16_t>(b1) & 0x3fu) << 6
          | (b2 & 0x3fu);
      *out = static_cast<char16_t>(val);
      buf = buf + 3;
    } else { // Unrecognized bit pattern.
      throw std::invalid_argument("java::serialization: incorrect encoding");
    }

    ++out;
  }

  return out;
}


} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_STRING_H */
