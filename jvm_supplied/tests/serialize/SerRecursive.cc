#include <java/serialization/encdec.h>
#include "UnitTest++/UnitTest++.h"
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include "support.h"
#include <cycle_ptr/cycle_ptr.h>

using namespace java::serialization::stream;

std::string SER_RECURSIVE_PATH;

TEST(deserialize) {
  const auto expect = cycle_ptr::make_cycle<new_object>();
  const auto expect_cls_desc = cycle_ptr::make_cycle<new_class_desc__class_desc>();
  expect->cls = expect_cls_desc;
  expect_cls_desc->class_name = field_descriptor(u"SerRecursive");
  expect_cls_desc->serial_version_uid = 0x12345u;
  expect_cls_desc->info.flags = SC_SERIALIZABLE;
  expect_cls_desc->info.fields = {
    object_desc(u"SerRecursive", u"self", 0)
  };
  expect_cls_desc->info.annotation = {};
  expect_cls_desc->info.super = nullptr;

  expect->data.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(expect_cls_desc),
      std::forward_as_tuple(
          std::initializer_list<class_desc::field_map::value_type>{
            { stream_string(u"self"), expect }
          }));


  auto deser_obj = read_one(SER_RECURSIVE_PATH);
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
  SER_RECURSIVE_PATH = argv[1];

  return UnitTest::RunAllTests();
}
