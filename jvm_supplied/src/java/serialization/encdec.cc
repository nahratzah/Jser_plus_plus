#include <java/serialization/encdec.h>
#include <cassert>
#include <iomanip>

namespace java::serialization::stream {


auto operator==(const stream_element& x, const stream_element& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator<<(std::ostream& out, const stream_element& s)
-> std::ostream& {
  str_ctx ctx;
  out << ctx(s);
  return out;
}

auto to_string(const stream_element& s)
-> std::string {
  std::ostringstream oss;
  oss << s;
  return oss.str();
}

stream_element::~stream_element() noexcept = default;


auto stream_string::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<const stream_string> {
  using boost::asio::buffer;

  std::vector<std::uint8_t> data_buffer;
  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case TC_REFERENCE:
      {
        std::uint32_t ref;
        read(buffer(&ref, sizeof(ref)));
        boost::endian::big_to_native_inplace(ref);
        return r.template get_handle<stream_string>(ref);
      }
    case TC_STRING:
      {
        std::uint8_t buf[2];
        read(buffer(buf));

        boost::asio::const_buffer rbuf = buffer(buf);
        data_buffer.resize(read_utf_len(rbuf));
        assert(rbuf.size() == 0u); // All bytes consumed by read_utf_len.
      }
      break;
    case TC_LONGSTRING:
      {
        std::uint8_t buf[8];
        read(buffer(buf));

        boost::asio::const_buffer rbuf = buffer(buf);
        data_buffer.resize(read_long_utf_len(rbuf));
        assert(rbuf.size() == 0u); // All bytes consumed by read_utf_len.
      }
      break;
  }
  read(buffer(data_buffer));

  auto result = cycle_ptr::allocate_cycle<stream_string>(r.get_allocator(), r.get_allocator());
  r.next_handle(result);
  read_utf_data(buffer(data_buffer), std::back_inserter(*result));
  return result;
}

auto stream_string::decode(reader& r, basic_read_wrapper& read)
-> cycle_ptr::cycle_gptr<const stream_string> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint8_t hdr;
  read(buffer(&hdr, sizeof(hdr)));
  big_to_native_inplace(hdr);

  switch (hdr) {
    default:
      throw decoding_error("unexpected opcode while decoding string");
    case TC_REFERENCE:
      [[fallthrough]];
    case TC_STRING:
      [[fallthrough]];
    case TC_LONGSTRING:
      return decode(r, read, hdr);
  }
}

auto stream_string::read_utf(basic_read_wrapper& read, allocator_type alloc) -> stream_string {
  using boost::asio::buffer;

  std::vector<std::uint8_t> data_buffer;
  {
    std::uint8_t buf[2];
    read(buffer(buf));

    boost::asio::const_buffer rbuf = buffer(buf);
    data_buffer.resize(read_utf_len(rbuf));
    assert(rbuf.size() == 0u); // All bytes consumed by read_utf_len.
  }

  read(buffer(data_buffer));
  stream_string result;
  read_utf_data(buffer(data_buffer), std::back_inserter(result));
  return result;
}

auto stream_string::read_long_utf(basic_read_wrapper& read, allocator_type alloc) -> stream_string {
  using boost::asio::buffer;

  std::vector<std::uint8_t> data_buffer;
  {
    std::uint8_t buf[8];
    read(buffer(buf));

    boost::asio::const_buffer rbuf = buffer(buf);
    data_buffer.resize(read_long_utf_len(rbuf));
    assert(rbuf.size() == 0u); // All bytes consumed by read_long_utf_len.
  }

  read(buffer(data_buffer));
  stream_string result;
  read_utf_data(buffer(data_buffer), std::back_inserter(result));
  return result;
}

auto stream_string::to_out_(std::ostream& out, [[maybe_unused]] str_ctx& ctx) const
-> void {
  using conversion_type = std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t,
      typename stream_string::allocator_type,
      typename std::allocator_traits<typename stream_string::allocator_type>::template rebind_alloc<char>>;

  conversion_type conversion;
  out << conversion.to_bytes(*this);
}

auto stream_string::eq_(const stream_element& y, [[maybe_unused]] eq_ctx& ctx) const
-> bool {
  const std::u16string_view x_str = *this;
  const std::u16string_view y_str = dynamic_cast<const stream_string&>(y);
  return x_str == y_str;
}


auto field_descriptor::from_string(std::u16string_view s, char16_t sep)
-> field_descriptor {
  auto extents = s.find_first_not_of(u"[");
  if (extents == std::u16string_view::npos) extents = s.size();

  std::u16string_view name = s.substr(extents);

  if (name.empty()) throw std::invalid_argument("invalid field descriptor");
  switch (name.front()) {
    default:
      throw std::invalid_argument("invalid field descriptor");
    case u'B':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::byte_type, extents);
    case u'C':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::char_type, extents);
    case u'D':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::double_type, extents);
    case u'F':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::float_type, extents);
    case u'I':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::int_type, extents);
    case u'J':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::long_type, extents);
    case u'S':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::short_type, extents);
    case u'Z':
      if (name.length() != 1) throw std::invalid_argument("invalid field descriptor");
      return field_descriptor(primitive_type::bool_type, extents);
    case u'L':
      break;
  }

  assert(name.front() == u'L');
  if (name.back() != u';') throw std::invalid_argument("invalid field descriptor");

  assert(name.size() >= 2); // Should be, as front() == 'L' and back() == ';'.
  name.remove_prefix(1); // Consume 'L'.
  name.remove_suffix(1); // Consume trailing ';'.

  if (sep == field_descriptor::sep)
    return field_descriptor(name, extents);

  std::u16string name_str(name.data(), name.length());
  for (auto pos = name_str.find(sep);
      pos != std::u16string::npos;
      pos = name_str.find(sep, pos)) {
    name_str[pos] = field_descriptor::sep;
  }
  return field_descriptor(std::move(name_str), extents);
}

