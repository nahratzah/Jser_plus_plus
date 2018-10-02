#ifndef JAVA_SERIALIZATION_ENCDEC_H
#define JAVA_SERIALIZATION_ENCDEC_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <scoped_allocator>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <boost/asio/read.hpp>
#include <cycle_ptr/cycle_ptr.h>
#include <cycle_ptr/allocator.h>
#include <java/serialization/exception.h>

namespace java::serialization::stream {


/// Signal that decoding is to stop and a serialized exception is to be processed.
class encountered_tc_exception {};

class basic_read_wrapper {
 protected:
  ~basic_read_wrapper() = default;

 public:
  virtual auto operator()(boost::asio::mutable_buffer buffer) const -> void = 0;
  virtual auto operator()(std::initializer_list<boost::asio::mutable_buffer> buffers) const -> void = 0;
};

class stream_element;

auto to_string(const stream_element& s) -> std::string;
auto operator<<(std::ostream& out, const stream_element& s) -> std::ostream&;
auto operator==(const stream_element& x, const stream_element& y) -> bool;
auto operator!=(const stream_element& x, const stream_element& y) -> bool;

class eq_ctx {
 private:
  struct hasher {
    auto operator()(const std::tuple<const void*, const void*>& v) const noexcept -> std::size_t;
  };

 public:
  eq_ctx() = default;
  eq_ctx(const eq_ctx&) = delete;

  auto operator()(const stream_element& x, const stream_element& y) -> bool;

  template<typename T>
  auto operator()(const T& x, const T& y)
  -> std::enable_if_t<!std::is_base_of_v<stream_element, T>, bool>;

 private:
  std::unordered_set<std::tuple<const void*, const void*>, hasher> recurse_;
};

class str_ctx {
 public:
  template<typename T>
  class proxy {
   public:
    explicit proxy(str_ctx& ctx, const T& v) noexcept
    : ctx_(ctx),
      v_(v)
    {}

    template<typename Char, typename Traits>
    friend auto operator<<(std::basic_ostream<Char, Traits>& out, const proxy& self)
    -> std::basic_ostream<Char, Traits>& {
      if constexpr(std::is_base_of_v<stream_element, std::decay_t<T>>) {
        auto ins = self.ctx_.recurse_.insert(std::addressof(self.v_));
        out << "@" << std::addressof(self.v_);
        if (std::get<1>(ins)) {
          self.v_.to_out_(out << " ", self.ctx_);
          self.ctx_.recurse_.erase(std::get<0>(ins));
        }
      } else {
        self.v_.to_out_(out, self.ctx_);
      }
      return out;
    }

   private:
    str_ctx& ctx_;
    const T& v_;
  };

  str_ctx() = default;
  str_ctx(const str_ctx&) = delete;

  template<typename T>
  auto operator()(const T& x)
  noexcept
  -> proxy<T> {
    return proxy<T>(*this, x);
  }

 private:
  std::unordered_set<const void*> recurse_;
};

template<typename T>
auto to_string(const str_ctx::proxy<T>& proxy)
-> std::string {
  std::ostringstream out;
  out << proxy;
  return out.str();
}

///\brief Base class for stream elements, to facilitate type agnostic handling.
class stream_element {
  friend class eq_ctx;
  friend class str_ctx;
  template<typename> friend class str_ctx::proxy;

 public:
  stream_element() noexcept = default;
  virtual ~stream_element() noexcept = 0;

  auto get_allocator() const
  -> std::allocator<char> {
    return std::allocator<char>();
  }

