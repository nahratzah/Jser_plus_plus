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
  auto deser_obj = read_one(SER_FIELD_PATH);
  REQUIRE CHECK(deser_obj.is_object());
  REQUIRE CHECK(deser_obj.is_exception());

  const auto obj_ptr = std::dynamic_pointer_cast<const new_object>(deser_obj.as_object());
  REQUIRE CHECK(obj_ptr != nullptr);

  const auto cls_ptr = std::dynamic_pointer_cast<const new_class_desc__class_desc>(obj_ptr->cls);
  REQUIRE CHECK(cls_ptr != nullptr);
  CHECK_EQUAL(field_descriptor(u"java.io.IOException"), cls_ptr->class_name);

  // XXX: figure out how to extract:
  // (*obj_ptr)[class: "java.lang.Throwable"].fields[field: "detailMessage"]
  // and that it holds: u"exception test"
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Require argument: path to serialized file SerClass.ser\n";
    return 1;
  }
  SER_FIELD_PATH = argv[1];

  return UnitTest::RunAllTests();
}