auto field_descriptor::to_string() const
-> std::u16string {
  std::u16string result;

  // Reserve space.
  if (std::holds_alternative<primitive_type>(type_))
    result.reserve(extents_ + 1u);
  else
    result.reserve(extents_ + std::get<std::u16string>(type_).size() + 2u);

  result.append(extents_, u'[');
  if (std::holds_alternative<primitive_type>(type_)) {
    switch (std::get<primitive_type>(type_)) {
      case primitive_type::byte_type:
        result.append(1, u'B');
        break;
      case primitive_type::char_type:
        result.append(1, u'C');
        break;
      case primitive_type::double_type:
        result.append(1, u'D');
        break;
      case primitive_type::float_type:
        result.append(1, u'F');
        break;
      case primitive_type::int_type:
        result.append(1, u'I');
        break;
      case primitive_type::long_type:
        result.append(1, u'J');
        break;
      case primitive_type::short_type:
        result.append(1, u'S');
        break;
      case primitive_type::bool_type:
        result.append(1, u'Z');
        break;
    }
  } else {
    result
        .append(1, u'L')
        .append(std::get<std::u16string>(type_))
        .append(1, u';');
  }
  return result;
}

auto field_descriptor::path() const
-> std::vector<std::u16string> {
  std::vector<std::u16string> result;

  std::u16string_view sv = std::get<std::u16string>(type_);

  for (auto path_sep = sv.find(sep);
      path_sep != std::u16string_view::npos;
      path_sep = (sv = sv.substr(path_sep + 1u)).find(sep)) {
    result.emplace_back(sv.data(), path_sep);
  }
  result.emplace_back(sv.data(), sv.size());

  return result;
}


auto blockdata::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> blockdata {
  using boost::asio::buffer;
  using boost::endian::big_to_native;

  blockdata result;

  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case TC_BLOCKDATA:
      {
        std::uint8_t len;
        read(buffer(&len, sizeof(len)));
        result.resize(big_to_native(len));
      }
      break;
    case TC_BLOCKDATALONG:
      {
        std::uint32_t len;
        read(buffer(&len, sizeof(len)));
        result.resize(big_to_native(len));
      }
      break;
  }

  read(buffer(result));
  return result;
}

auto operator<<(std::ostream& out, const blockdata& data)
-> std::ostream& {
  using size_type = blockdata::size_type;

  // Save stream formatters and restore after writing.
  boost::io::ios_flags_saver saved_fl(out);
  boost::io::ios_fill_saver saved_fill(out);
  boost::io::ios_width_saver saved_width(out);

  out << data.size() << " bytes:";
  out << std::hex << std::setfill('0') << std::right;

  size_type idx = 0;
  for (const std::uint32_t byte : data) {
    if (idx % 16u == 0u)
      out << "\n";
    else if (idx % 8u == 0u)
      out << "    ";
    else if (idx % 4u == 0u)
      out << "  ";
    else
      out << " ";

    out << std::setw(2) << byte;
    ++idx;
  }

  return out;
}

auto to_string(const blockdata& data)
-> std::string {
  std::ostringstream oss;
  oss << data;
  return oss.str();
}


auto primitive_desc::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> primitive_desc& {
  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case static_cast<std::uint8_t>(primitive_type::byte_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::char_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::double_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::float_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::int_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::long_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::short_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::bool_type):
      type = static_cast<primitive_type>(hdr);
      break;
  }

  field_name = stream_string::read_utf(read, r.get_allocator());

  return *this;
}

auto primitive_desc::decode_field(reader& r, basic_read_wrapper& read) const
-> value_type {
  return decode_field(type, r, read);
}

auto primitive_desc::decode_field(primitive_type type, [[maybe_unused]] reader& r, basic_read_wrapper& read)
-> value_type {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  value_type result;

  switch (type) {
    case primitive_type::byte_type:
      result.emplace<std::int8_t>();
      read(buffer(&std::get<std::int8_t>(result), sizeof(std::int8_t)));
      big_to_native_inplace(std::get<std::int8_t>(result));
      break;
    case primitive_type::char_type:
      result.emplace<char16_t>();
      read(buffer(&std::get<char16_t>(result), sizeof(char16_t)));
      big_to_native_inplace(std::get<char16_t>(result));
      break;
    case primitive_type::double_type:
      {
        std::uint64_t tmp;
        read(buffer(&tmp, sizeof(tmp)));
        big_to_native_inplace(tmp);

        double v;
        static_assert(sizeof(tmp) == sizeof(v));
        std::memcpy(&v, &tmp, sizeof(v));
        result.emplace<double>(v);
      }
      break;
    case primitive_type::float_type:
      {
        std::uint32_t tmp;
        read(buffer(&tmp, sizeof(tmp)));
        big_to_native_inplace(tmp);

        float v;
        static_assert(sizeof(tmp) == sizeof(v));
        std::memcpy(&v, &tmp, sizeof(v));
        result.emplace<float>(v);
      }
      break;
    case primitive_type::int_type:
      result.emplace<std::int32_t>();
      read(buffer(&std::get<std::int32_t>(result), sizeof(std::int32_t)));
      big_to_native_inplace(std::get<std::int32_t>(result));
      break;
    case primitive_type::long_type:
      result.emplace<std::int64_t>();
      read(buffer(&std::get<std::int64_t>(result), sizeof(std::int64_t)));
      big_to_native_inplace(std::get<std::int64_t>(result));
      break;
    case primitive_type::short_type:
      result.emplace<std::int16_t>();
      read(buffer(&std::get<std::int16_t>(result), sizeof(std::int16_t)));
      big_to_native_inplace(std::get<std::int16_t>(result));
      break;
    case primitive_type::bool_type:
      {
        std::uint8_t tmp;
        read(buffer(&tmp, sizeof(tmp)));
        big_to_native_inplace(tmp);
        result.emplace<bool>(tmp != 0);
      }
      break;
  }

  return result;
}