 private:
  virtual auto to_out_(std::ostream& out, str_ctx& ctx) const -> void = 0;
  virtual auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool = 0;
};

class reader;
struct class_desc;
struct content;

constexpr std::uint16_t MAGIC = 0xaced;
constexpr std::uint16_t VERSION = 5;
constexpr std::uint8_t TC_NULL = 0x70, ///< Nullptr serialization
                       TC_REFERENCE = 0x71, ///< Reference to earlier object in stream
                       TC_CLASSDESC = 0x72, ///< Stream representation of class
                       TC_OBJECT = 0x73, ///< Object instance
                       TC_STRING = 0x74, ///< Short string (max 1<<16-1 encoded bytes)
                       TC_ARRAY = 0x75,
                       TC_CLASS = 0x76, ///< Corresponds to serialization of java.lang.Class
                       TC_BLOCKDATA = 0x77, ///< Opaque bytes
                       TC_ENDBLOCKDATA = 0x78,
                       TC_RESET = 0x79,
                       TC_BLOCKDATALONG = 0x7a, ///< Opaque bytes
                       TC_EXCEPTION = 0x7b,
                       TC_LONGSTRING = 0x7c, ///< Long string (max 1<<64-1 encoded bytes)
                       TC_PROXYCLASSDESC = 0x7d, ///< Stream representation of java.lang.reflect.Proxy based class
                       TC_ENUM = 0x7e;
constexpr std::uint8_t SC_WRITE_METHOD = 0x01,
                       SC_BLOCK_DATA = 0x08,
                       SC_SERIALIZABLE = 0x02,
                       SC_EXTERNALIZABLE = 0x04,
                       SC_ENUM = 0x10;
constexpr std::uint32_t base_wire_handle = 0x7e0000;

enum class primitive_type : std::uint8_t {
  byte_type = u8'B',
  char_type = u8'C',
  double_type = u8'D',
  float_type = u8'F',
  int_type = u8'I',
  long_type = u8'J',
  short_type = u8'S',
  boolean_type = u8'Z'
};

enum class object_type : std::uint8_t {
  array_type = u8'[',
  object_type = u8'L'
};

struct stream_string
: stream_element,
  std::u16string
{
  using allocator_type = std::u16string::allocator_type;

  explicit stream_string(allocator_type alloc = allocator_type())
  : std::u16string(alloc)
  {}

  stream_string(std::u16string_view sv, allocator_type alloc = allocator_type())
  : std::u16string(sv.data(), sv.length(), alloc)
  {}

  stream_string& operator=(std::u16string_view sv) {
    this->assign(sv.data(), sv.length());
    return *this;
  }

  using std::u16string::operator=;

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<const stream_string>;
  static auto decode(reader& r, basic_read_wrapper& read) -> cycle_ptr::cycle_gptr<const stream_string>;

  static auto read_utf(basic_read_wrapper& read) -> stream_string;
  static auto read_long_utf(basic_read_wrapper& read) -> stream_string;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, [[maybe_unused]] eq_ctx& ctx) const -> bool override;
};

class field_descriptor {
 public:
  static constexpr char16_t sep = u'.';

  field_descriptor() = default;

  explicit field_descriptor(primitive_type prim, std::uint32_t extents = 0)
  : extents_(extents),
    type_(std::in_place_type<primitive_type>, prim)
  {
    if (extents > 255) throw std::invalid_argument("too many extents");
  }

  explicit field_descriptor(std::u16string_view cls_name, std::uint32_t extents = 0)
  : extents_(extents),
    type_(std::in_place_type<std::u16string>, cls_name.data(), cls_name.size())
  {
    if (extents > 255) throw std::invalid_argument("too many extents");
  }

  explicit field_descriptor(const char16_t* cls_name, std::uint32_t extents = 0)
  : field_descriptor(std::u16string_view(cls_name), extents)
  {}

  explicit field_descriptor(std::u16string&& cls_name, std::uint32_t extents = 0)
  : extents_(extents),
    type_(std::in_place_type<std::u16string>, std::move(cls_name))
  {
    if (extents > 255) throw std::invalid_argument("too many extents");
  }

  static auto from_string(std::u16string_view s, char16_t sep) -> field_descriptor;
  auto to_string() const -> std::u16string;

  auto extents() const
  noexcept
  -> std::uint32_t {
    return extents_;
  }

  auto type() const
  noexcept
  -> std::variant<primitive_type, std::u16string_view> {
    return std::visit(
        [](const auto& x) -> std::variant<primitive_type, std::u16string_view> {
          return x;
        },
        type_);
  }

  auto is_primitive() const
  noexcept
  -> bool {
    return std::holds_alternative<primitive_type>(type_);
  }

  auto is_object() const
  noexcept
  -> bool {
    return !is_primitive();
  }

  auto is_array() const
  noexcept
  -> bool {
    return extents() != 0;
  }

  auto path() const -> std::vector<std::u16string>;

  auto operator==(const field_descriptor& y) const noexcept
  -> bool {
    return extents_ == y.extents_
        && type_ == y.type_;
  }

  auto operator!=(const field_descriptor& y) const noexcept
  -> bool {
    return !(*this == y);
  }

 private:
  std::uint32_t extents_;
  std::variant<primitive_type, std::u16string> type_;
};

auto operator<<(std::ostream& out, const field_descriptor& fd) -> std::ostream&;
auto to_string(const field_descriptor& fd) -> std::string;


} /* namespace java::serialization::stream */

namespace std {


template<>
struct hash<java::serialization::stream::stream_string>
: public hash<u16string_view>
{};


} /* namespace std */

namespace java::serialization::stream {


using referent = cycle_ptr::cycle_gptr<const stream_element>;


struct blockdata
: std::vector<std::uint8_t>
{
  using std::vector<std::uint8_t>::vector;
  using std::vector<std::uint8_t>::operator=;

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> blockdata;
};

auto operator<<(std::ostream& out, const blockdata& x) -> std::ostream&;
auto to_string(const blockdata& x) -> std::string;


struct primitive_desc {
  // XXX use field_desc::value_type
  using value_type = std::variant<
      std::int8_t,
      char16_t,
      double,
      float,
      std::int32_t,
      std::int64_t,
      std::int16_t,
      bool,
      cycle_ptr::cycle_gptr<const stream_element>>;

