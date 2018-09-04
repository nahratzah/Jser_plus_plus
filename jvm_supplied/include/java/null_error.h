#ifndef JAVA_NULL_ERROR_H
#define JAVA_NULL_ERROR_H

#include <stdexcept>

namespace java {

class null_error
: public std::logic_error
{
 public:
  null_error();
  null_error(const std::string& what_arg);
  null_error(const char* what_arg);
  ~null_error() override;
};

} /* namespace java */

#endif /* JAVA_NULL_ERROR_H */