auto primitive_desc::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  out << "field: "
      << ctx(field_name)
      << " (discriminant '"
      << static_cast<char>(static_cast<std::uint8_t>(type))
      << "')\n";
  switch (type) {
    case primitive_type::byte_type:
      out << "type: byte";
      break;
    case primitive_type::char_type:
      out << "type: char";
      break;
    case primitive_type::double_type:
      out << "type: double";
      break;
    case primitive_type::float_type:
      out << "type: float";
      break;
    case primitive_type::int_type:
      out << "type: int";
      break;
    case primitive_type::long_type:
      out << "type: long";
      break;
    case primitive_type::short_type:
      out << "type: short";
      break;
    case primitive_type::bool_type:
      out << "type: bool";
      break;
  }
}

auto primitive_desc::eq_(const primitive_desc& y, eq_ctx& ctx) const
-> bool {
  return type == y.type
      && ctx(field_name, y.field_name);
}

auto operator==(const primitive_desc& x, const primitive_desc& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator!=(const primitive_desc& x, const primitive_desc& y)
-> bool {
  return !(x == y);
}

auto to_string(const primitive_desc& desc)
-> std::string {
  std::ostringstream oss;
  oss << desc;
  return oss.str();
}

auto operator<<(std::ostream& out, const primitive_desc& desc)
-> std::ostream& {
  str_ctx ctx;
  return out << ctx(desc);
}


auto object_desc::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> object_desc& {
  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case static_cast<std::uint8_t>(object_type::array_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(object_type::object_type):
      type = static_cast<object_type>(hdr);
      break;
  }

  field_name = stream_string::read_utf(read, r.get_allocator());
  const auto field_type_str = stream_string::decode(r, read);

  try {
    field_type = field_descriptor::from_string(*field_type_str, u'/');
  } catch (const std::invalid_argument& ex) {
    // Repackage as decoding error.
    throw decoding_error(ex.what());
  }

  if (type == object_type::array_type) {
    if (field_type.extents() == 0)
      throw decoding_error("object_desc: array type declared, but field is not an array");
  } else {
    if (field_type.extents() != 0)
      throw decoding_error("object_desc: object type declared, but field is an array");
  }

  return *this;
}

auto object_desc::decode_field(reader& r, basic_read_wrapper& read) const
-> value_type {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint8_t type_hdr;
  read(buffer(&type_hdr, sizeof(type_hdr)));
  big_to_native_inplace(type_hdr);

  switch (type_hdr) {
    default:
      [[fallthrough]];
    case TC_BLOCKDATA:
      [[fallthrough]];
    case TC_BLOCKDATALONG:
      throw decoding_error("unexpected opcode while decoding object field value");

    case TC_OBJECT:
      [[fallthrough]];
    case TC_CLASS:
      [[fallthrough]];
    case TC_ARRAY:
      [[fallthrough]];
    case TC_STRING:
      [[fallthrough]];
    case TC_LONGSTRING:
      [[fallthrough]];
    case TC_ENUM:
      [[fallthrough]];
    case TC_CLASSDESC:
      [[fallthrough]];
    case TC_PROXYCLASSDESC:
      [[fallthrough]];
    case TC_REFERENCE:
      [[fallthrough]];
    case TC_NULL:
      [[fallthrough]];
    case TC_EXCEPTION:
      return std::get<cycle_ptr::cycle_gptr<const stream_element>>(
          content::decode(r, read, type_hdr));
  }
}

auto object_desc::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  out << "field: "
      << ctx(field_name)
      << " (discriminant '"
      << static_cast<char>(static_cast<std::uint8_t>(type))
      << "')\n"
      << "type: "
      << indent_(field_type, 7);
}

auto object_desc::eq_(const object_desc& y, eq_ctx& ctx) const
-> bool {
  return type == y.type
      && ctx(field_name, y.field_name)
      && field_type == y.field_type;
}