  primitive_desc() = default;

  primitive_desc(primitive_type type, stream_string field_name)
  : type(type),
    field_name(std::move(field_name))
  {}

  primitive_desc(primitive_type type, std::u16string_view field_name)
  : primitive_desc(type, stream_string(field_name))
  {}

  auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> primitive_desc&;
  auto decode_field(reader& r, basic_read_wrapper& read) const -> value_type;
  static auto decode_field(primitive_type type, reader& r, basic_read_wrapper& read) -> value_type;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const primitive_desc& y, eq_ctx& ctx) const -> bool;

  primitive_type type;
  stream_string field_name;
};

auto operator==(const primitive_desc& x, const primitive_desc& y) -> bool;
auto operator!=(const primitive_desc& x, const primitive_desc& y) -> bool;

auto to_string(const primitive_desc& desc) -> std::string;
auto operator<<(std::ostream& out, const primitive_desc& desc) -> std::ostream&;


struct object_desc {
  // XXX use field_desc::value_type
  using value_type = std::variant<
      std::int8_t,
      char16_t,
      double,
      float,
      std::int32_t,
      std::int64_t,
      std::int16_t,
      bool,
      cycle_ptr::cycle_gptr<const stream_element>>;

  object_desc() = default;

  explicit object_desc(field_descriptor type, std::u16string_view name)
  : type(type.extents() == 0 ? object_type::object_type : object_type::array_type),
    field_name(name),
    field_type(std::move(type))
  {}

  explicit object_desc(std::u16string_view type, std::u16string_view name, std::uint32_t extents)
  : object_desc(field_descriptor(type, extents), name)
  {}

  explicit object_desc(primitive_type type, std::u16string_view name, std::uint32_t extents)
  : object_desc(field_descriptor(type, extents), name)
  {}

  auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> object_desc&;
  auto decode_field(reader& r, basic_read_wrapper& read) const -> value_type;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const object_desc& y, eq_ctx& ctx) const -> bool;

  object_type type;
  stream_string field_name;
  field_descriptor field_type;
};

auto operator==(const object_desc& x, const object_desc& y) -> bool;
auto operator!=(const object_desc& x, const object_desc& y) -> bool;

auto to_string(const object_desc& desc) -> std::string;
auto operator<<(std::ostream& out, const object_desc& desc) -> std::ostream&;


struct field_desc // XXX make interface for primitive_desc, object_desc
: std::variant<primitive_desc, object_desc>
{
  using value_type = std::variant<
      std::int8_t,
      char16_t,
      double,
      float,
      std::int32_t,
      std::int64_t,
      std::int16_t,
      bool,
      cycle_ptr::cycle_gptr<const stream_element>>;

  using std::variant<primitive_desc, object_desc>::variant;
  using std::variant<primitive_desc, object_desc>::operator=;

  auto decode(reader& r, basic_read_wrapper& read) -> field_desc&;
  auto decode_field(reader& r, basic_read_wrapper& read) const -> value_type;

  auto name() const -> std::u16string_view {
    return std::visit(
        [](const auto& desc) -> std::u16string_view { return desc.field_name; },
        *this);
  }

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const field_desc& y, eq_ctx& ctx) const -> bool;
};

auto operator==(const field_desc& x, const field_desc& y) -> bool;
auto operator!=(const field_desc& x, const field_desc& y) -> bool;

auto to_string(const field_desc& desc) -> std::string;
auto operator<<(std::ostream& out, const field_desc& desc) -> std::ostream&;


