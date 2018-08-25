#ifndef JAVA_SERIALIZATION_EXCEPTION_H
#define JAVA_SERIALIZATION_EXCEPTION_H

#include <stdexcept>

namespace java::serialization {


class serialization_error
: public std::runtime_error
{
 public:
  using std::runtime_error::runtime_error;
};

class decoding_error
: public serialization_error
{
 public:
  using serialization_error::serialization_error;
};


} /* namespace java::serialization */

#endif /* JAVA_SERIALIZATION_EXCEPTION_H */