auto operator==(const object_desc& x, const object_desc& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator!=(const object_desc& x, const object_desc& y)
-> bool {
  return !(x == y);
}

auto to_string(const object_desc& desc)
-> std::string {
  std::ostringstream oss;
  oss << desc;
  return oss.str();
}

auto operator<<(std::ostream& out, const object_desc& desc)
-> std::ostream& {
  str_ctx ctx;
  return out << ctx(desc);
}


auto field_desc::decode(reader& r, basic_read_wrapper& read)
-> field_desc& {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint8_t type_hdr;
  read(buffer(&type_hdr, sizeof(type_hdr)));
  big_to_native_inplace(type_hdr);

  field_desc result;
  switch (type_hdr) {
    default:
      throw decoding_error("unrecognized field desc discriminant");

    case static_cast<std::uint8_t>(primitive_type::byte_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::char_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::double_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::float_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::int_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::long_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::short_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(primitive_type::bool_type):
      this->template emplace<primitive_desc>();
      std::get<primitive_desc>(*this)
          .decode(r, read, type_hdr);
      break;

    case static_cast<std::uint8_t>(object_type::array_type):
      [[fallthrough]];
    case static_cast<std::uint8_t>(object_type::object_type):
      this->template emplace<object_desc>();
      std::get<object_desc>(*this)
          .decode(r, read, type_hdr);
      break;
  }
  return *this;
}

auto field_desc::decode_field(reader& r, basic_read_wrapper& read) const
-> value_type {
  return std::visit(
      [&r, &read](const auto& desc) -> value_type {
        return desc.decode_field(r, read);
      },
      *this);
}

auto field_desc::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  std::visit(
      [&out, &ctx](const auto& x) {
        out << ctx(x);
      },
      *this);
}

auto field_desc::eq_(const field_desc& y, eq_ctx& ctx) const
-> bool {
  return std::visit(
      [&ctx](const auto& x, const auto& y) -> bool {
        if constexpr(!std::is_same_v<std::decay_t<decltype(x)>, std::decay_t<decltype(y)>>)
          return false;
        else
          return ctx(x, y);
      },
      *this, y);
}

auto operator==(const field_desc& x, const field_desc& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator!=(const field_desc& x, const field_desc& y)
-> bool {
  return !(x == y);
}

auto to_string(const field_desc& desc)
-> std::string {
  std::ostringstream oss;
  oss << desc;
  return oss.str();
}

auto operator<<(std::ostream& out, const field_desc& desc)
-> std::ostream& {
  str_ctx ctx;
  return out << ctx(desc);
}


auto new_object::class_data::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (!fields.empty()) {
    out << "fields:\n"
        << "-------";
    for (const auto& field : fields) {
      out << "\n" << ctx(field.first) << " = ";
      std::visit(
          [&out, &field, &ctx](const auto& x) {
            if constexpr(std::is_convertible_v<std::decay_t<decltype(x)>, cycle_ptr::cycle_gptr<const stream_element>>) {
              if (x == nullptr)
                out << "nullptr";
              else
                out << indent_(ctx(*x), field.first.length() + 3u);
            } else {
              out << std::int64_t(x);
            }
          },
          field.second);
    }
  }

  if (!fields.empty() && !annotation.empty()) out << "\n";

  if (!annotation.empty()) {
    out << "annotation\n"
        << "----------";
    for (const auto& annot : annotation) {
      out << "\n";
      std::visit(
          [&out, &ctx](const auto& x) {
            if constexpr(std::is_same_v<blockdata, std::decay_t<decltype(x)>>) {
              out << "binary data:\n"
                  << "    " << indent_(x, 4);
            } else {
              if (x == nullptr)
                out << "(nullptr)";
              else
                out << ctx(*x);
            }
          },
          annot);
    }
  }
}

auto new_object::class_data::eq_(const class_data& y, eq_ctx& ctx) const
-> bool {
  if (!std::is_permutation(
          fields.begin(), fields.end(),
          y.fields.begin(), y.fields.end(),
          [&ctx](const auto& x, const auto& y) -> bool {
            if (!ctx(x.first, y.first)) return false;
            return std::visit(
                [&ctx](const auto& x, const auto& y) -> bool {
                  if constexpr(!std::is_same_v<std::decay_t<decltype(x)>, std::decay_t<decltype(y)>>) {
                    return false;
                  } else if constexpr(std::is_same_v<typename cycle_ptr::cycle_member_ptr<const stream_element>, std::decay_t<decltype(x)>>) {
                    if (x == nullptr || y == nullptr) return x == y;
                    return ctx(*x, *y);
                  } else {
                    return x == y;
                  }
                },
                x.second, y.second);
          }))
    return false;

  return std::equal(
      annotation.begin(), annotation.end(),
      y.annotation.begin(), y.annotation.end(),
      [&ctx](const auto& x, const auto& y) {
        if constexpr(!std::is_same_v<std::decay_t<decltype(x)>, std::decay_t<decltype(y)>>) {
          return false;
        } else if constexpr(std::is_same_v<typename cycle_ptr::cycle_member_ptr<const stream_element>, std::decay_t<decltype(x)>>) {
          if (x == nullptr || y == nullptr) return x == y;
          return ctx(*x, *y);
        } else {
          return x == y;
        }
      });
}

auto new_object::class_data::operator==(const class_data& y) const
-> bool {
  eq_ctx ctx;
  return ctx(*this, y);
}

auto new_object::class_data::operator!=(const class_data& y) const
-> bool {
  return !(*this == y);
}

auto new_object::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_object> {
  if (hdr != TC_OBJECT) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_object>(r.get_allocator());

  result->cls = class_desc::decode(r, read);
  r.next_handle(result);

  // Build inheritance stack.
  std::stack<cycle_ptr::cycle_gptr<const class_desc>> types;
  for (cycle_ptr::cycle_gptr<const class_desc> type_ptr = result->cls;
      type_ptr != nullptr;
      type_ptr = type_ptr->get_super())
    types.push(type_ptr);

  while (!types.empty()) {
    const auto type = types.top();

    [[maybe_unused]] bool inserted;
    auto fields = type->decode_fields(r, read);
    auto annotation = type->decode_annotation(r, read);
    std::tie(std::ignore, inserted) = result->data.emplace(
        std::piecewise_construct,
        std::make_tuple(type),
        std::forward_as_tuple(std::move(fields), std::move(annotation)));

    // Insert should never fail, as class diagrams can't recurse,
    // and the code doesn't allow class references to be used until
    // the class is complete.
    assert(inserted);

    types.pop();
  }

  return result;
}

auto new_object::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (cls == nullptr)
    out << "class: (nullptr)";
  else
    out << "class: " << indent_(ctx(*cls), 7);

  if (data.empty()) {
    out << "\ndata: {}";
  } else {
    std::string_view indent = "data: ";

    for (const auto& i : data) {
      out << "\n"
          << std::exchange(indent, "      ");
      if (i.first == nullptr)
        out << "(nullptr)";
      else
        out << indent_(i.first->name_for_to_string_(), indent.size());
      out << "\n"
          << indent
          << indent_(ctx(i.second), indent.size());
    }
  }
}

auto new_object::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_object& y_val = dynamic_cast<const new_object&>(y);
  if ((cls == nullptr) != (y_val.cls == nullptr)) return false;
  if (cls != nullptr && !ctx(*cls, *y_val.cls)) return false;

  return std::is_permutation(
      data.begin(), data.end(),
      y_val.data.begin(), y_val.data.end(),
      [&ctx](const auto& x, const auto& y) -> bool {
        if ((x.first == nullptr) != (y.first == nullptr)) return false;
        if (x.first != nullptr && !ctx(*x.first, *y.first)) return false;
        return ctx(x.second, y.second);
      });
}


auto class_annotation::decode(reader& r, basic_read_wrapper& read)
-> class_annotation& {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint8_t hdr;
  read(buffer(&hdr, sizeof(hdr)));
  big_to_native_inplace(hdr);

  this->clear();
  while (hdr != TC_ENDBLOCKDATA) {
    std::visit(
        [this](auto&& x) {
          this->emplace_back(std::forward<decltype(x)>(x));
        },
        content::decode(r, read, hdr));

    read(buffer(&hdr, sizeof(hdr)));
    big_to_native_inplace(hdr);
  }

  return *this;
}

