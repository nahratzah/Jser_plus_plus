#ifndef SUPPORT_H
#define SUPPORT_H

#include <java/serialization/encdec.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>

inline auto read_one(std::string filename) {
  std::ifstream file(filename, std::ifstream::in | std::ifstream::binary);

  java::serialization::stream::reader r;
  auto result = r.read(file);
  std::cout << result << std::endl;
  return result;
}

#endif /* SUPPORT_H */
