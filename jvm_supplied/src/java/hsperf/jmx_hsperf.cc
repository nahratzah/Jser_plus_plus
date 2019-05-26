#include <java/hsperf/jmx_hsperf.h>
#include <regex>

namespace java::hsperf {


using namespace std::literals;


auto jmx_hsperf(const hsperf& p)
-> std::optional<std::string> {
  const auto map = p.get();
  const auto entry_iter = map.find(u8"sun.management.JMXConnectorServer.address"s);
  if (entry_iter == map.end()) return {};

  if (!std::holds_alternative<std::string>(entry_iter->second)) return {};
  return std::get<std::string>(entry_iter->second);
}

auto jmx_hsperf_remotes(const hsperf& p)
-> std::vector<std::string> {
  std::vector<std::string> result;

  const std::regex filter(u8R"(sun\.management\.JMXConnectorServer\.[[:digit:]]+\.remoteAddress)");
  const auto map = p.get();
  for (const auto& entry : map) {
    if (std::holds_alternative<std::string>(entry.second) && regex_match(entry.first, filter))
      result.push_back(std::get<std::string>(entry.second));
  }
  return result;
}


} /* namespace java::hsperf */
