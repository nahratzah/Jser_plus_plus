#include <java/serialization/encdec.h>
#include "UnitTest++/UnitTest++.h"
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include "support.h"
#include <cycle_ptr/cycle_ptr.h>

using namespace java::serialization::stream;

std::string SER_FIELD_PATH;

TEST(deserialize) {
  const auto expect = cycle_ptr::make_cycle<new_object>();
  const auto expect_cls_desc = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  expect->cls = expect_cls_desc;
  expect_cls_desc->class_name = field_descriptor(u"SerField");
  expect_cls_desc->serial_version_uid = 0x12345u;
  expect_cls_desc->info.flags = SC_SERIALIZABLE;
  expect_cls_desc->info.fields = {
    primitive_desc(primitive_type::byte_type, u"byteField"),
    primitive_desc(primitive_type::short_type, u"shortField"),
    primitive_desc(primitive_type::int_type, u"intField"),
    primitive_desc(primitive_type::long_type, u"longField"),
    primitive_desc(primitive_type::float_type, u"floatField"),
    primitive_desc(primitive_type::double_type, u"doubleField"),
    object_desc(u"java.lang.String", u"stringField", 0),
    object_desc(u"java.lang.Object", u"nullField", 0)
  };
  expect_cls_desc->info.annotation = {};
  expect_cls_desc->info.super = nullptr;

  const auto stringFieldValue = cycle_ptr::make_cycle<stream_string>(u"six");

  expect->data.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(expect_cls_desc),
      std::forward_as_tuple(
          std::initializer_list<class_desc::field_map::value_type>{
            { stream_string(u"byteField"), primitive_desc::value_type(std::in_place_type<std::int8_t>, 0) },
            { stream_string(u"shortField"), primitive_desc::value_type(std::in_place_type<std::int16_t>, 1) },
            { stream_string(u"intField"), primitive_desc::value_type(std::in_place_type<std::int32_t>, 2) },
            { stream_string(u"longField"), primitive_desc::value_type(std::in_place_type<std::int64_t>, 3) },
            { stream_string(u"floatField"), primitive_desc::value_type(std::in_place_type<float>, 4) },
            { stream_string(u"doubleField"), primitive_desc::value_type(std::in_place_type<double>, 5) },
            { stream_string(u"stringField"), object_desc::value_type(stringFieldValue) },
            { stream_string(u"nullField"), object_desc::value_type(std::in_place_type<cycle_ptr::cycle_gptr<const stream_element>>) }
          }));


  auto deser_obj = read_one(SER_FIELD_PATH);
  REQUIRE CHECK(deser_obj.is_object());
  REQUIRE CHECK(!deser_obj.is_exception());
  auto element_ptr = deser_obj.as_object();
  REQUIRE CHECK(element_ptr != nullptr);
  CHECK_EQUAL(*expect, *element_ptr);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Require argument: path to serialized file SerClass.ser\n";
    return 1;
  }
  SER_FIELD_PATH = argv[1];

  return UnitTest::RunAllTests();
}
