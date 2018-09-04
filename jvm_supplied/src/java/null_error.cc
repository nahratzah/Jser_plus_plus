#include <java/null_error.h>

namespace java {

null_error::null_error()
: null_error("nullptr dereference")
{}

null_error::null_error(const std::string& what_arg)
: std::logic_error(what_arg)
{}

null_error::null_error(const char* what_arg)
: std::logic_error(what_arg)
{}

null_error::~null_error() = default;

} /* namespace java */
