#include <java/serialization/encdec.h>
#include "UnitTest++/UnitTest++.h"
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include "support.h"
#include <cycle_ptr/cycle_ptr.h>

using namespace java::serialization::stream;

std::string SER_ENUM_PATH;

TEST(deserialize) {
  const auto super_cls_desc = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  super_cls_desc->class_name = field_descriptor(u"java.lang.Enum");
  super_cls_desc->serial_version_uid = 0x0u;
  super_cls_desc->info.flags = SC_SERIALIZABLE | SC_ENUM;
  super_cls_desc->info.annotation = {};
  super_cls_desc->info.super = nullptr;

  const auto expect = cycle_ptr::make_cycle<new_enum>();
  const auto expect_cls_desc = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  expect->type = expect_cls_desc;
  expect_cls_desc->class_name = field_descriptor(u"SerEnum$Impl");
  expect_cls_desc->serial_version_uid = 0x0u;
  expect_cls_desc->info.flags = SC_SERIALIZABLE | SC_ENUM;
  expect_cls_desc->info.annotation = {};
  expect_cls_desc->info.super = super_cls_desc;

  expect->value = cycle_ptr::make_cycle<stream_string>(u"ENUM_VALUE");


  auto deser_obj = read_one(SER_ENUM_PATH);
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
  SER_ENUM_PATH = argv[1];

  return UnitTest::RunAllTests();
}