auto class_annotation::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (this->empty()) {
    out << "(empty annotation)";
  } else {
    bool first = true;
    for (const auto& annot : *this) {
      if (!std::exchange(first, false)) out << "\n";
      std::visit(
          [&out, &ctx](const auto& x) {
            if constexpr(std::is_same_v<blockdata, std::decay_t<decltype(x)>>) {
              out << "binary data:\n"
                  << "    " << indent_(x, 4);
            } else {
              if (x == nullptr)
                out << "(nullptr)";
              else
                out << ctx(*x);
            }
          },
          annot);
    }
  }
}

auto class_annotation::eq_(const class_annotation& y, eq_ctx& ctx) const
-> bool {
  return std::equal(
      begin(), end(),
      y.begin(), y.end(),
      [&ctx](const auto& x, const auto& y) {
        if constexpr(!std::is_same_v<std::decay_t<decltype(x)>, std::decay_t<decltype(y)>>) {
          return false;
        } else if constexpr(std::is_same_v<typename cycle_ptr::cycle_member_ptr<const stream_element>, std::decay_t<decltype(x)>>) {
          if (x == nullptr || y == nullptr) return x == y;
          return ctx(*x, *y);
        } else {
          return x == y;
        }
      });
}

auto to_string(const class_annotation& data)
-> std::string {
  std::ostringstream oss;
  oss << data;
  return oss.str();
}

auto operator<<(std::ostream& out, const class_annotation& data)
-> std::ostream& {
  str_ctx ctx;
  out << ctx(data);
  return out;
}


auto class_desc_info::decode(reader& r, basic_read_wrapper& read)
-> class_desc_info& {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint16_t num_fields;
  read({ buffer(&flags, sizeof(flags)), buffer(&num_fields, sizeof(num_fields)) });
  big_to_native_inplace(flags);
  big_to_native_inplace(num_fields);

  fields.resize(num_fields);
  std::for_each(
      fields.begin(),
      fields.end(),
      [&r, &read](auto& fd) { fd.decode(r, read); });
  annotation.decode(r, read);
  super = class_desc::decode(r, read);

  return *this;
}

auto class_desc_info::decode_fields(reader& r, basic_read_wrapper& read) const
-> field_map {
  field_map result;

  if ((flags & SC_SERIALIZABLE) == SC_SERIALIZABLE) {
    result.reserve(fields.size());
    std::transform(
        fields.begin(),
        fields.end(),
        std::inserter(result, result.end()),
        [&r, &read](const field_desc& desc) {
          return std::make_pair(stream_string(desc.name(), r.get_allocator()), desc.decode_field(r, read));
        });
  }

  return result;
}

auto class_desc_info::decode_annotation(reader& r, basic_read_wrapper& read) const
-> obj_content {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  obj_content result;

  if ((flags & (SC_SERIALIZABLE | SC_WRITE_METHOD)) == SC_SERIALIZABLE)
    return result;
  if ((flags & SC_EXTERNALIZABLE) == SC_EXTERNALIZABLE) {
    if ((flags & SC_BLOCK_DATA) != SC_BLOCK_DATA)
      throw decoding_error("externalizable data in PROTOCOL_VERSION_1 is not supported");
  }

  bool done = false;
  do {
    std::uint8_t hdr;
    read(buffer(&hdr, sizeof(hdr)));
    big_to_native_inplace(hdr);

    switch (hdr) {
      default:
        throw decoding_error("incorrect content header");
      case TC_BLOCKDATA:
        [[fallthrough]];
      case TC_BLOCKDATALONG:
        [[fallthrough]];
      case TC_OBJECT:
        [[fallthrough]];
      case TC_CLASS:
        [[fallthrough]];
      case TC_STRING:
        [[fallthrough]];
      case TC_LONGSTRING:
        [[fallthrough]];
      case TC_ENUM:
        [[fallthrough]];
      case TC_CLASSDESC:
        [[fallthrough]];
      case TC_PROXYCLASSDESC:
        [[fallthrough]];
      case TC_REFERENCE:
        [[fallthrough]];
      case TC_NULL:
        [[fallthrough]];
      case TC_EXCEPTION:
        std::visit(
            [&result](auto&& x) {
              result.emplace_back(std::forward<decltype(x)>(x));
            },
            content::decode(r, read, hdr));
        break;
      case TC_ENDBLOCKDATA:
        done = true; // Guard.
        break;
    }
  } while (!done);

  return result;
}

auto class_desc_info::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  boost::io::ios_flags_saver saved_fl(out);
  boost::io::ios_fill_saver saved_fill(out);
  boost::io::ios_width_saver saved_width(out, 0);

  out << "flags:";
  if (flags == 0u) out << " (none)";
  if (flags & SC_WRITE_METHOD) out << " SC_WRITE_METHOD";
  if (flags & SC_BLOCK_DATA) out << " SC_BLOCK_DATA";
  if (flags & SC_SERIALIZABLE) out << " SC_SERIALIZABLE";
  if (flags & SC_EXTERNALIZABLE) out << " SC_EXTERNALIZABLE";
  if (flags & SC_ENUM) out << " SC_ENUM";
  if (flags & ~(SC_WRITE_METHOD | SC_BLOCK_DATA | SC_SERIALIZABLE | SC_EXTERNALIZABLE | SC_ENUM)) {
    out << " unknown: 0x" << std::hex << std::setfill('0') << std::setw(2)
        << std::uint32_t(flags & ~(SC_WRITE_METHOD | SC_BLOCK_DATA | SC_SERIALIZABLE | SC_EXTERNALIZABLE | SC_ENUM));
    saved_fl.restore();
    saved_fill.restore();
  }

  if (fields.empty()) {
    out << std::endl
        << "fields: none";
  } else {
    out << std::endl
        << "fields: (" << fields.size() << " fields)\n"
        << "-------";
    for (const auto& field : fields)
      out << std::endl << ctx(field);
  }

  out << std::endl
      << "annotation:\n"
      << "-----------\n"
      << ctx(annotation);

  if (super != nullptr) {
    out << std::endl
        << "super: "
        << indent_(ctx(*super), 7);
  }
}

