#include <java/serialization/string.h>
#include <cassert>
#include <stdexcept>

namespace java::serialization {


auto compute_utf_data_size(std::u16string_view s)
noexcept
-> std::size_t {
  std::size_t sz = 0;

  for (const char16_t elem : s) {
    const std::uint16_t c = static_cast<std::uint16_t>(elem);

    if (c > 0u && c <= 0x7fu)
      ++sz;
    else if (c <= 0x7ffu)
      sz += 2;
    else
      sz += 3;
  }

  return sz;
}

auto compute_utf_size(std::u16string_view s)
noexcept
-> std::size_t {
  return compute_utf_data_size(s) + 2u;
}

auto write_utf_data(std::u16string_view s, std::vector<std::uint8_t>& buffer)
-> std::vector<std::uint8_t>::size_type {
  const auto buffer_orig_sz = buffer.size();

  // Encoding loop.
  for (const char16_t elem : s) {
    const std::uint16_t c = static_cast<std::uint16_t>(elem);

    if (c > 0u && c <= 0x7fu) {
      buffer.push_back(c);
    } else if (c <= 0x7ffu) {
      buffer.push_back(0xc0u | ((c >> 6) & 0x1fu));
      buffer.push_back(0x80u | (c & 0x3fu));
    } else {
      buffer.push_back(0xe0u | ((c >> 12) & 0x0fu));
      buffer.push_back(0x80u | ((c >> 6) & 0x3fu));
      buffer.push_back(0x80u | (c & 0x3fu));
    }
  }

  assert(buffer.size() >= buffer_orig_sz);
  return buffer.size() - buffer_orig_sz;
}

auto write_utf(std::u16string_view s)
-> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> buffer(2); // Reserve space for length record.

  // Write encoded data.
  const auto len = write_utf_data(s, buffer);

  // Figure out number of bytes needed for encoding.
  // We could check the length inside the loop, but that would slow down the
  // encoding, and it's unlikely to happen often.
  // So doing it after the fact is faster for the common (happy) path.
  //
  // Note that we subtract 2, as we have 2 bytes at the start of the buffer
  // reserved for the length of the encoded string.
  if (len > 0xffffu)
    throw std::length_error("java::serialization: string too long");

  // Big endian, 2 bytes.
  buffer[0] = (len >> 8) & 0xffu;
  buffer[1] = len & 0xffu;
  return buffer;
}

auto write_long_utf(std::u16string_view s)
-> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> buffer(8); // Reserve space for length record.

  // Write encoded data.
  auto len = write_utf_data(s, buffer);

  // Figure out number of bytes needed for encoding.
  // We could check the length inside the loop, but that would slow down the
  // encoding, and it's unlikely to happen often.
  // So doing it after the fact is faster for the common (happy) path.
  //
  // Note that we subtract 2, as we have 2 bytes at the start of the buffer
  // reserved for the length of the encoded string.
  if (len > 0xffffffffffffffffull)
    throw std::length_error("java::serialization: string too long");

  // Big endian, 8 bytes.
  for (auto buf_iter = buffer.rend() - 8; buf_iter != buffer.rend(); ++buf_iter) {
    *buf_iter = len & 0xffu;
    len >>= 8;
  }
  return buffer;
}

auto read_utf_data(boost::asio::const_buffer buf)
-> std::u16string {
  std::u16string result;

  read_utf_data(buf, std::back_inserter(result));
  return result;
}

auto read_utf_len(boost::asio::const_buffer& buf)
-> std::uint16_t {
  // Check that the buffer holds a length specifier.
  if (buf.size() < 2)
    throw std::invalid_argument("java::serialization: buffer too short");

  const std::uint8_t* u8buf = static_cast<const std::uint8_t*>(buf.data());
  std::uint16_t len = static_cast<std::uint16_t>(u8buf[0]) << 8
      | static_cast<std::uint16_t>(u8buf[1]);

  buf = buf + 2;
  return len;
}

auto read_utf(boost::asio::const_buffer& buf_arg)
-> std::u16string {
  boost::asio::const_buffer buf = buf_arg;

  // Read length.
  const auto len = read_utf_len(buf);

  // Check that we have enough input to decode.
  if (buf.size() < len)
    throw std::invalid_argument("java::serialization: buffer too short");

  std::u16string result = read_utf_data(boost::asio::buffer(buf, len));

  // Epilogue: commit our changes to the argument reference.
  buf_arg = buf + len;
  return result;
}

auto read_long_utf_len(boost::asio::const_buffer& buf)
-> std::uint64_t {
  // Check that the buffer holds a length specifier.
  if (buf.size() < 8)
    throw std::invalid_argument("java::serialization: buffer too short");

  const std::uint8_t* u8buf = static_cast<const std::uint8_t*>(buf.data());
  std::uint64_t len = 0;
  for (const std::uint8_t* i = u8buf; i != u8buf + 8u; ++i) {
    len <<= 8;
    len |= *i;
  }

  buf = buf + 8;
  return len;
}

auto read_long_utf(boost::asio::const_buffer& buf_arg)
-> std::u16string {
  boost::asio::const_buffer buf = buf_arg;

  // Read length.
  const auto len = read_long_utf_len(buf);

  // Check that we have enough input to decode.
  if (buf.size() < len)
    throw std::invalid_argument("java::serialization: buffer too short");

  std::u16string result = read_utf_data(boost::asio::buffer(buf, len));

  // Epilogue: commit our changes to the argument reference.
  buf_arg = buf + len;
  return result;
}


} /* namespace java::serialization */