struct new_object
: stream_element,
  private cycle_ptr::cycle_base
{
  struct class_data {
    using allocator_type = std::scoped_allocator_adaptor<cycle_ptr::cycle_allocator<std::allocator<char>>>;
    using field_value = std::variant<
        std::int8_t,
        char16_t,
        double,
        float,
        std::int32_t,
        std::int64_t,
        std::int16_t,
        bool,
        cycle_ptr::cycle_member_ptr<const stream_element>>;
    using field_map = std::unordered_map<
        stream_string,
        field_value,
        std::hash<stream_string>,
        std::equal_to<stream_string>,
        typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const stream_string, field_value>>>;

    using annotation_type = std::variant<
        cycle_ptr::cycle_member_ptr<const stream_element>,
        blockdata>;
    using annotation_vector = std::vector<
        annotation_type,
        typename std::allocator_traits<allocator_type>::template rebind_alloc<annotation_type>>;

    using field_initializer_list = std::initializer_list<
        std::pair<
            std::u16string,
            std::variant<
                std::int8_t,
                char16_t,
                double,
                float,
                std::int32_t,
                std::int64_t,
                std::int16_t,
                bool,
                cycle_ptr::cycle_member_ptr<const stream_element>>>>;
    using annotation_initializer_list = std::initializer_list<
        std::variant<
            cycle_ptr::cycle_gptr<const stream_element>,
            blockdata>>;

    explicit class_data(allocator_type alloc)
    : fields(alloc),
      annotation(alloc)
    {}

    class_data(class_data&&) = delete;
    class_data(const class_data&) = delete;

    explicit class_data(const class_data& cd, allocator_type alloc)
    : fields(cd.fields, alloc),
      annotation(cd.annotation, alloc)
    {}

    explicit class_data(class_data&& cd, allocator_type alloc)
    : fields(std::move(cd.fields), alloc),
      annotation(std::move(cd.annotation), alloc)
    {}

    template<typename FieldMap>
    class_data(const FieldMap& fields, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
    }

    class_data(field_initializer_list fields, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
    }

    template<typename FieldMap, typename AnnotationVector>
    class_data(const FieldMap& fields, const AnnotationVector& annotation, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
      assign_annotation(annotation);
    }

    template<typename AnnotationVector>
    class_data(field_initializer_list fields, const AnnotationVector& annotation, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
      assign_annotation(annotation);
    }

    template<typename FieldMap>
    class_data(const FieldMap& fields, annotation_initializer_list annotation, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
      assign_annotation(annotation);
    }

    class_data(field_initializer_list fields, annotation_initializer_list annotation, allocator_type alloc)
    : class_data(alloc)
    {
      assign_fields(fields);
      assign_annotation(annotation);
    }

    field_map fields;
    annotation_vector annotation;

    auto eq_(const class_data& y, eq_ctx& ctx) const -> bool;
    auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
    auto operator==(const class_data& y) const -> bool;
    auto operator!=(const class_data& y) const -> bool;

    friend
    auto operator<<(std::ostream& out, const class_data& elem)
    -> std::ostream& {
      str_ctx ctx;
      out << ctx(elem);
      return out;
    }

    friend
    auto to_string(const class_data& elem)
    -> std::string {
      std::ostringstream oss;
      oss << elem;
      return oss.str();
    }

    template<typename FieldMap>
    auto assign_fields(const FieldMap& new_fields)
    -> void {
      fields.clear();
      fields.reserve(new_fields.size());

      std::for_each(
          new_fields.begin(), new_fields.end(),
          [this](const auto& pair) {
            std::visit(
                [this, &pair](const auto& v) {
                  if constexpr(std::is_convertible_v<std::decay_t<decltype(v)>, cycle_ptr::cycle_gptr<const stream_element>>)
                    fields.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(pair.first),
                        std::forward_as_tuple(std::in_place_type<cycle_ptr::cycle_member_ptr<const stream_element>>, v));
                  else
                    fields.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(pair.first),
                        std::forward_as_tuple(std::in_place_type<std::decay_t<decltype(v)>>, v));
                },
                pair.second);
          });
    }

    template<typename AnnotationVector>
    auto assign_annotation(const AnnotationVector& new_annotation)
    -> void {
      annotation.clear();
      annotation.reserve(new_annotation.size());

      std::for_each(
          new_annotation.begin(), new_annotation.end(),
          [this](const auto& v) {
            std::visit(
                [this](const auto& v) {
                  if constexpr(std::is_convertible_v<std::decay_t<decltype(v)>, cycle_ptr::cycle_gptr<const stream_element>>)
                    annotation.emplace_back(std::in_place_type<cycle_ptr::cycle_member_ptr<const stream_element>>, v);
                  else
                    annotation.emplace_back(std::in_place_type<std::decay_t<decltype(v)>>, v);
                },
                v);
          });
    }
  };

  using field_value = class_data::field_value;
  using field_map = class_data::field_map;
  using field_initializer_list = class_data::field_initializer_list;
  using annotation_type = class_data::annotation_type;
  using annotation_vector = class_data::annotation_vector;
  using annotation_initializer_list = class_data::annotation_initializer_list;

  new_object()
  : data(cycle_ptr::cycle_allocator<std::allocator<void>>(*this))
  {}

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_object>;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;

  cycle_ptr::cycle_member_ptr<const class_desc> cls;
  std::map<
      cycle_ptr::cycle_member_ptr<const class_desc>,
      class_data,
      std::less<>,
      typename std::allocator_traits<std::scoped_allocator_adaptor<cycle_ptr::cycle_allocator<std::allocator<void>>>>::template rebind_alloc<
          std::pair<const cycle_ptr::cycle_member_ptr<const class_desc>, class_data>>
      > data;
};


