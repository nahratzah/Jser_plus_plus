#include <java/serialization/encdec.h>
#include "UnitTest++/UnitTest++.h"
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <iostream>
#include <cycle_ptr/cycle_ptr.h>

using namespace java::serialization::stream;

std::string SER_RESET_PATH;

TEST(deserialize) {
  std::ifstream file(SER_RESET_PATH, std::ifstream::in | std::ifstream::binary);

  reader r;

  auto deser_obj = r.read(file);
  REQUIRE CHECK(deser_obj.is_object());
  REQUIRE CHECK(!deser_obj.is_exception());
  auto element_ptr = deser_obj.as_object();
  REQUIRE CHECK(element_ptr != nullptr);
  std::cout << *element_ptr << std::endl;
  REQUIRE CHECK_EQUAL(stream_string(u"foo"), *element_ptr);

  deser_obj = r.read(file);
  REQUIRE CHECK(deser_obj.is_object());
  REQUIRE CHECK(!deser_obj.is_exception());
  element_ptr = deser_obj.as_object();
  REQUIRE CHECK(element_ptr != nullptr);
  std::cout << *element_ptr << std::endl;
  REQUIRE CHECK_EQUAL(stream_string(u"bar"), *element_ptr);

  deser_obj = r.read(file);
  REQUIRE CHECK(deser_obj.is_object());
  REQUIRE CHECK(!deser_obj.is_exception());
  element_ptr = deser_obj.as_object();
  REQUIRE CHECK(element_ptr != nullptr);
  std::cout << *element_ptr << std::endl;
  CHECK_EQUAL(stream_string(u"bar"), *element_ptr);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Require argument: path to serialized file SerString.ser\n";
    return 1;
  }
  SER_RESET_PATH = argv[1];

  return UnitTest::RunAllTests();
}
