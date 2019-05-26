#include <java/hsperf/hsperf.h>
#include <cassert>
#include <cstring>
#include <string_view>

namespace java::hsperf {


hsperf::hsperf(std::string filename)
: data_(filename)
{
  if (data_.size() < sizeof(hsperf_prologue))
    throw std::invalid_argument("hsperf: \"" + filename + "\" is not a hsperf file (too short)");

  const hsperf_prologue* p = reinterpret_cast<const hsperf_prologue*>(data_.data());
  assert(p != nullptr);
  if (!p->valid_magic())
    throw std::invalid_argument("hsperf: \"" + filename + "\" is not a hsperf file (magic mismatch)");
  if (!p->valid_version())
    throw std::invalid_argument("hsperf: \"" + filename + "\" uses unsupported version");
}

hsperf::~hsperf() noexcept {}

auto hsperf::get() const
-> std::unordered_map<std::string, entry_type> {
  std::unordered_map<std::string, entry_type> result;

  const hsperf_prologue* p = reinterpret_cast<const hsperf_prologue*>(data_.data());
  p->validate_or_throw();

  std::uint32_t off = p->entry_offset();
  while (off != 0 && off < p->used()) {
    const perfdata* entry = read_at_offset<perfdata>(p->entry_offset());
    if (off + entry->entry_length(*p) > data_.size()) throw std::invalid_argument("hsperf: invalid entry length");

    // Figure out the name.
    // UTF-8 encoded, null-terminated string.
    // I don't know if the name is bound by the entry limit,
    // so I'll just use the file limit as upper bound.
    std::string_view name(read_at_offset<char>(off + entry->name_offset(*p)), data_.size() - (off + entry->name_offset(*p)));
    auto name_len = name.find('\0');
    if (name_len == std::string_view::npos) throw std::invalid_argument("invalid entry name");
    name = name.substr(0, name_len);

    const auto vector_len = entry->vector_len(*p);
    const auto data_off = off + entry->data_off(*p);
    entry_type value;
    switch (entry->data_type()) {
    case static_cast<std::uint8_t>(u8'Z'):
      if (vector_len == 0) {
        value.emplace<bool>(*read_at_offset<std::uint8_t>(data_off) != 0u);
      } else {
        std::vector<bool> values;
        values.reserve(vector_len);
        for (std::size_t i = 0; i < vector_len; ++i)
          values.push_back(*read_at_offset<std::uint8_t>(data_off + i) != 0u);
        value.emplace<std::vector<bool>>(std::move(values));
      }
      break;
    case static_cast<std::uint8_t>(u8'C'):
      if (vector_len == 0) {
        value.emplace<char>(*read_at_offset<char>(data_off));
      } else {
        value.emplace<std::string>(
            read_at_offset<char>(data_off),
            read_at_offset<char>(data_off + vector_len));
      }
      break;
    case static_cast<std::uint8_t>(u8'F'): // 8 bytes, for some reason
      if (vector_len == 0) {
        std::uint64_t dbl_encoded;
        std::memcpy(&dbl_encoded, read_at_offset<std::uint64_t>(data_off), sizeof(dbl_encoded));
        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          dbl_encoded = boost::endian::big_to_native(dbl_encoded);
          break;
        case hsperf_prologue::little_endian:
          dbl_encoded = boost::endian::little_to_native(dbl_encoded);
          break;
        }
        double dbl;
        std::memcpy(&dbl, &dbl_encoded, sizeof(dbl_encoded));
        value.emplace<float>(dbl);
      } else {
        std::vector<float> values;
        values.reserve(vector_len);
        for (std::size_t i = 0; i < vector_len; ++i) {
          std::uint64_t dbl_encoded;
          std::memcpy(&dbl_encoded, read_at_offset<std::uint64_t>(data_off), sizeof(dbl_encoded));
          switch (p->byte_order()) {
          default:
            throw std::runtime_error("Invalid endian flag");
          case hsperf_prologue::big_endian:
            dbl_encoded = boost::endian::big_to_native(dbl_encoded);
            break;
          case hsperf_prologue::little_endian:
            dbl_encoded = boost::endian::little_to_native(dbl_encoded);
            break;
          }
          double dbl;
          std::memcpy(&dbl, &dbl_encoded, sizeof(dbl_encoded));
          values.push_back(dbl);
        }
        value.emplace<std::vector<float>>(std::move(values));
      }
      break;
    case static_cast<std::uint8_t>(u8'D'):
      if (vector_len == 0) {
        std::uint64_t dbl_encoded;
        std::memcpy(&dbl_encoded, read_at_offset<std::uint64_t>(data_off), sizeof(dbl_encoded));
        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          dbl_encoded = boost::endian::big_to_native(dbl_encoded);
          break;
        case hsperf_prologue::little_endian:
          dbl_encoded = boost::endian::little_to_native(dbl_encoded);
          break;
        }
        double dbl;
        std::memcpy(&dbl, &dbl_encoded, sizeof(dbl_encoded));
        value.emplace<double>(dbl);
      } else {
        std::vector<double> values;
        values.reserve(vector_len);
        for (std::size_t i = 0; i < vector_len; ++i) {
          std::uint64_t dbl_encoded;
          std::memcpy(&dbl_encoded, read_at_offset<std::uint64_t>(data_off), sizeof(dbl_encoded));
          switch (p->byte_order()) {
          default:
            throw std::runtime_error("Invalid endian flag");
          case hsperf_prologue::big_endian:
            dbl_encoded = boost::endian::big_to_native(dbl_encoded);
            break;
          case hsperf_prologue::little_endian:
            dbl_encoded = boost::endian::little_to_native(dbl_encoded);
            break;
          }
          double dbl;
          std::memcpy(&values.back(), &dbl_encoded, sizeof(dbl_encoded));
          values.push_back(dbl);
        }
        value.emplace<std::vector<double>>(std::move(values));
      }
      break;
    case static_cast<std::uint8_t>(u8'B'):
      if (vector_len == 0) {
        value.emplace<std::uint8_t>(*read_at_offset<std::uint8_t>(data_off));
      } else {
        value.emplace<std::vector<std::uint8_t>>(
            read_at_offset<std::uint8_t>(data_off),
            read_at_offset<std::uint8_t>(data_off + vector_len - 1u) + 1u);
      }
      break;
    case static_cast<std::uint8_t>(u8'S'):
      if (vector_len == 0) {
        std::int16_t tmp;
        std::memcpy(&tmp, read_at_offset<std::int16_t>(data_off), sizeof(tmp));
        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          tmp = boost::endian::big_to_native(tmp);
          break;
        case hsperf_prologue::little_endian:
          tmp = boost::endian::little_to_native(tmp);
          break;
        }
        value.emplace<std::int16_t>(tmp);
      } else {
        value.emplace<std::vector<std::int16_t>>(
            read_at_offset<std::int16_t>(data_off),
            read_at_offset<std::int16_t>(data_off + sizeof(std::int16_t) * (vector_len - 1u)) + 1u);

        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          std::transform(
              std::get<std::vector<std::int16_t>>(value).begin(),
              std::get<std::vector<std::int16_t>>(value).end(),
              std::get<std::vector<std::int16_t>>(value).begin(),
              [](std::int16_t x) { return boost::endian::big_to_native(x); });
          break;
        case hsperf_prologue::little_endian:
          std::transform(
              std::get<std::vector<std::int16_t>>(value).begin(),
              std::get<std::vector<std::int16_t>>(value).end(),
              std::get<std::vector<std::int16_t>>(value).begin(),
              [](std::int16_t x) { return boost::endian::little_to_native(x); });
          break;
        }
      }
      break;
    case static_cast<std::uint8_t>(u8'I'):
      if (vector_len == 0) {
        std::int32_t tmp;
        std::memcpy(&tmp, read_at_offset<std::int32_t>(data_off), sizeof(tmp));
        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          tmp = boost::endian::big_to_native(tmp);
          break;
        case hsperf_prologue::little_endian:
          tmp = boost::endian::little_to_native(tmp);
          break;
        }
        value.emplace<std::int32_t>(tmp);
      } else {
        value.emplace<std::vector<std::int32_t>>(
            read_at_offset<std::int32_t>(data_off),
            read_at_offset<std::int32_t>(data_off + sizeof(std::int32_t) * (vector_len - 1u)) + 1u);

        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          std::transform(
              std::get<std::vector<std::int32_t>>(value).begin(),
              std::get<std::vector<std::int32_t>>(value).end(),
              std::get<std::vector<std::int32_t>>(value).begin(),
              [](std::int32_t x) { return boost::endian::big_to_native(x); });
          break;
        case hsperf_prologue::little_endian:
          std::transform(
              std::get<std::vector<std::int32_t>>(value).begin(),
              std::get<std::vector<std::int32_t>>(value).end(),
              std::get<std::vector<std::int32_t>>(value).begin(),
              [](std::int32_t x) { return boost::endian::little_to_native(x); });
          break;
        }
      }
      break;
    case static_cast<std::uint8_t>(u8'J'):
      if (vector_len == 0) {
        std::int64_t tmp;
        std::memcpy(&tmp, read_at_offset<std::int64_t>(data_off), sizeof(tmp));
        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          tmp = boost::endian::big_to_native(tmp);
          break;
        case hsperf_prologue::little_endian:
          tmp = boost::endian::little_to_native(tmp);
          break;
        }
        value.emplace<std::int64_t>(tmp);
      } else {
        value.emplace<std::vector<std::int64_t>>(
            read_at_offset<std::int64_t>(data_off),
            read_at_offset<std::int64_t>(data_off + sizeof(std::int64_t) * (vector_len - 1u)) + 1u);

        switch (p->byte_order()) {
        default:
          throw std::runtime_error("Invalid endian flag");
        case hsperf_prologue::big_endian:
          std::transform(
              std::get<std::vector<std::int64_t>>(value).begin(),
              std::get<std::vector<std::int64_t>>(value).end(),
              std::get<std::vector<std::int64_t>>(value).begin(),
              [](std::int64_t x) { return boost::endian::big_to_native(x); });
          break;
        case hsperf_prologue::little_endian:
          std::transform(
              std::get<std::vector<std::int64_t>>(value).begin(),
              std::get<std::vector<std::int64_t>>(value).end(),
              std::get<std::vector<std::int64_t>>(value).begin(),
              [](std::int64_t x) { return boost::endian::little_to_native(x); });
          break;
        }
      }
      break;
    }

    result[std::string(name.data(), name.size())] = std::move(value);

    off += entry->entry_length(*p);
  }

  return result;
}


} /* namespace java::hsperf */