// XXX Maybe rename to 'annotation' and re-use in new_object?
struct class_annotation
: new_object::annotation_vector,
  cycle_ptr::cycle_base
{
  using new_object::annotation_vector::operator=;

  explicit class_annotation()
  : new_object::annotation_vector(cycle_ptr::cycle_allocator<std::allocator<void>>(*this))
  {}

  class_annotation(const class_annotation& other)
  : new_object::annotation_vector(other, cycle_ptr::cycle_allocator<std::allocator<void>>(*this))
  {}

  class_annotation(class_annotation&& other)
  : new_object::annotation_vector(std::move(other), cycle_ptr::cycle_allocator<std::allocator<void>>(*this))
  {}

  class_annotation(new_object::annotation_initializer_list il)
  : new_object::annotation_vector(cycle_ptr::cycle_allocator<std::allocator<void>>(*this))
  {
    clear();
    reserve(il.size());

    std::for_each(
        il.begin(), il.end(),
        [this](const auto& v) {
          std::visit(
              [this](const auto& v) {
                if constexpr(std::is_convertible_v<std::decay_t<decltype(v)>, cycle_ptr::cycle_gptr<const stream_element>>)
                  emplace_back(std::in_place_type<cycle_ptr::cycle_member_ptr<const stream_element>>, v);
                else
                  emplace_back(std::in_place_type<std::decay_t<decltype(v)>>, v);
              },
              v);
        });
  }

  auto decode(reader& r, basic_read_wrapper& read) -> class_annotation&;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const class_annotation& y, eq_ctx& ctx) const -> bool;
};

auto to_string(const class_annotation& annot) -> std::string;
auto operator<<(std::ostream& out, const class_annotation& annot) -> std::ostream&;


struct class_desc_info {
  using field_map = std::vector<std::pair<stream_string, field_desc::value_type>>;
  using obj_content = std::vector<std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata>>;

  class_desc_info() = default;

  explicit class_desc_info(cycle_ptr::cycle_gptr<const class_desc> super, std::uint8_t flags = 0, std::initializer_list<field_desc> fields = {})
  : super(std::move(super)),
    flags(flags),
    fields(std::move(fields))
  {}

  auto decode(reader& r, basic_read_wrapper& read) -> class_desc_info&;
  auto decode_fields(reader& r, basic_read_wrapper& read) const -> field_map;
  auto decode_annotation(reader& r, basic_read_wrapper& read) const -> obj_content;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const class_desc_info& y, eq_ctx& ctx) const -> bool;

  std::uint8_t flags = 0;
  std::vector<field_desc> fields;
  class_annotation annotation;
  cycle_ptr::cycle_member_ptr<const class_desc> super;
};

auto operator==(const class_desc_info& x, const class_desc_info& y) -> bool;
auto operator!=(const class_desc_info& x, const class_desc_info& y) -> bool;

auto to_string(const class_desc_info& info) -> std::string;
auto operator<<(std::ostream& out, const class_desc_info& info) -> std::ostream&;


struct class_desc
: stream_element
{
  using field_map = class_desc_info::field_map;
  using obj_content = class_desc_info::obj_content;

  explicit class_desc() {}
  ~class_desc() noexcept override;

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<const class_desc>;
  static auto decode(reader& r, basic_read_wrapper& read) -> cycle_ptr::cycle_gptr<const class_desc>;

  virtual auto get_super() const -> cycle_ptr::cycle_gptr<const class_desc> = 0;
  virtual auto decode_fields(reader& r, basic_read_wrapper& read) const -> field_map = 0;
  virtual auto decode_annotation(reader& r, basic_read_wrapper& read) const -> obj_content = 0;
  virtual auto name_for_to_string_() const -> std::string = 0;
};


struct new_class_desc__class_desc
: class_desc
{
  new_class_desc__class_desc() = default;

  new_class_desc__class_desc(field_descriptor class_name, std::uint64_t serial_version_uid, cycle_ptr::cycle_gptr<const class_desc> super = nullptr, std::uint8_t flags = 0u, std::initializer_list<field_desc> fields = {});

  new_class_desc__class_desc(std::u16string_view class_name, std::uint64_t serial_version_uid, std::uint32_t extents)
  : class_name(class_name, extents),
    serial_version_uid(serial_version_uid)
  {}

  new_class_desc__class_desc(primitive_type class_name, std::uint64_t serial_version_uid, std::uint32_t extents)
  : class_name(class_name, extents),
    serial_version_uid(serial_version_uid)
  {}

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_class_desc__class_desc>;

  auto get_super() const
  -> cycle_ptr::cycle_gptr<const class_desc> override {
    return info.super;
  }

  auto decode_fields(reader& r, basic_read_wrapper& read) const
  -> typename class_desc::field_map override {
    return info.decode_fields(r, read);
  }

  auto decode_annotation(reader& r, basic_read_wrapper& read) const
  -> typename class_desc::obj_content override {
    return info.decode_annotation(r, read);
  }

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;
  auto name_for_to_string_() const -> std::string override { return to_string(class_name); }

  field_descriptor class_name;
  std::uint64_t serial_version_uid;
  class_desc_info info;
};