auto class_desc_info::eq_(const class_desc_info& y, eq_ctx& ctx) const
-> bool {
  if (flags != y.flags) return false;

  if (!std::is_permutation(
          fields.begin(), fields.end(),
          y.fields.begin(), y.fields.end(),
          std::ref(ctx)))
    return false;

  if (!ctx(annotation, y.annotation)) return false;

  if (super == nullptr || y.super == nullptr) {
    if (super != y.super) return false;
  } else if (!ctx(*super, *y.super)) {
    return false;
  }

  return true;
}

auto operator==(const class_desc_info& x, const class_desc_info& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator!=(const class_desc_info& x, const class_desc_info& y)
-> bool {
  return !(x == y);
}

auto to_string(const class_desc_info& info)
-> std::string {
  std::ostringstream oss;
  oss << info;
  return oss.str();
}

auto operator<<(std::ostream& out, const class_desc_info& info)
-> std::ostream& {
  str_ctx ctx;
  out << ctx(info);
  return out;
}


class_desc::~class_desc() noexcept = default;

auto class_desc::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<const class_desc> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case TC_CLASSDESC:
      return new_class_desc__class_desc::decode(r, read, hdr);
    case TC_PROXYCLASSDESC:
      return new_class_desc__proxy_class_desc::decode(r, read, hdr);
    case TC_NULL: // nullReference
      return nullptr;
    case TC_REFERENCE: // (ClassDesc)prevObject
      {
        std::uint32_t handle;
        read(buffer(&handle, sizeof(handle)));
        big_to_native_inplace(handle);
        return r.template get_handle<class_desc>(handle);
      }
  }
}

auto class_desc::decode(reader& r, basic_read_wrapper& read)
-> cycle_ptr::cycle_gptr<const class_desc> {
  using boost::asio::buffer;

  std::uint8_t hdr;
  read(buffer(&hdr, sizeof(hdr)));

  switch (hdr) {
    default:
      throw decoding_error("invalid class desc discriminant");
    case TC_CLASSDESC:
      [[fallthrough]];
    case TC_PROXYCLASSDESC:
      [[fallthrough]];
    case TC_NULL:
      [[fallthrough]];
    case TC_REFERENCE:
      return decode(r, read, hdr);
  }
}


auto new_class_desc__class_desc::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_class_desc__class_desc> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  if (hdr != TC_CLASSDESC) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_class_desc__class_desc>(r.get_allocator());

  const auto class_name = stream_string::read_utf(read, r.get_allocator());
  if (!class_name.empty() && class_name.front() == u'[')
    result->class_name = field_descriptor::from_string(class_name, u'.');
  else
    result->class_name = field_descriptor(std::move(class_name));

  read(buffer(&result->serial_version_uid, sizeof(result->serial_version_uid)));
  big_to_native_inplace(result->serial_version_uid);
  r.next_handle(result);
  result->info.decode(r, read);

  return result;
}

auto new_class_desc__class_desc::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  {
    boost::io::ios_flags_saver saved_fl(out);
    boost::io::ios_fill_saver saved_fill(out);
    boost::io::ios_width_saver saved_width(out);

    out << class_name
        << " (serialVersionUID = 0x"
        << std::hex << std::setfill('0') << std::right << std::setw(16) << serial_version_uid
        << ")";
  }

  out << "\n" << ctx(info);
}

auto new_class_desc__class_desc::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_class_desc__class_desc& y_val = dynamic_cast<const new_class_desc__class_desc&>(y);
  return class_name == y_val.class_name
      && (class_name.extents() != 0 // Serial version UID does not need to match for array types, according to the spec.
          || serial_version_uid == y_val.serial_version_uid)
      && ctx(info, y_val.info);
}


auto proxy_class_desc_info::decode(reader& r, basic_read_wrapper& read)
-> proxy_class_desc_info& {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint32_t interface_count;
  read(buffer(&interface_count, sizeof(interface_count)));
  big_to_native_inplace(interface_count);

  proxy_interface_names.clear();
  proxy_interface_names.reserve(interface_count);
  std::generate_n(
      std::back_inserter(proxy_interface_names),
      interface_count,
      [&r, &read]() { return stream_string::read_utf(read, r.get_allocator()); });

  annotation.decode(r, read);
  super = class_desc::decode(r, read);

  return *this;
}

auto proxy_class_desc_info::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  out << "annotation:\n"
      << "-----------\n"
      << ctx(annotation);
  if (super != nullptr) {
    out << "\n"
        << "super:\n"
        << "------\n"
        << ctx(*super);
  }
}

auto proxy_class_desc_info::eq_(const proxy_class_desc_info& y, eq_ctx& ctx) const
-> bool {
  if (!std::is_permutation(
          proxy_interface_names.begin(), proxy_interface_names.end(),
          y.proxy_interface_names.begin(), y.proxy_interface_names.end(),
          std::ref(ctx)))
    return false;

  if (!ctx(annotation, y.annotation)) return false;

  if (super == nullptr) {
    if (y.super != nullptr) return false;
  } else {
    if (y.super == nullptr || !ctx(*super, *y.super)) return false;
  }

  return true;
}

auto operator==(const proxy_class_desc_info& x, const proxy_class_desc_info& y)
-> bool {
  eq_ctx ctx;
  return ctx(x, y);
}

auto operator!=(const proxy_class_desc_info& x, const proxy_class_desc_info& y)
-> bool {
  return !(x == y);
}

auto to_string(const proxy_class_desc_info& data)
-> std::string {
  std::ostringstream oss;
  oss << data;
  return oss.str();
}

