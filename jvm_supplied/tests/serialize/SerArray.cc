#include <java/serialization/encdec.h>
#include "UnitTest++/UnitTest++.h"
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include "support.h"
#include <cycle_ptr/cycle_ptr.h>

using namespace java::serialization::stream;

std::string SER_ARRAY_PATH;

TEST(deserialize) {
  using alloc_t = std::allocator<char>;

  const auto expect = cycle_ptr::make_cycle<new_object>();
  const auto expect_cls_desc = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  expect->cls = expect_cls_desc;
  expect_cls_desc->class_name = field_descriptor(u"SerArray");
  expect_cls_desc->serial_version_uid = 0x12345u;
  expect_cls_desc->info.flags = SC_SERIALIZABLE;
  expect_cls_desc->info.fields = {
    object_desc(primitive_type::byte_type, u"byteField", 1),
    object_desc(primitive_type::short_type, u"shortField", 1),
    object_desc(primitive_type::int_type, u"intField", 1),
    object_desc(primitive_type::long_type, u"longField", 1),
    object_desc(primitive_type::float_type, u"floatField", 1),
    object_desc(primitive_type::double_type, u"doubleField", 1),
    object_desc(u"java.lang.String", u"stringField", 1),
    object_desc(u"java.lang.Object", u"nullField", 1)
  };
  expect_cls_desc->info.annotation = {};
  expect_cls_desc->info.super = nullptr;

  const auto stringFieldValues = {
    cycle_ptr::make_cycle<stream_string>(u"eighteen"),
    cycle_ptr::make_cycle<stream_string>(u"nineteen"),
    cycle_ptr::make_cycle<stream_string>(u"twenty")
  };

  const auto expect_byteField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::byte_type, 0, 1);
  expect_byteField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_byteField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<std::int8_t>,
          expect_byteField_type,
          std::initializer_list<int>{ 0, 1, 2 });

  const auto expect_shortField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::short_type, 0, 1);
  expect_shortField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_shortField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<std::int16_t>,
          expect_shortField_type,
          std::initializer_list<int>{ 3, 4, 5 });

  const auto expect_intField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::int_type, 0, 1);
  expect_intField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_intField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<std::int32_t>,
          expect_intField_type,
          std::initializer_list<int>{ 6, 7, 8 });

  const auto expect_longField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::long_type, 0, 1);
  expect_longField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_longField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<std::int64_t>,
          expect_longField_type,
          std::initializer_list<int>{ 9, 10, 11 });

  const auto expect_floatField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::float_type, 0, 1);
  expect_floatField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_floatField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<float>,
          expect_floatField_type,
          std::initializer_list<int>{ 12, 13, 14 });

  const auto expect_doubleField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(primitive_type::double_type, 0, 1);
  expect_doubleField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_doubleField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<double>,
          expect_doubleField_type,
          std::initializer_list<int>{ 15, 16, 17 });

  const auto expect_stringField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(u"java.lang.String", 0, 1);
  expect_stringField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_stringField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<referent>,
          expect_stringField_type,
          stringFieldValues);

  const auto expect_nullField_type =
      cycle_ptr::make_cycle<new_class_desc__class_desc>(u"java.lang.Object", 0, 1);
  expect_nullField_type->info.flags = SC_SERIALIZABLE;
  const auto expect_nullField = cycle_ptr::make_cycle<new_array>(
          std::in_place_type<referent>,
          expect_nullField_type,
          std::initializer_list<std::nullptr_t>{ nullptr, nullptr, nullptr });

  expect->data.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(expect_cls_desc),
      std::forward_as_tuple(
          std::initializer_list<class_desc::field_map::value_type>{
            { stream_string(u"byteField"), object_desc::value_type(expect_byteField) },
            { stream_string(u"shortField"), object_desc::value_type(expect_shortField) },
            { stream_string(u"intField"), object_desc::value_type(expect_intField) },
            { stream_string(u"longField"), object_desc::value_type(expect_longField) },
            { stream_string(u"floatField"), object_desc::value_type(expect_floatField) },
            { stream_string(u"doubleField"), object_desc::value_type(expect_doubleField) },
            { stream_string(u"stringField"), object_desc::value_type(expect_stringField) },
            { stream_string(u"nullField"), object_desc::value_type(expect_nullField) }
          }));


  auto deser_obj = read_one(SER_ARRAY_PATH);
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
  SER_ARRAY_PATH = argv[1];

  return UnitTest::RunAllTests();
}