struct proxy_class_desc_info {
  auto decode(reader& r, basic_read_wrapper& read) -> proxy_class_desc_info&;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const proxy_class_desc_info& y, eq_ctx& ctx) const -> bool;

  std::vector<stream_string> proxy_interface_names;
  class_annotation annotation;
  cycle_ptr::cycle_member_ptr<const class_desc> super;
};

auto operator==(const proxy_class_desc_info& x, const proxy_class_desc_info& y) -> bool;
auto operator!=(const proxy_class_desc_info& x, const proxy_class_desc_info& y) -> bool;

auto to_string(const proxy_class_desc_info& data) -> std::string;
auto operator<<(std::ostream& out, const proxy_class_desc_info& data) -> std::ostream&;


struct new_class_desc__proxy_class_desc
: class_desc
{
  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_class_desc__proxy_class_desc>;

  auto get_super() const -> cycle_ptr::cycle_gptr<const class_desc> override;
  auto decode_fields(reader& r, [[maybe_unused]] basic_read_wrapper& read) const -> class_desc::field_map override;
  auto decode_annotation(reader& r, basic_read_wrapper& read) const -> class_desc::obj_content override;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;
  auto name_for_to_string_() const -> std::string override { return "proxy"; }

  proxy_class_desc_info desc;
};


struct new_class
: stream_element,
  cycle_ptr::cycle_member_ptr<const class_desc>
{
  using cycle_ptr::cycle_member_ptr<const class_desc>::cycle_member_ptr;
  using cycle_ptr::cycle_member_ptr<const class_desc>::operator=;

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_class>;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;
};


struct new_array
: stream_element,
  cycle_ptr::cycle_base
{
  using byte_array = std::vector<std::int8_t>;
  using char_array = std::vector<char16_t>;
  using double_array = std::vector<double>;
  using float_array = std::vector<float>;
  using int_array = std::vector<std::int32_t>;
  using long_array = std::vector<std::int64_t>;
  using short_array = std::vector<std::int16_t>;
  using bool_array = std::vector<bool>;
  using object_array = std::vector<
      cycle_ptr::cycle_member_ptr<const stream_element>,
      typename cycle_ptr::cycle_allocator<std::allocator<cycle_ptr::cycle_member_ptr<const stream_element>>>>;
  using array_variant = std::variant<
      byte_array,
      char_array,
      double_array,
      float_array,
      int_array,
      long_array,
      short_array,
      bool_array,
      object_array>;

  new_array() = default;

  template<typename T, typename U>
  explicit new_array(std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, const std::initializer_list<U>& values)
  : new_array(ip, type, values.begin(), values.end())
  {}

  template<typename T, typename U>
  explicit new_array(std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, std::initializer_list<U>&& values)
  : new_array(ip, type, std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()))
  {}

  template<typename T, typename U>
  explicit new_array(std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, const U& values)
  : new_array(ip, type, values.begin(), values.end())
  {}

  template<typename T, typename U, std::enable_if_t<!std::is_const_v<U>>>
  explicit new_array(std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, U&& values)
  : new_array(ip, type, std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()))
  {}

  template<typename T, typename Iter>
  explicit new_array([[maybe_unused]] std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, Iter b, Iter e)
  : type(std::move(type)),
    data(std::in_place_type<std::vector<T>>, b, e)
  {}

  template<typename Iter>
  explicit new_array([[maybe_unused]] std::in_place_type_t<referent> ip, cycle_ptr::cycle_gptr<const class_desc> type, Iter b, Iter e)
  : type(std::move(type)),
    data(std::in_place_type<object_array>, b, e, object_array::allocator_type(*this))
  {}

  template<typename T, typename Iter>
  auto assign([[maybe_unused]] std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, Iter b, Iter e)
  -> void {
    this->type = std::move(type);
    this->data.emplace<std::vector<T>>(b, e);
  }

  template<typename T, typename Iter>
  auto assign([[maybe_unused]] std::in_place_type_t<referent> ip, cycle_ptr::cycle_gptr<const class_desc> type, Iter b, Iter e)
  -> void {
    this->type = std::move(type);
    this->data.emplace<std::vector<object_array>>(b, e, object_array::allocator_type(*this));
  }

  template<typename T, typename U>
  auto assign([[maybe_unused]] std::in_place_type_t<T> ip, cycle_ptr::cycle_gptr<const class_desc> type, const U& values)
  -> void {
    this->type = std::move(type);
    this->data.emplace<std::vector<T>>(values.begin(), values.end());
  }

  template<typename T, typename U>
  auto assign([[maybe_unused]] std::in_place_type_t<referent> ip, cycle_ptr::cycle_gptr<const class_desc> type, U&& values)
  -> void {
    this->type = std::move(type);
    this->data.emplace<std::vector<object_array>>(std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()), object_array::allocator_type(*this));
  }

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_array>;

  auto size() const noexcept
  -> std::uint32_t {
    return std::visit(
        [](const auto& v) -> std::uint32_t { return v.size(); },
        data);
  }

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;

  cycle_ptr::cycle_member_ptr<const class_desc> type;
  array_variant data;
};