auto operator<<(std::ostream& out, const proxy_class_desc_info& data)
-> std::ostream& {
  str_ctx ctx;
  return out << ctx(data);
}


auto new_class_desc__proxy_class_desc::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_class_desc__proxy_class_desc> {
  if (hdr != TC_PROXYCLASSDESC) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_class_desc__proxy_class_desc>(r.get_allocator());
  result->desc.decode(r, read);

  return result;
}

auto new_class_desc__proxy_class_desc::get_super() const
-> cycle_ptr::cycle_gptr<const class_desc> {
  return desc.super;
}

auto new_class_desc__proxy_class_desc::decode_fields(
    [[maybe_unused]] reader& r,
    [[maybe_unused]] basic_read_wrapper& read) const
-> class_desc::field_map {
  return {};
}

auto new_class_desc__proxy_class_desc::decode_annotation(
    [[maybe_unused]] reader& r,
    [[maybe_unused]] basic_read_wrapper& read) const
-> class_desc::obj_content {
  return {};
}

auto new_class_desc__proxy_class_desc::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_class_desc__proxy_class_desc& y_val = dynamic_cast<const new_class_desc__proxy_class_desc&>(y);

  return ctx(desc, y_val.desc);
}

auto new_class_desc__proxy_class_desc::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  out << "proxy: " << indent_(ctx(desc), 7);
}


auto new_class::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_class> {
  using boost::asio::buffer;

  if (hdr != TC_CLASS) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_class>(r.get_allocator());
  *result = class_desc::decode(r, read);

  r.next_handle(result);
  return result;
}

auto new_class::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_class& y_val = dynamic_cast<const new_class&>(y);

  if (*this == nullptr) return y_val == nullptr;
  return y_val != nullptr && ctx(**this, *y_val);
}

auto new_class::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (*this == nullptr)
    out << "(nullptr new class)";
  else
    out << ctx(**this);
}


auto new_array::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_array> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  if (hdr != TC_ARRAY) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_array>(r.get_allocator());
  result->type = class_desc::decode(r, read);
  r.next_handle(result);

  std::uint32_t size;
  read(buffer(&size, sizeof(size)));
  big_to_native_inplace(size);

  auto cls = std::dynamic_pointer_cast<const new_class_desc__class_desc>(result->type);
  if (cls == nullptr)
    throw decoding_error("arrays must always have a concrete class name?");
  if (cls->class_name.extents() == 0)
    throw decoding_error("array type must have at least one extent");

  if (cls->class_name.extents() > 1 || !cls->class_name.is_primitive()) {
    result->data.emplace<object_array>(object_array::allocator_type(*result));
    auto& vec = std::get<object_array>(result->data);
    vec.reserve(size);
    for (std::uint64_t i = 0; i < size; ++i) {
      auto obj = content::decode(r, read);
      try {
        vec.emplace_back(std::get<cycle_ptr::cycle_gptr<const stream_element>>(std::move(obj)));
      } catch (const std::bad_variant_access&) {
        throw decoding_error("expected an object");
      }
    }
  } else {
    switch (std::get<primitive_type>(cls->class_name.type())) {
      case primitive_type::byte_type:
        {
          result->data.template emplace<byte_array>(r.get_allocator());
          auto& vec = std::get<byte_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<std::int8_t>(primitive_desc::decode_field(primitive_type::byte_type, r, read)); });
        }
        break;
      case primitive_type::char_type:
        {
          result->data.template emplace<char_array>(r.get_allocator());
          auto& vec = std::get<char_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<char16_t>(primitive_desc::decode_field(primitive_type::char_type, r, read)); });
        }
        break;
      case primitive_type::double_type:
        {
          result->data.template emplace<double_array>(r.get_allocator());
          auto& vec = std::get<double_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<double>(primitive_desc::decode_field(primitive_type::double_type, r, read)); });
        }
        break;
      case primitive_type::float_type:
        {
          result->data.template emplace<float_array>(r.get_allocator());
          auto& vec = std::get<float_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<float>(primitive_desc::decode_field(primitive_type::float_type, r, read)); });
        }
        break;
      case primitive_type::int_type:
        {
          result->data.template emplace<int_array>(r.get_allocator());
          auto& vec = std::get<int_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<std::int32_t>(primitive_desc::decode_field(primitive_type::int_type, r, read)); });
        }
        break;
      case primitive_type::long_type:
        {
          result->data.template emplace<long_array>(r.get_allocator());
          auto& vec = std::get<long_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<std::int64_t>(primitive_desc::decode_field(primitive_type::long_type, r, read)); });
        }
        break;
      case primitive_type::short_type:
        {
          result->data.template emplace<short_array>(r.get_allocator());
          auto& vec = std::get<short_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<std::int16_t>(primitive_desc::decode_field(primitive_type::short_type, r, read)); });
        }
        break;
      case primitive_type::bool_type:
        {
          result->data.template emplace<bool_array>(r.get_allocator());
          auto& vec = std::get<bool_array>(result->data);
          vec.reserve(size);
          std::generate_n(
              std::back_inserter(vec),
              size,
              [&r, &read]() { return std::get<bool>(primitive_desc::decode_field(primitive_type::bool_type, r, read)); });
        }
        break;
    }
  }

  return result;
}

auto new_array::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_array& y_val = dynamic_cast<const new_array&>(y);

  if ((type == nullptr) != (y_val.type == nullptr)) return false;
  if (type != nullptr && !ctx(*type, *y_val.type)) return false;

  return std::visit(
      [&ctx](const auto& x_arr, const auto& y_arr) -> bool {
        if constexpr(!std::is_same_v<std::decay_t<decltype(x_arr)>, std::decay_t<decltype(y_arr)>>) {
          return false;
        } else if constexpr(std::is_same_v<object_array, std::decay_t<decltype(x_arr)>>) {
          return std::equal(
              x_arr.begin(), x_arr.end(),
              y_arr.begin(), y_arr.end(),
              [&ctx](const auto& x, const auto& y) {
                if (x == nullptr || y == nullptr) return x == y;
                return ctx(*x, *y);
              });
        } else {
          return x_arr == y_arr;
        }
      },
      data, y_val.data);
}

