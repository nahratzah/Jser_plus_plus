#ifndef JAVA_HSPERF_JMX_HSPERF_H
#define JAVA_HSPERF_JMX_HSPERF_H

#include <java/hsperf/hsperf.h>
#include <optional>
#include <string>

namespace java::hsperf {


///\brief Retrieve JMX connection string from hsperf.
auto jmx_hsperf(const hsperf& p) -> std::optional<std::string>;

///\brief Retrieve JMX connection string for remote connections, from hsperf.
auto jmx_hsperf_remotes(const hsperf& p) -> std::vector<std::string>;


} /* namespace asio_jmx */

#endif /* JAVA_HSPERF_JMX_HSPERF_H */