struct new_enum
: stream_element
{
  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> cycle_ptr::cycle_gptr<new_enum>;

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void override;
  auto eq_(const stream_element& y, eq_ctx& ctx) const -> bool override;

  cycle_ptr::cycle_gptr<const class_desc> type;
  cycle_ptr::cycle_gptr<const stream_string> value;
};

struct content
: std::variant<
    cycle_ptr::cycle_member_ptr<const stream_element>,
    blockdata>,
  cycle_ptr::cycle_base
{
  content()
  : content(nullptr)
  {}

  explicit content([[maybe_unused]] std::nullptr_t ptr)
  : std::variant<cycle_ptr::cycle_member_ptr<const stream_element>, blockdata>(std::in_place_index<0>, *this, nullptr)
  {}

  template<typename T>
  explicit content(cycle_ptr::cycle_gptr<T> ptr)
  : std::variant<cycle_ptr::cycle_member_ptr<const stream_element>, blockdata>(std::in_place_index<0>, *this, std::move(ptr))
  {}

  explicit content(blockdata data)
  : std::variant<cycle_ptr::cycle_member_ptr<const stream_element>, blockdata>(std::in_place_index<1>, std::move(data))
  {}

  static auto decode(reader& r, basic_read_wrapper& read, std::uint8_t hdr) -> std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata>;
  static auto decode(reader& r, basic_read_wrapper& read) -> std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata>;

  auto is_object() const
  noexcept
  -> bool {
    return std::holds_alternative<cycle_ptr::cycle_member_ptr<const stream_element>>(*this);
  }

  auto is_blockdata() const
  noexcept
  -> bool {
    return std::holds_alternative<blockdata>(*this);
  }

  auto as_object() const
  -> cycle_ptr::cycle_gptr<const stream_element> {
    return std::get<cycle_ptr::cycle_member_ptr<const stream_element>>(*this);
  }

  auto as_blockdata() const
  -> const blockdata& {
    return std::get<blockdata>(*this);
  }

  auto to_out_(std::ostream& out, str_ctx& ctx) const -> void;
  auto eq_(const content& y, eq_ctx& ctx) const -> bool;
};

auto operator<<(std::ostream& out, const content& c) -> std::ostream&;
auto to_string(const content& c) -> std::string;


template<typename Stream>
class read_wrapper_
: public basic_read_wrapper
{
  static_assert(std::is_same_v<
      void,
      std::void_t<decltype(std::declval<Stream&>().read_some(std::declval<std::array<boost::asio::mutable_buffer, 1>&>()))>>);

 public:
  explicit read_wrapper_(Stream& stream) noexcept
  : stream_(stream)
  {}

  auto operator()(boost::asio::mutable_buffer buffer) const
  -> void override {
    boost::asio::read(stream_, buffer);
  }

  auto operator()(std::initializer_list<boost::asio::mutable_buffer> buffers) const
  -> void override {
    boost::asio::read(stream_, buffers);
  }

 private:
  Stream& stream_;
};

class iostream_read_wrapper_
: public basic_read_wrapper
{
 public:
  explicit iostream_read_wrapper_(std::istream& stream) noexcept
  : stream_(stream)
  {}

  auto operator()(boost::asio::mutable_buffer buffer) const
  -> void override {
    if (!stream_)
      throw decoding_error("stream is bad");

    stream_.read(static_cast<char*>(buffer.data()), buffer.size());
    if (stream_.fail())
      throw decoding_error("read failed");
    if (static_cast<std::size_t>(stream_.gcount()) == buffer.size())
      return;

    throw decoding_error("insufficient data");
  }

  auto operator()(std::initializer_list<boost::asio::mutable_buffer> buffers) const
  -> void override {
    for (auto& buf : buffers)
      (*this)(buf);
  }

 private:
  std::istream& stream_;
};