auto new_array::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (type == nullptr)
    out << "array (nullptr type) = ";
  else
    out << "array " << indent_(type->name_for_to_string_(), 6)
        << "\n"
        << "values:\n"
        << "-------";

  std::visit(
      [&out, &ctx](const auto& arr_data) {
        for (const auto& v : arr_data) {
          if constexpr(std::is_same_v<object_array, std::decay_t<decltype(arr_data)>>) {
            if (v == nullptr)
              out << "\nnullptr";
            else
              out << "\n" << ctx(*v);
          } else {
            out << "\n" << std::int64_t(v);
          }
        }
      },
      data);
}


auto new_enum::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> cycle_ptr::cycle_gptr<new_enum> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  if (hdr != TC_ENUM) throw std::invalid_argument("hdr");

  auto result = cycle_ptr::allocate_cycle<new_enum>(r.get_allocator());
  result->type = class_desc::decode(r, read);
  r.next_handle(result);
  result->value = stream_string::decode(r, read);
  return result;
}

auto new_enum::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  if (type == nullptr)
    out << "enum (nullptr type) = ";
  else
    out << "enum " << indent_(ctx(*type), 5) << " = ";

  if (value == nullptr)
    out << "(nullptr name)";
  else
    out << ctx(*value);
}

auto new_enum::eq_(const stream_element& y, eq_ctx& ctx) const
-> bool {
  const new_enum& y_val = dynamic_cast<const new_enum&>(y);

  if ((type == nullptr) != (y_val.type == nullptr)) return false;
  if (type != nullptr && !ctx(*type, *y_val.type)) return false;

  if ((value == nullptr) != (y_val.value == nullptr)) return false;
  if (value != nullptr && !ctx(*value, *y_val.value)) return false;

  return true;
}


auto content::decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr)
-> std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  switch (hdr) {
    default:
      throw std::invalid_argument("hdr");
    case TC_BLOCKDATA:
      [[fallthrough]];
    case TC_BLOCKDATALONG:
      return blockdata::decode(r, read, hdr);
    case TC_OBJECT:
      return new_object::decode(r, read, hdr);
    case TC_CLASS:
      return new_class::decode(r, read, hdr);
    case TC_ARRAY:
      return new_array::decode(r, read, hdr);
    case TC_STRING:
      [[fallthrough]];
    case TC_LONGSTRING:
      return stream_string::decode(r, read, hdr);
    case TC_ENUM:
      return new_enum::decode(r, read, hdr);
    case TC_CLASSDESC:
      [[fallthrough]];
    case TC_PROXYCLASSDESC:
      return class_desc::decode(r, read, hdr);
    case TC_REFERENCE:
      {
        std::uint32_t ref;
        read(buffer(&ref, sizeof(ref)));
        big_to_native_inplace(ref);
        return r.template get_handle<const stream_element>(ref);
      }
    case TC_NULL:
      return nullptr;
    case TC_EXCEPTION:
      throw encountered_tc_exception();
  }
}

auto content::decode(reader& r, basic_read_wrapper& read)
-> std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata> {
  using boost::asio::buffer;
  using boost::endian::big_to_native_inplace;

  std::uint8_t type;
  read(buffer(&type, sizeof(type)));
  big_to_native_inplace(type);

  switch (type) {
    default:
      throw decoding_error("unrecognized content magic");
    case TC_BLOCKDATA:
      [[fallthrough]];
    case TC_BLOCKDATALONG:
      [[fallthrough]];
    case TC_OBJECT:
      [[fallthrough]];
    case TC_CLASS:
      [[fallthrough]];
    case TC_ARRAY:
      [[fallthrough]];
    case TC_STRING:
      [[fallthrough]];
    case TC_LONGSTRING:
      [[fallthrough]];
    case TC_ENUM:
      [[fallthrough]];
    case TC_CLASSDESC:
      [[fallthrough]];
    case TC_PROXYCLASSDESC:
      [[fallthrough]];
    case TC_REFERENCE:
      [[fallthrough]];
    case TC_NULL:
      [[fallthrough]];
    case TC_EXCEPTION:
      return decode(r, read, type);
  }
}

auto content::to_out_(std::ostream& out, str_ctx& ctx) const
-> void {
  std::visit(
      [&out, &ctx](const auto& x) {
        if constexpr(std::is_same_v<cycle_ptr::cycle_gptr<const stream_element>, std::decay_t<decltype(x)>>
            || std::is_same_v<cycle_ptr::cycle_member_ptr<const stream_element>, std::decay_t<decltype(x)>>) {
          if (x == nullptr)
            out << "(nullptr object)";
          else
            out << ctx(*x);
        } else {
          out << x;
        }
      },
      *this);
}

auto content::eq_(const content& y, eq_ctx& ctx) const
-> bool {
  return std::visit(
      [&ctx](const auto& x, const auto& y) -> bool {
        if constexpr(!std::is_same_v<std::decay_t<decltype(x)>, std::decay_t<decltype(y)>>) {
          return false;
        } else if constexpr(std::is_same_v<blockdata, std::decay_t<decltype(x)>>) {
          return x == y;
        } else if constexpr(std::is_same_v<cycle_ptr::cycle_gptr<const stream_element>, std::decay_t<decltype(x)>>
            || std::is_same_v<cycle_ptr::cycle_member_ptr<const stream_element>, std::decay_t<decltype(x)>>) {
          if (x == nullptr || y == nullptr) return x == y;
          return ctx(*x, *y);
        }
      },
      *this, y);
}

auto operator<<(std::ostream& out, const content& c)
-> std::ostream& {
  str_ctx ctx;
  return out << ctx(c);
}

auto to_string(const content& c)
-> std::string {
  std::ostringstream oss;
  oss << c;
  return oss.str();
}


} /* namespace java::serialization::stream */