template<typename Stream, typename Yield>
class co_read_wrapper_
: public basic_read_wrapper
{
 public:
  explicit co_read_wrapper_(Stream& stream, Yield& yield) noexcept
  : stream_(stream),
    yield_(yield)
  {}

  auto operator()(boost::asio::mutable_buffer buffer) const
  -> void override {
    return boost::asio::async_read(stream_, buffer, yield_);
  }

  auto operator()(std::initializer_list<boost::asio::mutable_buffer> buffers) const
  -> void override {
    boost::asio::async_read(stream_, buffers, yield_);
  }

 private:
  Stream& stream_;
  Yield& yield_;
};

///\brief Helper type, that wraps calls to (synchronous) read.
template<typename Stream>
auto read_wrapper(Stream& stream) noexcept
-> std::enable_if_t<
      std::is_same_v<void, std::void_t<decltype(std::declval<Stream&>().read_some(std::declval<std::array<boost::asio::mutable_buffer, 1>&>()))>>,
      read_wrapper_<Stream>> {
  return read_wrapper_<Stream>(stream);
}

///\brief Helper type, that wraps calls to read_async, using the suplied yield.
template<typename Stream, typename Yield>
auto read_wrapper(Stream& stream, Yield& yield) noexcept
-> co_read_wrapper_<Stream, Yield> {
  return co_read_wrapper_<Stream, Yield>(stream, yield);
}

inline auto read_wrapper(std::istream& stream) noexcept
-> iostream_read_wrapper_ {
  return iostream_read_wrapper_(stream);
}


class reader {
  friend struct stream_string;
  friend struct content;
  friend struct new_object;
  friend struct new_class;
  friend struct new_class_desc__class_desc;
  friend struct new_class_desc__proxy_class_desc;
  friend struct class_desc;
  friend struct new_array;
  friend struct new_enum;

 public:
  using allocator_type = std::allocator<char>;

 private:
  using referent_vector = std::vector<cycle_ptr::cycle_gptr<const stream_element>>;

 public:
  class content_type
  : public std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata>
  {
   public:
    using variant_type = std::variant<cycle_ptr::cycle_gptr<const stream_element>, blockdata>;

    content_type() = default;

    explicit content_type(
        cycle_ptr::cycle_gptr<const stream_element> c,
        bool is_exception)
    : variant_type(std::move(c)),
      is_exception_(is_exception)
    {}

    explicit content_type(blockdata&& c)
    : variant_type(std::move(c))
    {}

    explicit content_type(
        blockdata&& c,
        bool is_exception)
    : content_type(std::move(c))
    {
      if (is_exception)
        throw std::invalid_argument("exception must be an object");
    }

    auto is_exception() const
    noexcept
    -> bool {
      return is_exception_;
    }

    auto is_object() const
    noexcept
    -> bool {
      return std::holds_alternative<cycle_ptr::cycle_gptr<const stream_element>>(*this);
    }

    auto is_blockdata() const
    noexcept
    -> bool {
      return std::holds_alternative<blockdata>(*this);
    }

    auto as_object() const
    -> cycle_ptr::cycle_gptr<const stream_element> {
      return std::get<cycle_ptr::cycle_gptr<const stream_element>>(*this);
    }

    auto as_blockdata() const
    -> blockdata {
      return std::get<blockdata>(*this);
    }

    friend
    auto operator<<(std::ostream& out, const content_type& c)
    -> std::ostream& {
      if (c.is_exception()) out << "exception ";
      if (c.is_blockdata()) out << c.as_blockdata();
      if (c.is_object()) {
        auto obj_ptr = c.as_object();
        if (obj_ptr == nullptr)
          out << "nullptr";
        else
          out << *obj_ptr;
      }
      return out;
    }

   private:
    bool is_exception_ = false;
  };

  explicit reader(allocator_type alloc = allocator_type())
  : referents_(alloc)
  {}

  auto get_allocator() const
  -> allocator_type {
    return allocator_type();
  }

  template<typename Stream>
  auto read(Stream& stream)
  -> content_type {
    auto wrapper = read_wrapper(stream);
    return read_(wrapper);
  }

  template<typename Stream, typename Yield>
  auto co_read(Stream& stream, Yield& yield)
  -> content_type {
    auto wrapper = read_wrapper(stream, yield);
    return read_(wrapper);
  }

 private:
  auto read_(basic_read_wrapper& read) -> content_type;

  auto next_handle(cycle_ptr::cycle_gptr<const stream_element> v) -> std::uint32_t;
  template<typename T>
  auto get_handle(std::uint32_t idx) const -> cycle_ptr::cycle_gptr<const T>;

  bool header_consumed_ = false;
  referent_vector referents_;
};


} /* namespace java::serialization::stream */

#include "encdec-inl.h"

#endif /* JAVA_SERIALIZATION_ENCDEC_H */
